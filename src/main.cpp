#include <iostream>
#include <boost/log/trivial.hpp>

#include "log.h"
#include "parameters.h"
#include "abr_segmentation.h"

using namespace std;
int main(int argc, char* argv[])
{
    Parameters parameters;
    
    // TODO: for test only
    parameters.set_var("ABR_CONTENT_ID", "sample");
    parameters.set_var("ABR_INPUT_SOURCE", "file:///home/karim/Videos/20.mp4");

    if(argc < 2)
        parameters.get_environments();
    else
        parameters.get_commandline((const char**) argv);


    LOG(info) << "Parameters:" << parameters.to_string();
    if(parameters.get_var("ABR_INPUT_SOURCE").empty()){
        LOG(error) << "Please set parameters!";
        return EXIT_FAILURE;
    }
    return AbrSegmentation::run(parameters);
}
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
