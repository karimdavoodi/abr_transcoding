import logging
from gi.repository import GLib, GObject, Gst

import util
import pipeline

bin_id = 0

class Bin_decode_stream:
    
    def __init__(self, stream_type):
        global bin_id
        logging.info(str(pipeline.streams))
        bin_id += 1
        self.stream_type = stream_type
        self.bin = Gst.Bin.new("bin_decode_" + stream_type + str(bin_id))
        self.decoder = Gst.ElementFactory.make("decodebin")
        self.decoder.set_state(Gst.State.PLAYING)              
        self.bin.set_state(Gst.State.PLAYING)              
        self.bin.add(self.decoder)
        
        self.decoder.connect("pad-added", self.decoder_pad_add)
        ghost_sink = Gst.GhostPad.new("sink", self.decoder.get_static_pad('sink'))
        self.bin.add_pad(ghost_sink)
        logging.info("Init bin_decode_stream for " + stream_type)
    
    def get_sink_pad(self):
        return self.bin.get_static_pad('sink')

    def decoder_pad_add(self, elm, pad):
        caps =  pad.query_caps()[0]
        logging.info(caps.to_string())
        logging.info("Add pad in decodebin, caps:" + caps.to_string())
        self.fakesink = Gst.ElementFactory.make("fakesink")
        self.fakesink.set_state(Gst.State.PLAYING)              
        self.bin.add(self.fakesink)
        self.decoder.link(self.fakesink)
    
