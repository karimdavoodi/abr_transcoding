#include "log.h"
#include "pad.h"
#include "element.h"

bool Element::make(const std::string& _type, const std::string& _name)
        {
            name = _name;
            type = _type;
            if(name.empty()) elm = gst_element_factory_make(type.c_str(), nullptr);
            else             elm = gst_element_factory_make(type.c_str(), name.c_str());
            return elm == nullptr ? false : true;
        }
bool Element::link_pad_to_static_sink(GstPad* pad, const std::string& sink_name)
{
    if(!elm){
        LOG(error) << "Elemet is null " << name;
        return false;
    } 
    auto element_pad = gst_element_get_static_pad(elm, sink_name.c_str());
    if(!element_pad){
        LOG(error) << "Can't get static pad of " << name;
        return false;
    }
    bool ret = gst_pad_link(pad, element_pad);
    gst_object_unref(element_pad);

    if(ret != GST_PAD_LINK_OK){
        LOG(error) << "Can'n link " << Pad::pad_name(pad) 
            << " to " << name
            << " SRC PAD CAPS:" << Pad::pad_caps_string(pad);
        return false;
    }
    LOG(trace) << "Link " << Pad::pad_name(pad) << " to " << name;
    return true;
}
