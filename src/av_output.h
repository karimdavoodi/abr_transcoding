
#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class AvOutput {
    private:
        int id;
        int index;
    public:
        AvOutput(int id, int sid);
        void process_input(AVPacket*, AVFrame*);
        void flush();
        ~AvOutput();
};
