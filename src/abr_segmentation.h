#pragma once
#include <gst/gst.h>
#include "parameters.h"
#include "log.h"
#include "bin.h"

/*
 *   PIPELINE:
 *
 *      bin_url_to_estreams
 *          urlbin --> typefind --> demux --> 
 *                                          video parser --> 
 *                                          audio parser --> 
 *      bin_estream_to_dstream
 *          decoder -->  
 *      bin_dstream_to_estream
 *          filter --> encoder -->  
 *      bin_estream_to_cmafv
 *          parser --> mp4mux or (bento4 lib)
 *      bin_estream_to_cmafa
 *          parser --> mp4mux or (bento4 lib)
 *      bin_cmaf_to_s3_db 
 *          buffer --> S3
 *          buffer info --> DynamoDB
 *
 *
 *      bin_url_to_estreams -> 
 *                   video estream -> bin_estream_to_cmafv     (for passthrough)
 *                   video estream -> bin_estream_to_dstream -> 
 *                                          bin_dstream_to_estream -> bin_estream_to_cmafv
 *                                          bin_dstream_to_estream -> bin_estream_to_cmafv
 *                   audio estream -> bin_estream_to_cmafa     (for passthrough)
 *                   audio estream -> bin_estream_to_dstream -> 
 *                                          bin_dstream_to_estream -> bin_estream_to_cmafa
 *                                          bin_dstream_to_estream -> bin_estream_to_cmafa
 *
 */
namespace AbrSegmentation {
    struct Pipeline_data {
        GMainLoop*      loop = nullptr;
        GstPipeline*    pipeline = nullptr;
        GstBus*         bus = nullptr;
        guint           watch_id = 0;
        ~Pipeline_data(){
            try{
                if(pipeline != nullptr){
                    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
                    gst_object_unref(pipeline);
                }
                if(bus != nullptr){
                    gst_object_unref(bus);
                }
                if(watch_id){
                    g_source_remove(watch_id);
                } 
                if(loop != nullptr){
                    g_main_loop_unref(loop);
                }
                bus = nullptr; pipeline = nullptr; loop = nullptr; watch_id = 0;
                BOOST_LOG_TRIVIAL(info) << "Clean pipeline";
            }catch(std::exception& e){
                LOG(error) << e.what();
            }
        }
    };

    bool init();
    void add_bus_watch();
    void dot_file(int sec);
    void find_video_stream(GstPad*);
    void find_audio_stream(GstPad*);
    int on_bus_message(GstBus*, GstMessage*, gpointer);
    bool run(Parameters*);
};
