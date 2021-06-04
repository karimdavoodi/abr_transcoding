extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "av_input.h"
#include "config.h"

bool AvInput::init()
{
    int ret;
    unsigned int i;

    if (input.empty())
    {
        LOG(error) << "Please set input ";
        return false;
    }
    fmt_ctx = NULL;
    if ((ret = avformat_open_input(&fmt_ctx, input.c_str(), NULL, NULL)) < 0)
    {
        LOG(error) << "Can't open input " << input;
        return false;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
    {
        LOG(error) << "Cannot find stream information";
        return false;
    }
    stream_ctxs.resize(fmt_ctx->nb_streams);

    for (i = 0; i < fmt_ctx->nb_streams; i++)
    {
        AVStream *stream = fmt_ctx->streams[i];
        // Only Video and Audio
        if (stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO){
            stream_ctxs[i].id = -1;
            stream_ctxs[i].index = -1;
            continue;
        }

        const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!dec)
        {
            LOG(error) << "Failed to find decoder for stream " << i;
            return false;
        }
        AVCodecContext *codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx)
        {
            LOG(error) << "Failed to allocate the decoder context for stream " << i;
            return false;
        }
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0)
        {
            LOG(error) << "Failed to copy decoder parameters to input decoder context "
                "for stream "
                << i;
            return false;
        }
        LOG(info) << "Input: stream id " << stream->id
            << " index " << stream->index
            << " type " << codec_ctx->codec_type;

        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            codec_ctx->framerate = av_guess_frame_rate(fmt_ctx, stream, NULL);
            LOG(info) << "Resolution " << codec_ctx->width << "x"
                << codec_ctx->height;
            width = codec_ctx->width;
            height = codec_ctx->height;
        }
        /* Open decoder */
        ret = avcodec_open2(codec_ctx, dec, NULL);
        if (ret < 0)
        {
            LOG(error) << "Failed to open decoder for stream " << i;
            return false;
        }

        StreamContext sctx; 
        sctx.index = stream->index;
        sctx.id = stream->id;
        sctx.type = codec_ctx->codec_type;
        sctx.dec_ctx = codec_ctx;

        sctx.dec_frame = av_frame_alloc();
        if (!sctx.dec_frame)
        {
            LOG(error) << "Can't alloc memory for frame";
            return false;
        }
        stream_ctxs[i] = sctx;
    }

    if (!(packet = av_packet_alloc())){
        LOG(error) << "Can't alloc packet!";
        return false;
    }
    duration = fmt_ctx->duration;
    bitrate = fmt_ctx->bit_rate;
    return true;
}
bool AvInput::next_packet(AVPacket* pkt)
{
    int ret = av_read_frame(fmt_ctx, packet);
    if(ret < 0 ) return false;
    pkt = packet;
    return true;
}
AVFrame *AvInput::decodec_packet(AVPacket *)
{
    return NULL;
}
AvInput::~AvInput()
{
    if(fmt_ctx != NULL)
        avformat_close_input(&fmt_ctx);
}
