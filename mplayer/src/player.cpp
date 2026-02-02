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
        ifmt = av_find_input_format("dshow");
        path = "video=" + inputPath;
    }
    
    if (avformat_open_input(&ctx->fmt_ctx, path.toUtf8().data(), ifmt, nullptr) < 0) {
        qDebug() << "Failed to open input:" << path;
        return -1;
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
                    
                    // 发送进度信号
                    if (ctx->fmt_ctx && ctx->video_index != -1) {
                        int64_t pts = ctx->frame->pts;
                        if (pts == AV_NOPTS_VALUE) pts = ctx->frame->pkt_dts;
                        int64_t ms = 0;
                        if (pts != AV_NOPTS_VALUE) {
                            ms = av_rescale_q(pts, ctx->fmt_ctx->streams[ctx->video_index]->time_base, {1, 1000});
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

