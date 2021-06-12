#include "pad.h"
namespace Pad {
    std::string pad_mime_type(GstPad* pad)
    {
        if(!pad) return "";
        auto caps = gst_pad_query_caps(pad, nullptr);
        if(!caps) return "";

        auto caps_struct = gst_caps_get_structure(caps, 0);
        auto pad_type = std::string(gst_structure_get_name(caps_struct));

        gst_caps_unref(caps);
        return pad_type;
    }
    std::string pad_name(GstPad* pad)
    {
        if(!pad) return "";
        auto name = gst_pad_get_name(pad);
        std::string result {name};
        g_free(name);
        return result;
    }
    std::string pad_caps_string(GstPad* pad)
    {
        auto caps = gst_pad_query_caps(pad, nullptr);
        if(!caps) return "";
        auto caps_string = gst_caps_to_string(caps);
        std::string result{caps_string};

        gst_caps_unref(caps);
        g_free(caps_string);

        return result;
    }
}
