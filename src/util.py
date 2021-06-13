import os
import sys

args = { 
        "ABR_CONTENT_ID":"",
        "ABR_INPUT_SOURCE":"",
        "ABR_SEGMENT_LEN":"4",
        "ABR_OUTPUT_FORMAT":"fmp4",
        "ABR_OUTPUT_VCODEC":"h264",
        "ABR_OUTPUT_ACODEC":"aac",
        "ABR_AUTO_SCALE":"yes"   
        #ABR_0_SIZE="original"
        #ABR_0_BITRATE="original"
        #ABR_1_SIZE="FHD"
        #ABR_1_BITRATE="6000000"
        }

mime_demuxer = {
        "video/mpegts": "tsdemux",
        "video/quicktime": "qtdemux",
        "video/x-matroska": "matroskademux",
        "video/x-msvideo": "matroskademux",
        "video/x-ms-asf": "asfdemux",
        "video/x-flv": "flvdemux"
    }
mime_decoder = {
        "video/mpeg": "avdec_mpeg2video",
        "video/x-vp8": "avdev_vp8",
        "video/x-vp9": "avdec_vp9",
        "video/x-wmv": "avdec_wmv3", 
        "video/x-h264": "avdec_h264",
        "video/x-h265": "avdec_h265"
    }
def get_parameters():
    # Get from environment
    for key in args:
        val = os.getenv(key, '')
        if val != '':
            args[key] = val
    # Get from command line
    if len(sys.argv) > 1:
        for i in range(1, len(sys.argv)):
            tok = sys.argv[i].split('=')
            if len(tok) == 2:
                args[tok[0]] = tok[1].rstrip()
            


