#pragma once
#include <string>
#include <vector>
#include <initializer_list>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class AvInput
{
private:
    struct StreamContext
    {
        int type;
        int id;
        int index;
        AVCodecContext *dec_ctx;
        AVFrame *dec_frame;
        StreamContext():dec_ctx(NULL),dec_frame(NULL){}
        ~StreamContext(){
            if(dec_ctx) avcodec_free_context(&dec_ctx);
            if(dec_frame) av_frame_free(&dec_frame);
        }
    };
    int width = 0;
    int height = 0;
    int bitrate = 0;
    int duration = 0;

    AVPacket *packet = NULL;
    AVFrame *frame = NULL;

    AVFormatContext *fmt_ctx = NULL;
    std::string input;

public:
    std::vector<StreamContext> stream_ctxs;
    
    void set_input(const std::string _input) { input = _input; }
    bool next_packet(AVPacket*);
    AVFrame *decodec_packet(AVPacket *);
    bool init();
    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_bitrate() const { return bitrate; }
    ~AvInput();
};
