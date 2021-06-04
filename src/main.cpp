#include <iostream>
#include <cstdlib>
#include <map>
#include <boost/log/trivial.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "config.h"
#include "util.h"
#include "amqp_client.h"
#include "av_input.h"
#include "av_output.h"

using namespace std;

/**
 * 
 *    ABR_CONTENT_ID = "name"
 *    ABR_INPUT_SOURCE = "http://"
 *
 *    ABR_AMQP_HOST="mq_host"
 *    ABR_AMQP_PORT="mq_host"
 *    ABR_AMQP_USER="mq_host"
 *    ABR_AMQP_PASS="mq_host"
 *    ABR_AMQP_SEG_QUEUE="seg_queue"
 *    ABR_AMQP_IMG_QUEUE="img_queue"
 *    ABR_AMQP_INF_QUEUE="inf_queue"
 *
 *    ABR_SEGMENT_LEN="4"
 *
 *    ABR_OUTPUT_FORMAT="fmp4"
 *    ABR_OUTPUT_VCODEC="h264"
 *    ABR_OUTPUT_ACODEC="aac"
 *    
 *    TODO: ABR_CREATE_SLIDE="yes"
 *
 *    ABR_AUTO_SCALE="yes/no"
 *
 *    if ABR_AUTO_SCALE=="yes"
 *      if video_size == 4K:
 *          ABR_0_SIZE="original"
 *          ABR_0_BITRATE="original"
 *          ABR_1_SIZE="FHD"
 *          ABR_1_BITRATE="6000000"
 *          ABR_1_SIZE="HD"
 *          ABR_1_BITRATE="3000000"
 *          ABR_1_SIZE="SD"
 *          ABR_1_BITRATE="1500000"
 *          ABR_1_SIZE="CD"
 *          ABR_1_BITRATE="500000"
 *      if video_size == 4K:
 *          ABR_0_SIZE="original"
 *          ABR_0_BITRATE="original"
 *          ...
 *
 *    if ABR_AUTO_SCALE=="no"
 *          ABR_0_SIZE="original"
 *          ABR_0_BITRATE="original"
 *          ABR_1_SIZE="HD"
 *          ABR_1_BITRATE="3000000"
 *          ...
 *          
 *
 *    OUTPUT:
 *    content metadata ---> info_queue
 *    url -> prob -> 
 *              audio1 --> encodeIfNotOUTPUT_ACODE -> segmentation -> seg_queue 
 *              audioN --> encodeIfNotOUTPUT_ACODE -> segmentation -> seg_queue 
 *
 *              video  --> original -> encodeIfNotOUTPUT_VCODE ->  segmentation -> seg_queue 
 *              video  --> scale1 --> encode -> segmentation -> seg_queue 
 *              video  --> scaleN --> encode -> segmentation -> seg_queue 
 *
 *              video  --> key-frame --> jpeg(original) -> image_queue 
 *              video  --> key-frame --> jpeg(small) -> image_queue 
 *
 */
int main()
{
    AmqpClient          amqp_client;
    AvInput             input;
    vector<AvOutput>    video_outputs;
    vector<AvOutput>    audio_outputs;
    
    map<string, string> env_vars = {
        {"ABR_CONTENT_ID", "sample1"},
        {"ABR_INPUT_SOURCE", "file:///home/karim/Videos/20.mp4"},
        {"ABR_AMQP_HOST", "127.0.0.1"},
        {"ABR_AMQP_PORT", "5672"},
        {"ABR_AMQP_USER", "guest"},
        {"ABR_AMQP_PASS", "guest"},
        {"ABR_AMQP_SEG_QUEUE", "seg_queue"},
        {"ABR_AMQP_IMG_QUEUE", "img_queue"},
        {"ABR_AMQP_INF_QUEUE", "inf_queue"},
        {"ABR_SEGMENT_LEN", "4"},
        {"ABR_OUTPUT_FORMAT", "fmp4"},
        {"ABR_OUTPUT_VCODEC", "h264"},
        {"ABR_OUTPUT_ACODEC", "aac"},
        {"ABR_AUTO_SCALE", "yes"}
    };
    Util::read_env_variables(env_vars);
    if (env_vars["ABR_AUTO_SCALE"] == "no") {
        Util::read_abr_env_variables(env_vars);
    }

    amqp_client.set_host(env_vars["ABR_AMQP_HOST"], env_vars["ABR_AMQP_PORT"],
                         env_vars["ABR_AMQP_USER"], env_vars["ABR_AMQP_PASS"]);   

    input.set_input(env_vars["ABR_INPUT_SOURCE"]);

    if (!input.init()){
        LOG(error) << "Can't probe input " << env_vars["ABR_INPUT_SOURCE"];
        return EXIT_FAILURE;
    }

    if (env_vars["ABR_AUTO_SCALE"] == "yes") {
        Util::auto_scale_vars(env_vars, input);    
    }

    // push content metadata 
    string json = Util::generate_content_json_metadata(env_vars);
    amqp_client.send_queue(env_vars["ABR_AMQP_INF_QUEUE"], json.c_str(), json.size());

    for(const auto& stream : input.stream_ctxs){
        if(stream.type == AVMEDIA_TYPE_VIDEO)
            video_outputs.emplace_back(AvOutput{ stream.id, stream.index });
        if(stream.type == AVMEDIA_TYPE_AUDIO)
            audio_outputs.emplace_back(AvOutput{ stream.id, stream.index });
    }

    AVPacket *packet = NULL;
    AVFrame *frame = NULL;
    
    while(true) {

        if(!input.next_packet(packet)) 
            break;
        
        if(Util::packet_is_video(packet)){
            frame = input.decodec_packet(packet); 
            for (auto& out : video_outputs){
                out.process_input(packet, frame);
            }
            // TODO: generate slide show
        }else if(Util::packet_is_audio(packet)){
            for (auto& out : audio_outputs){
                out.process_input(packet, frame);
            }
        } else {
            // TODO: ignore other non AV streams for now
        }
    }
    for (auto& out : video_outputs)
        out.flush();
    for (auto& out : audio_outputs)
        out.flush();
}
