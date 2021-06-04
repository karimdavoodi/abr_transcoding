extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "av_output.h"


AvOutput::AvOutput(int _id, int _index)
{
    id = _id;
    index = _index;

}
void AvOutput::process_input(AVPacket*, AVFrame*)
{

}
void AvOutput::flush()
{
    
}
AvOutput::~AvOutput()
{

}
