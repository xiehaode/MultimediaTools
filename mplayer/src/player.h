#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <vector>
#include <mutex>
#include "mdevice.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
}

struct FFPlayerContext {
    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *video_decode_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    AVFrame *frame = nullptr;
    int video_index = -1;

    uint8_t *data[4] = {nullptr};
    int linesize[4] = {0};
    
    SharedFrameBuffer shared_buf;
    bool is_quit = false;
};

class player : public QObject
{
    Q_OBJECT
public:
    explicit player(QObject *parent = nullptr);
    ~player();

    int ffplayer_open(const QString& inputPath, bool isDevice = true);
    void ffplayer_read_frame();
    void ffplayer_close();

    bool isQuit() const { return ctx->is_quit; }
    void stop();
    void pause(bool pause);
    void seek(int64_t timestamp_ms);
    
    bool isPaused() const { return m_paused; }
    
    SharedFrameBuffer* getSharedBuffer() { return &ctx->shared_buf; }
    int64_t getDuration() const;
    int64_t getCurrentTime() const;

signals:
    void frameReady(int w, int h, int type, int bpp);
    void positionChanged(int64_t ms, int64_t total_ms);


private:
    FFPlayerContext *ctx;
    bool m_paused = false;
    bool m_seek_req = false;
    int64_t m_seek_target = 0;
    std::mutex m_ctrl_mtx;
};

#endif // PLAYER_H
