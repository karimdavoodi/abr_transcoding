#include <vector>
#include <map>
#include "gst/gstelementfactory.h"
#include "log.h"
#include "gst.h"
#include "bin_url_to_estreams.h"

using namespace std;


void urisourcebin_pad_added(
        GstElement *, GstPad *pad, gpointer obj)
{
    Bin_url_to_estreams *bin = (Bin_url_to_estreams *)obj;
    LOG(debug) << "Get caps in urisourcebin:" << Gst::pad_caps_string(pad);
    Element& queue = bin->get_element("url_queue_src");
    if(!queue.link_pad_to_static_sink(pad, "sink")){
        LOG(error) << "Can't link urisourcebin to queue by caps:" 
            <<  Gst::pad_caps_string(pad);
    }
}
void demux_padd_added(GstElement* , GstPad* pad, gpointer obj)
{
    Bin_url_to_estreams *bin = (Bin_url_to_estreams *)obj;
    auto pad_type = Gst::pad_mime_type(pad);
    LOG(debug) << "Pad of demuxer:" << pad_type;
    if(pad_type.find("video/") != string::npos){
        bin->has_video(pad);
    }else if(pad_type.find("audio/") != string::npos){
        bin->has_audio(pad);
    }
}

void typefind_have_type(GstElement *typefind,
        guint,
        GstCaps *caps,
        gpointer obj)
{
    Bin_url_to_estreams *bin = (Bin_url_to_estreams *)obj;
    auto caps_struct = gst_caps_get_structure(caps, 0);
    string pad_type = string(gst_structure_get_name(caps_struct));
    LOG(debug) << "Pad_type:" << pad_type; 
    
    // needs demuxer
    
     
    if( Gst::mime_demuxer.count(pad_type) > 0){
        LOG(debug) << "init demuxer:" << Gst::mime_demuxer[pad_type];
        auto demux = bin->add_element(Gst::mime_demuxer[pad_type], "url_demux", true);
        gst_element_link(typefind, demux);
        g_signal_connect(demux, "pad-added", G_CALLBACK(demux_padd_added), obj);
        // it is video stream
    }else if(Gst::mime_decoder.count(pad_type) > 0){
        auto src_pad = gst_element_get_static_pad(typefind, "src");
        bin->has_video(src_pad);
        // it is RTP stream
    }else if(pad_type.find("application/x-rtp") != string::npos){
        const char* encodin_name = gst_structure_get_string(caps_struct, "encoding-name");
        if(!encodin_name){
            LOG(error)<< "RTP dose not have encodin_name!";
            bin->main_loop_quit();
            return;
        }
        if(!strncmp(encodin_name, "MP2T", 4)){
            auto depay = bin->add_element("rtpmp2tdepay", "url_rtmpdepay", true);
            auto demux = bin->add_element("tsdemux", "url_demux", true);

            gst_element_link_many(typefind, depay, demux, nullptr);
            g_signal_connect(demux, "pad-added", G_CALLBACK(demux_padd_added), obj);
        }else if(!strncmp(encodin_name, "H264", 4)){
            auto depay = bin->add_element("rtph264depay", "url_rtmpdepay", true);
            gst_element_link_many(typefind, depay, nullptr);
            auto src_pad = gst_element_get_static_pad(depay, "src");
            bin->has_video(src_pad);
        }else if(!strncmp(encodin_name, "H265", 4)){
            auto depay = bin->add_element("rtph265depay", "url_rtmpdepay", true);
            gst_element_link_many(typefind, depay, nullptr);
            auto src_pad = gst_element_get_static_pad(depay, "src");
            bin->has_video(src_pad);
        // not support format!
        }else{
            LOG(error) << "Not support RTP type:" << encodin_name;
            bin->main_loop_quit();
        }
    }else{
            LOG(error) << "Not support input type:" << pad_type;
            bin->main_loop_quit();
    }
}
bool Bin_url_to_estreams::build(
        std::function<void(GstPad *)> find_video_stream,
        std::function<void(GstPad *)> find_audio_stream){
    has_video = find_video_stream;
    has_audio = find_audio_stream;

    add_element("urisourcebin", "url_urisourcebin");
    add_element("queue", "url_queue_src");
    add_element("typefind", "url_typefind");

    if (elements_count() != 3)
    {
        LOG(error) << "Can't make all elements in " << get_name();
        return false;
    }

    if (!gst_element_link_many(
                get_element("url_queue_src").get_ptr(),
                get_element("url_typefind").get_ptr(),
                nullptr))
    {
        LOG(error) << "Can't link elements in " << get_name();
        return false;
    }
    bool ret;
    ret = g_signal_connect(
            get_element("url_urisourcebin").get_ptr(),
            "pad-added",
            G_CALLBACK(urisourcebin_pad_added),
            this);
    if (ret <= 0)
    {
        LOG(error) << "Can't connect signal to url_urisourcebin";
    }
    ret = g_signal_connect(
            get_element("url_typefind").get_ptr(),
            "have-type",
            G_CALLBACK(typefind_have_type),
            this);

    g_object_set(
            get_element("url_urisourcebin").get_ptr(),
            "uri",
            url.c_str(),
            nullptr);
    return true;
}
