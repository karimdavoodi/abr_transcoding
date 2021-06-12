#pragma once
#include <string>
#include <gst/gst.h>

namespace Pad {
    std::string pad_name(GstPad* p);
    std::string pad_mime_type(GstPad* p);
    std::string pad_caps_string(GstPad* p);
}
