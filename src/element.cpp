#include "log.h"
#include "gst.h"
#include "element.h"

bool Element::make(const std::string& _type, const std::string& _name, bool set_state)
{

    name = _name;
    type = _type;
    if(name.empty()) elm = gst_element_factory_make(type.c_str(), nullptr);
    else             elm = gst_element_factory_make(type.c_str(), name.c_str());
    if(!elm) return false;
    if(set_state)
        gst_element_set_state(elm, GST_STATE_PLAYING);

    return true;
}
bool Element::link_pad_to_static_sink(GstPad* pad, const std::string& sink_name)
{
    assert(elm);
    auto element_pad = gst_element_get_static_pad(elm, sink_name.c_str());
    if(!element_pad){
        LOG(error) << "Can't get static pad of " << name;
        return false;
    }
    bool ret = gst_pad_link(pad, element_pad);
    gst_object_unref(element_pad);

    if(ret != GST_PAD_LINK_OK){
        LOG(error) << "Can'n link " << Gst::pad_name(pad) 
            << " to " << name
            << " SRC PAD CAPS:" << Gst::pad_caps_string(pad);
        return false;
    }
    LOG(trace) << "Link " << Gst::pad_name(pad) << " to " << name;
    return true;
}
