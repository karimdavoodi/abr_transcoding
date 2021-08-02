import logging
import random
from gi.repository import Gst

import util
import pipeline


class Bin_decode_stream:
    
    def __init__(self, stream_type):
        bin_id = random.randint(0,1000)
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
        self.tee = Gst.ElementFactory.make("tee")
        self.tee.set_state(Gst.State.PLAYING)              
        self.bin.add(self.tee)
        self.decoder.link(self.tee)
        for abr in util.output_videos:
            tee_src = self.tee.get_request_pad('src_%u') 
            fake = Gst.ElementFactory.make("fakesink")
            fake.set_state(Gst.State.PLAYING)              
            self.bin.add(fake)
            tee_src.link(fake.get_static_pad('sink'))
            logging.info(f"Make fake for {abr['size']}")
            
    
