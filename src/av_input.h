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
    int width;
    int height;
    int bitrate;
    int duration;

    std::string input;

    AVPacket *packet;
    AVFrame *frame;
    AVFormatContext *fmt_ctx;

public:
    AvInput():width{0}, height{0}, bitrate{0}, duration{0}, 
        input{""},packet{nullptr},frame{nullptr}, fmt_ctx{nullptr} {}
    std::vector<StreamContext> stream_ctxs;
    
    void set_input(const std::string _input) { input = _input; }
    bool next_packet(AVPacket*);
    AVFrame *decodec_packet(AVPacket *);
    bool init();
    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_bitrate() const { return bitrate; }
    ~AvInput(){
            if(packet) av_packet_free(&packet);
            if(frame) av_frame_free(&frame);
            if(fmt_ctx) avformat_free_context(fmt_ctx);
    }
};
