#include <thread>
#include "log.h"
#include "abr_segmentation.h"
#include "bin_url_to_estreams.h"
/*
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

    Pipeline_data pdata;

    bool init()
    {
        gst_init (nullptr, nullptr);
        pdata.loop      = g_main_loop_new(nullptr, false);
        pdata.pipeline  = GST_PIPELINE(gst_element_factory_make("pipeline", nullptr));

        if(!pdata.loop){
            LOG(error) << "Can't make gst loop!";
            return false;
        }
        if(!pdata.pipeline){
            LOG(error) << "Can't make gst pipeline!";
            return false;
        }
        return true;
    }
    void find_video_stream(GstPad* src)
    {
        LOG(info) << "Find Video stream";
    }
    void find_audio_stream(GstPad* src)
    {
        LOG(info) << "Find Audio stream";
    }
    bool run(Parameters& parameters)
    {
        if(!init()) return false;

        Bin_url_to_estreams bin_url_to_estreams { 
            parameters.get_var("ABR_INPUT_SOURCE"), pdata.loop };

        bin_url_to_estreams.build( find_video_stream, find_audio_stream);

        gst_bin_add(GST_BIN(pdata.pipeline), bin_url_to_estreams.get_bin().get_ptr()); 

        //dot_file(9);
        add_bus_watch();
        gst_element_set_state(GST_ELEMENT(pdata.pipeline), GST_STATE_PLAYING);
        g_main_loop_run(pdata.loop);

        return true;
    }

    int on_bus_message(GstBus *, GstMessage* message, gpointer)
    {
        if(!AbrSegmentation::pdata.pipeline) return true;

        switch (GST_MESSAGE_TYPE (message)) {
            case GST_MESSAGE_ERROR:
                {
                    gchar *debug;
                    GError *err;
                    gst_message_parse_error (message, &err, &debug);
                    LOG(error) <<  err->message << " debug:" << debug;
                    g_error_free (err);
                    g_free (debug);
                    // TODO: enable pipile dump by default
                    setenv("GST_DEBUG_DUMP_DOT_DIR", "/tmp", 1);
                    char* path = getenv("GST_DEBUG_DUMP_DOT_DIR");
                    if(path != nullptr){
                        string fname = "gst_error";
                        LOG(info) << "Make DOT file in " << path 
                            << "/" << fname << ".dot";
                        gst_debug_bin_to_dot_file(
                                GST_BIN(pdata.pipeline), 
                                GST_DEBUG_GRAPH_SHOW_ALL, 
                                fname.c_str() );
                    }
                    if(pdata.loop) g_main_loop_quit (pdata.loop);
                    break;
                }
            case GST_MESSAGE_WARNING:
                {
                    gchar *debug;
                    GError *err;
                    gst_message_parse_warning (message, &err, &debug);
                    LOG(warning) <<  err->message << " debug:" << debug;
                    g_error_free (err);
                    g_free (debug);
                    break;
                }
            case GST_MESSAGE_EOS:
                {
                    LOG(info) << "Pipeline got EOS";
                    if(pdata.loop) g_main_loop_quit (pdata.loop);
                }
            default:
                break;
        }
        return true;
    }
    void add_bus_watch()
    {
        if(!pdata.pipeline){
            LOG(error) << "Can't add bus watch, pipeline in null";
            return;
        }
        pdata.bus = gst_element_get_bus (GST_ELEMENT(pdata.pipeline));
        if(!pdata.bus){
            LOG(error) << "Can't add bus watch";
            return;
        }
        pdata.watch_id = gst_bus_add_watch(pdata.bus, AbrSegmentation::on_bus_message, nullptr);
    }
    void dot_file(int sec)
    {
        if(!pdata.pipeline){
            LOG(error) << "Can't add bus watch, pipeline in null";
            return;
        }
        char* env = getenv("GST_DEBUG_DUMP_DOT_DIR");
        string make_dot_file = (env != nullptr) ? env : "";
        if(!make_dot_file.empty()){
            string fname = "abr_segmentation";
            std::thread t([&](){

                    std::this_thread::sleep_for(std::chrono::seconds(sec));
                    if(GST_OBJECT_REFCOUNT_VALUE(pdata.pipeline) == 0 ) return;
                    LOG(trace) << "Make DOT file "
                    << make_dot_file << "/" << fname << ".dot";
                    gst_debug_bin_to_dot_file(
                            GST_BIN(pdata.pipeline),
                            GST_DEBUG_GRAPH_SHOW_ALL,
                            fname.c_str());
                    });
            t.detach();
        }else{
            LOG(trace) << "Not make DOT file";
        }

    }
}
