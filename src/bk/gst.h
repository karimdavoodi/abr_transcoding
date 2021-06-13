#pragma once
#include <string>
#include <map>
#include <gst/gst.h>
using std::map;
using std::string;

namespace Gst {
    static map<string, string> mime_demuxer = {
        {"video/mpegts","tsdemux"},
        {"video/quicktime","qtdemux"},
        {"video/x-matroska","matroskademux"},
        {"video/x-msvideo","matroskademux"},
        {"video/x-ms-asf","asfdemux"},
        {"video/x-flv","flvdemux"}
    };
    static map<string, string> mime_decoder = {
        {"video/mpeg" , "avdec_mpeg2video"}, // 1,2
        {"video/x-vp8","avdev_vp8"},
        {"video/x-vp9","avdec_vp9"},
        {"video/x-wmv","avdec_wmv3"}, // 1,2,3
        {"video/x-h264","avdec_h264"},
        {"video/x-h265","avdec_h265"}
    };
    static map<string, string> rtp_payload_codecs = {
        {"video/x-h265","avdec_h265"}
    };
    static map<string, string> rtp_payload_formats = {
        {"MP2T","rtpmp2tdepay"},
        {"MP2T","rtpmp2tdepay"}
    };
    static map<string, string> mime_parser = {
        {"video/x-h264","h264parse"},
        {"video/x-h265","h265parse"},
        {"video/x-h264",""},
        {"",""},
        {"",""},
    };
    string pad_name(GstPad* p);
    string pad_mime_type(GstPad* p);
    string pad_caps_string(GstPad* p);
    string pad_proper_parser(GstPad* p);

    string find_parser_for_mime(const string& mime_type);
}
