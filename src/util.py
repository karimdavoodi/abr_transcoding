import os
import sys
import logging


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
        #ABR_1_SIZE="1280x720"
        #ABR_1_BITRATE="6000000"
        }

input_streams = {
        'video_heigh':0, 
        'video_width':0, 
        'video_profile':'',
        'video_level':'', 
        'video_mime':'',
        'audios':[]
    }
output_videos = [

        ]
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
    
    for i in range(0,20):
        size = os.getenv(f"ABR_{i}_SIZE", '')
        bitrate = os.getenv(f"ABR_{i}_BITRATE", '')
        if size != '' and bitrate != '':
            args[f"ABR_{i}_SIZE"] = size
            args[f"ABR_{i}_BITRATE"] = bitrate

    # Get from command line
    if len(sys.argv) > 1:
        for i in range(1, len(sys.argv)):
            tok = sys.argv[i].split('=')
            if len(tok) == 2:
                args[tok[0]] = tok[1].rstrip()
            
def set_video_auto_sacel():
    
    heigh = input_streams['video_heigh']
    width = input_streams['video_width']

    if args['ABR_AUTO_SCALE'] == 'no':
        for i in range(0,20):
            if args[f"ABR_{i}_SIZE"] == 'original':
                args[f"ABR_{i}_SIZE"] = f"{heigh}x{width}"
                break
        return

    if input_streams['video_heigh'] == 0 or input_streams['video_width'] == 0:
        logging.error("Can't get input stream Video resolution!")
        return
     
    i = 0
    while heigh > 240:
        bitrate = heigh * width * 3
        args[f"ABR_{i}_SIZE"] = f"{heigh}x{width}"
        args[f"ABR_{i}_BITRATE"] = f"{bitrate}"
        heigh /= 2
        width /= 2
        i -= 1

def set_video_abr():
    
    set_video_auto_sacel()

    for i in range(0,20):
        output_videos.append({
            'size': args[f"ABR_{i}_SIZE"],
            'bitrate': args[f"ABR_{i}_BITRATE"]
            })
    logging.info(str(output_videos))

