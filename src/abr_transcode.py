#!/usr/bin/env python3
import logging
import gi
gi.require_version('Gst', '1.0')
gi.require_version('GLib', '2.0')
gi.require_version('GObject', '2.0')
from gi.repository import GLib, GObject, Gst

import util
import pipeline
import bin_encode_stream   
import bin_url_to_streams  
import bin_decode_stream  
import bin_stream_to_cmaf  

def have_video(pad_name, bin_url):
    logging.info("Have video")

    util.set_video_abr()

    bin_decode = bin_decode_stream.Bin_decode_stream('video')
    pipeline.pipe.add(bin_decode.bin)
    src_pad = bin_url.bin.get_static_pad(pad_name)
    sink_pad = bin_decode.get_sink_pad()
    src_pad.link(sink_pad)

    
def have_audio(pad_name, bin_url):
    bin_decode = bin_decode_stream.Bin_decode_stream('audio')
    pipeline.pipe.add(bin_decode.bin)

    src_pad = bin_url.bin.get_static_pad(pad_name)
    sink_pad = bin_decode.get_sink_pad()
    src_pad.link(sink_pad)


def run(args):
    logging.info("Start abr")
    pipeline.init()

    bin_url = bin_url_to_streams.Bin_url_to_streams(
            args['ABR_INPUT_SOURCE'], 
            have_video, 
            have_audio)
     
    pipeline.pipe.add(bin_url.bin)

    pipeline.loop_start()
