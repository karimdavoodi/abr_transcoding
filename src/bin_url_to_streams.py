import logging
from gi.repository import Gst

import util
import pipeline

'''
    (url) => urisourcebin -> queue -> demuxer --> parserbin => (streams)

'''
class Bin_url_to_streams:
    def __init__(self, url, func_have_video, func_have_audio):
        logging.info("start  bin_url_to_streams")
        
        self.pad_num = 0

        # callback function when streams are ready
        self.func_have_video = func_have_video
        self.func_have_audio = func_have_audio
        
        self.bin = Gst.Bin.new("bin_url_to_streams")
        self.urisourcebin = Gst.ElementFactory.make("urisourcebin")
        self.queue = Gst.ElementFactory.make("queue")

        self.bin.add(self.urisourcebin)
        self.bin.add(self.queue)
        

        self.urisourcebin.set_property('uri', url)
        self.urisourcebin.connect('pad-added', self.urisourcebin_pad_add)
        logging.info("init bin_url_to_streams")

    def urisourcebin_pad_add(self, elm, pad):
        self.print_pad_info(pad, 'urisourcebin')

        logging.info(f"LINK: urisourcebin {pad.get_name()} -->  queue")
        pad.link(self.queue.get_static_pad('sink'))
        queue_src = self.queue.get_static_pad('src')

        cap =  pad.query_caps()[0]
        pad_mime = cap.get_name()
        if util.mime_demuxer.get(pad_mime):
            demux_name = util.mime_demuxer.get(pad_mime)
            self.demuxer = Gst.ElementFactory.make(demux_name, "url_demux")
            self.demuxer.set_state(Gst.State.PLAYING)              
            self.bin.add(self.demuxer)
            logging.info(f"LINK: queue --> {demux_name}")
            self.queue.link(self.demuxer)
            self.demuxer.connect("pad-added", self.demuxer_pad_add)
        elif util.mime_decoder.get(pad_mime):
            self.send_stream_to_parser(queue_src)
        elif pad_mime == 'application/x-rtp':
            encodin_name = cap.get_structure(0).get('encodin-name','')
            if encodin_name == 'MP2T':
                self.rtmpdepay = Gst.ElementFactory.make("rtpmp2tdepay", "url_rtmpdepay")
                self.rtmpdepay.set_state(Gst.State.PLAYING)              
                self.demuxer = Gst.ElementFactory.make("tsdemux", "url_demux")
                self.demuxer.set_state(Gst.State.PLAYING)          
                self.bin.app(self.rtmpdepay)
                self.bin.app(self.demuxer)
                logging.info(f"LINK: queue --> rtmpdepay -->  tsdemux")
                self.queue.link(self.rtmpdepay)
                self.rtmpdepay.link(self.demuxer)
                self.demuxer.connect("pad-added", self.demuxer_pad_add)
            elif encodin_name == 'H264' or encodin_name == 'H265':
                rtmpdepay_name = 'rtph264depay' if encodin_name == 'H264' else 'rtph265depay'
                self.rtmpdepay = Gst.ElementFactory.make(rtmpdepay_name, "url_rtmpdepay")
                self.rtmpdepay.set_state(Gst.State.PLAYING)   
                self.bin.app(self.rtmpdepay)
                logging.info(f"LINK: queue --> rtph264depay")
                self.queue.link(self.rtmpdepay)
                rtmpdepay_src = self.rtmpdepay.get_static_pad('src')
                self.send_stream_to_parser(rtmpdepay_src)
            else: 
                logging.error(f"Not support RTP type: {encodin_name}")
                pipeline.loop_quit(True)
        else:
            logging.error(f"Not support type: {pad_mime}")
            pipeline.loop_quit(True)


    def print_pad_info(self, pad, loc):
        cap =  pad.query_caps()[0].to_string()
        logging.info(f"Pad in {loc!r}: {pad.get_name()}: Caps: {cap}")

    def parsebin_pad_add(self, elm, pad):
        self.print_pad_info(pad, 'Parser')
        
        # Link pad to bin ghost pad 
        pad_name = 'src_' + str(self.pad_num)
        self.pad_num += 1
        ghost_pad = Gst.GhostPad.new(pad_name, pad)
        self.bin.add_pad(ghost_pad)
       
        # fill streams record
        struct = pad.query_caps().get_structure(0)
        pad_mime = struct.get_name()
        if 'video' in pad_mime:
            util.input_streams['video_heigh'] = struct.get_int("heigh").value
            util.input_streams['video_width'] = struct.get_int("width").value
            util.input_streams['video_profile'] = struct.get_string("profile")
            util.input_streams['video_level'] = struct.get_string("level")
            util.input_streams['video_mime'] = struct.get_name()
            self.func_have_video(pad_name, self)
        elif 'audio' in pad_mime:
            util.input_streams['audios'].append({
                'mime': struct.get_name(),
                'mpegversion': struct.get_int("mpegversion").value,
                'level': struct.get_int("level").value,
                'channels': struct.get_int("channels").value,
                'rate': struct.get_int("rate").value
                })
            self.func_have_audio(pad_name, self)


    def send_stream_to_parser(self, pad):
        #self.print_pad_info(pad, 'find')
        self.parsebin = Gst.ElementFactory.make("parsebin")
        self.parsebin.set_state(Gst.State.PLAYING)              
        self.bin.add(self.parsebin)
        pad.link(self.parsebin.get_static_pad('sink'))
        self.parsebin.connect("pad-added", self.parsebin_pad_add)

    def demuxer_pad_add(self, elm, pad):
        self.print_pad_info(pad, 'demuxer')
        pad_name = pad.get_name()
        self.send_stream_to_parser(pad)

