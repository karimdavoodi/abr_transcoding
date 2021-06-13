#include <cassert>
#include "gst.h"
using namespace std;


namespace Gst {

    string pad_mime_type(GstPad* pad)
    {
        assert(pad);

        auto caps = gst_pad_query_caps(pad, nullptr);
        if(!caps) return "";

        auto caps_struct = gst_caps_get_structure(caps, 0);
        auto pad_type = std::string(gst_structure_get_name(caps_struct));

        gst_caps_unref(caps);
        return pad_type;
    }
    string pad_name(GstPad* pad)
    {
        assert(pad);
        
        auto name = gst_pad_get_name(pad);
        string result {name};
        g_free(name);
        return result;
    }
    string pad_caps_string(GstPad* pad)
    {
        assert(pad);

        auto caps = gst_pad_query_caps(pad, nullptr);
        if(!caps) return "";
        auto caps_string = gst_caps_to_string(caps);
        string result{caps_string};

        gst_caps_unref(caps);
        g_free(caps_string);

        return result;
    }
    string pad_proper_parser(GstPad* pad)
    {
        assert(pad);
        return "";
           
    }
    string find_parser_for_mime(const string& mime_type)
    {
        return "";
    }

}
