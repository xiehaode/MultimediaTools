#include "player.h"
#include <QDebug>

player::player(QObject *parent) : QObject(parent)
{
    avdevice_register_all();
    ctx = new FFPlayerContext();
}

player::~player()
{
    stop();
    if (ctx->sws_ctx) sws_freeContext(ctx->sws_ctx);
    if (ctx->video_decode_ctx) avcodec_free_context(&ctx->video_decode_ctx);
    if (ctx->fmt_ctx) avformat_close_input(&ctx->fmt_ctx);
    if (ctx->frame) av_frame_free(&ctx->frame);
    delete ctx;
}

int player::ffplayer_open(const QString& inputPath, bool isDevice)
{
    AVInputFormat *ifmt = nullptr;
    QString path = inputPath;
    
    if (isDevice) {
        if (inputPath == "desktop") {
            ifmt = av_find_input_format("gdigrab");
            path = "desktop";
            
            // 设置屏幕抓取参数
            AVDictionary* options = nullptr;
            av_dict_set(&options, "framerate", "30", 0);
            av_dict_set(&options, "draw_mouse", "1", 0);
            
            if (avformat_open_input(&ctx->fmt_ctx, path.toUtf8().data(), ifmt, &options) < 0) {
                qDebug() << "Failed to open screen capture";
                av_dict_free(&options);
                return -1;
            }
            av_dict_free(&options);
        } else {
            ifmt = av_find_input_format("dshow");
            path = "video=" + inputPath;
            
            // 设置摄像头抓取参数，解决残影/模糊
            AVDictionary* options = nullptr;
            av_dict_set(&options, "framerate", "30", 0);
            // 移除 video_size 强制设置，由摄像头默认决定，防止因不支持导致卡死
            
            if (avformat_open_input(&ctx->fmt_ctx, path.toUtf8().data(), ifmt, &options) < 0) {
                qDebug() << "Failed to open camera:" << path;
                av_dict_free(&options);
                return -1;
            }
            av_dict_free(&options);
        }
    }
 else {
        if (avformat_open_input(&ctx->fmt_ctx, path.toUtf8().data(), ifmt, nullptr) < 0) {
            qDebug() << "Failed to open input file:" << path;
            return -1;
        }
    }



    if (avformat_find_stream_info(ctx->fmt_ctx, nullptr) < 0) return -1;

    for (unsigned int i = 0; i < ctx->fmt_ctx->nb_streams; i++) {
        if (ctx->fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ctx->video_index = i;
            break;
        }
    }

    if (ctx->video_index == -1) return -1;

    AVCodecParameters *codecpar = ctx->fmt_ctx->streams[ctx->video_index]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    ctx->video_decode_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx->video_decode_ctx, codecpar);
    
    if (avcodec_open2(ctx->video_decode_ctx, codec, nullptr) < 0) return -1;

    ctx->frame = av_frame_alloc();
    return 0;
}

void player::ffplayer_read_frame()
{
    if (m_paused && !m_seek_req) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_ctrl_mtx);
        if (m_seek_req) {
            int64_t seek_target = av_rescale_q(m_seek_target * 1000, {1, AV_TIME_BASE}, ctx->fmt_ctx->streams[ctx->video_index]->time_base);
            if (av_seek_frame(ctx->fmt_ctx, ctx->video_index, seek_target, AVSEEK_FLAG_BACKWARD) >= 0) {
                avcodec_flush_buffers(ctx->video_decode_ctx);
            }
            m_seek_req = false;
        }
    }

    AVPacket pkt;
    if (av_read_frame(ctx->fmt_ctx, &pkt) >= 0) {

        if (pkt.stream_index == ctx->video_index) {
            if (avcodec_send_packet(ctx->video_decode_ctx, &pkt) == 0) {
                while (avcodec_receive_frame(ctx->video_decode_ctx, ctx->frame) == 0) {
                    
                    int dw = ctx->video_decode_ctx->width;
                    int dh = ctx->video_decode_ctx->height;

                    {
                        std::lock_guard<std::mutex> lock(ctx->shared_buf.mtx);
                        
                        // 零拷贝准备：确保缓冲区大小正确
                        int target_size = dw * dh * 4; // 假设转为 RGBA
                        if (ctx->shared_buf.buf.size() != (size_t)target_size) {
                            ctx->shared_buf.buf.resize(target_size);
                            ctx->shared_buf.w = dw;
                            ctx->shared_buf.h = dh;
                            ctx->shared_buf.type = 1; // 标记为 RGBA/BGR
                        }

                        // 初始化 SwsContext
                        if (!ctx->sws_ctx) {
                            ctx->sws_ctx = sws_getContext(dw, dh, ctx->video_decode_ctx->pix_fmt,
                                                        dw, dh, AV_PIX_FMT_RGBA,
                                                        SWS_BILINEAR, nullptr, nullptr, nullptr);
                        }

                        // 直接写入共享缓冲区
                        ctx->data[0] = ctx->shared_buf.buf.data();
                        ctx->linesize[0] = dw * 4;

                        sws_scale(ctx->sws_ctx, ctx->frame->data, ctx->frame->linesize,
                                  0, dh, ctx->data, ctx->linesize);

                        ctx->shared_buf.ts = av_gettime_relative() / 1000;
                        ctx->shared_buf.state = BUFFER_FILLED;
                    }

                    emit frameReady(dw, dh, 1, 32);

                    // --- 录制逻辑 ---
                    {
                        std::lock_guard<std::mutex> lock(m_record_mtx);
                        if (m_recording && m_outFmtCtx) {
                            AVFrame* outFrame = av_frame_alloc();
                            outFrame->format = AV_PIX_FMT_YUV420P;
                            outFrame->width = dw;
                            outFrame->height = dh;
                            av_frame_get_buffer(outFrame, 32);

                            // RGBA -> YUV420P 转换 (录制通常需要 YUV)
                            static struct SwsContext* record_sws = nullptr;
                            record_sws = sws_getCachedContext(record_sws, dw, dh, AV_PIX_FMT_RGBA,
                                                              dw, dh, AV_PIX_FMT_YUV420P,
                                                              SWS_BILINEAR, nullptr, nullptr, nullptr);
                            
                            uint8_t* src_data[4] = { ctx->shared_buf.buf.data(), nullptr, nullptr, nullptr };
                            int src_linesize[4] = { dw * 4, 0, 0, 0 };
                            sws_scale(record_sws, src_data, src_linesize, 0, dh, outFrame->data, outFrame->linesize);

                            outFrame->pts = m_frame_count++;
                            
                            // 检查编码器上下文是否存在且已打开
                            if (m_outStream && m_outCodecCtx) {
                                if (avcodec_send_frame(m_outCodecCtx, outFrame) == 0) {
                                    AVPacket outPkt;
                                    av_init_packet(&outPkt);
                                    outPkt.data = nullptr;
                                    outPkt.size = 0;
                                    if (avcodec_receive_packet(m_outCodecCtx, &outPkt) == 0) {
                                        av_packet_rescale_ts(&outPkt, m_outCodecCtx->time_base, m_outStream->time_base);
                                        outPkt.stream_index = m_outStream->index;
                                        av_interleaved_write_frame(m_outFmtCtx, &outPkt);
                                        av_packet_unref(&outPkt);
                                    }
                                }
                            }
                            av_frame_free(&outFrame);


                        }
                    }
                    // --- 录制逻辑结束 ---
                    
                    // 发送进度信号
                    if (ctx->fmt_ctx && ctx->video_index != -1) {
                        AVStream* v_stream = ctx->fmt_ctx->streams[ctx->video_index];
                        int64_t pts = ctx->frame->best_effort_timestamp;
                        if (pts == AV_NOPTS_VALUE) pts = ctx->frame->pts;
                        if (pts == AV_NOPTS_VALUE) pts = ctx->frame->pkt_dts;
                        
                        int64_t ms = 0;
                        if (pts != AV_NOPTS_VALUE) {
                            int64_t start_time = (v_stream->start_time != AV_NOPTS_VALUE) ? v_stream->start_time : 0;
                            ms = av_rescale_q(pts - start_time, v_stream->time_base, {1, 1000});
                        }
                        int64_t total_ms = getDuration();
                        emit positionChanged(ms, total_ms);
                    }

                }
            }


        }
        av_packet_unref(&pkt);
    }
}

int64_t player::getDuration() const
{
    if (ctx->fmt_ctx && ctx->fmt_ctx->duration != AV_NOPTS_VALUE) {
        return ctx->fmt_ctx->duration / (AV_TIME_BASE / 1000);
    }
    return 0;
}

int64_t player::getCurrentTime() const
{
    // 简化处理，实际可以通过当前帧的pts获取
    return 0; 
}


void player::ffplayer_close()
{
    stop();
}

void player::stop()
{
    ctx->is_quit = true;
    stopRecord(); // 停止播放时自动停止录制
}


void player::pause(bool p)
{
    m_paused = p;
}

void player::seek(int64_t timestamp_ms)
{
    std::lock_guard<std::mutex> lock(m_ctrl_mtx);
    m_seek_target = timestamp_ms;
    m_seek_req = true;
}

int player::startRecord(const QString &outputPath)
{
    std::lock_guard<std::mutex> lock(m_record_mtx);
    if (m_recording) return -1;

    // 创建输出上下文
    avformat_alloc_output_context2(&m_outFmtCtx, nullptr, nullptr, outputPath.toUtf8().data());
    if (!m_outFmtCtx) return -1;

    // 添加视频流
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) return -1;
    m_outStream = avformat_new_stream(m_outFmtCtx, nullptr);
    
    AVCodecContext* c = avcodec_alloc_context3(codec);
    c->width = ctx->video_decode_ctx->width;
    c->height = ctx->video_decode_ctx->height;
    c->time_base = {1, 30}; 
    c->framerate = {30, 1};
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    c->gop_size = 30;
    c->bit_rate = 4000000; // 提高码率到 4Mbps 解决模糊
    c->max_b_frames = 0;   // 禁用 B 帧减少延迟和残影
    
    // 设置 H.264 编码预设
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "preset", "ultrafast", 0); // 最快速度，减少 CPU 占用
    av_dict_set(&opts, "tune", "zerolatency", 0); // 零延迟模式，减少残影
    
    if (m_outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(c, codec, &opts) < 0) {
        av_dict_free(&opts);
        return -1;
    }
    av_dict_free(&opts);

    m_outCodecCtx = c;
    avcodec_parameters_from_context(m_outStream->codecpar, c);
    m_outStream->time_base = c->time_base;



    if (!(m_outFmtCtx->oformat->flags & AVFMT_NOFILE)) {
        avio_open(&m_outFmtCtx->pb, outputPath.toUtf8().data(), AVIO_FLAG_WRITE);
    }

    avformat_write_header(m_outFmtCtx, nullptr);

    m_recording = true;
    m_frame_count = 0;
    return 0;
}

void player::stopRecord()
{
    std::lock_guard<std::mutex> lock(m_record_mtx);
    if (!m_recording) return;

    m_recording = false;
    av_write_trailer(m_outFmtCtx);
    
    if (m_outCodecCtx) {
        avcodec_free_context(&m_outCodecCtx);
    }
    
    if (m_outFmtCtx && !(m_outFmtCtx->oformat->flags & AVFMT_NOFILE)) {

        avio_closep(&m_outFmtCtx->pb);
    }
    
    avformat_free_context(m_outFmtCtx);
    m_outFmtCtx = nullptr;
    m_outStream = nullptr;
}


