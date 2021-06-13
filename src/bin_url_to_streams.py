import logging
from gi.repository import Gst

import util
import pipeline


bin = None
urisourcebin = None 
queue = None
typefind = None 
demuxer = None

have_video = None
have_audio = None
pad_num = 0

def find_new_pad(pad, pad_type):
    parser = find_parser(pad.get_name())
    ...


def find_new_pad0(pad, pad_type):
    global bin, pad_num
    
    # Ghost pad as 'src' 
    pad_name = 'src_' + str(pad_num)
    pad_num += 1
    ghost_pad = Gst.GhostPad.new(pad_name, pad)
    bin.add_pad(ghost_pad)

    cap =  pad.query_caps()[0].to_string()
    logging.info(f"HAVE PAD Caps:{cap}")
    if pad_type == 'video':
        struct = pad.query_caps().get_structure(0)
        width = struct.get_int("width").value
        height = struct.get_int("height").value
        level = struct.get_string("level")
        profile = struct.get_string("profile")
        pipeline.streams['video_h'] = height
        pipeline.streams['video_w'] = width 
        pipeline.streams['video_profile'] = profile 
        pipeline.streams['video_level'] = level
        pipeline.streams['video_codec'] = struct.get_name()
        have_video(pad_name)
    elif pad_type == 'audio':
        struct = pad.query_caps().get_structure(0)
        mpegversion = struct.get_int("mpegversion").value
        level = struct.get_int("level").value
        channels = struct.get_int("channels").value
        audio_codec = struct.get_name()
        pipeline.streams['audios'].append({
            'codec': audio_codec,
            'mpegversion': mpegversion,
            'level': level,
            'channels': channels
            })
        have_audio(pad_name)


def urisourcebin_pad_add(elm, pad):
    global bin, queue 
    cap =  pad.query_caps()[0].to_string()
    logging.info(f"Add pad in urisourcebin {pad.get_name()}: Caps: {cap}")
     
    queue_sink = queue.get_static_pad('sink')
    if not queue_sink:
        logging.error("Can't get queue sink pad")
    pad.link(queue.get_static_pad('sink'))

def demuxer_pad_add(elm, pad):
    pad_name = pad.get_name()
    logging.info(f"Add pad in demuxer: {pad_name}")
    if 'video' in pad_name:
        find_new_pad(pad, 'video')
    elif 'audio' in pad_name:
        find_new_pad(pad, 'audio')


def typefind_have_type(elm, arg0, cap):
    global bin, demuxer, have_video, have_audio 

    pad_cap = cap.to_string()
    pad_mime = cap.get_structure(0).get_name()
    logging.info(f"Add pad in typefind Caps:{pad_cap}, mime:{pad_mime}")
    if util.mime_demuxer.get(pad_mime):
        demux_name = util.mime_demuxer.get(pad_mime)
        demuxer = Gst.ElementFactory.make(demux_name, "url_demux")
        demuxer.set_state(Gst.State.PLAYING)              
        bin.add(demuxer)
        typefind.link(demuxer)
        demuxer.connect("pad-added", demuxer_pad_add)
        logging.info(f"link typefind to demuxer: {demux_name}")
    elif util.mime_decoder.get(pad_mime):
        typefind_src = typefind.get_static_pad('src')
        find_new_pad(typefind_src, 'video')
    elif pad_mime == 'application/x-rtp':
        encodin_name = cap.get_structure(0).get('encodin-name','')
        if encodin_name == 'MP2T':
            rtmpdepay = Gst.ElementFactory.make("rtpmp2tdepay", "url_rtmpdepay")
            rtmpdepay.set_state(Gst.State.PLAYING)              
            demuxer = Gst.ElementFactory.make("tsdemux", "url_demux")
            demuxer.set_state(Gst.State.PLAYING)          
            bin.app(rtmpdepay)
            bin.app(demuxer)
            typefind.link(rtmpdepay)
            rtmpdepay.link(demuxer)
            #Gst.ElementFactory.gst_element_link_many(typefind, rtmpdepay, demuxer) 
            demuxer.connect("pad-added", demuxer_pad_add)
            logging.info(f"link typefind to rtmpdepay and demuxer")
        elif encodin_name == 'H264' or encodin_name == 'H265':
            rtmpdepay_name = 'rtph264depay' if encodin_name == 'H264' else 'rtph265depay'
            rtmpdepay = Gst.ElementFactory.make(rtmpdepay_name, "url_rtmpdepay")
            rtmpdepay.set_state(Gst.State.PLAYING)   
            bin.app(rtmpdepay)
            typefind.link(rtmpdepay)
            rtmpdepay_src = rtmpdepay.get_static_pad('src')
            find_new_pad(rtmpdepay_src, 'video')
        else:
            logging.error(f"Not support RTP type: {encodin_name}")
            pipeline.loop_quit(True)
    else:
        logging.error(f"Not support type: {pad_mime}")
        pipeline.loop_quit(True)
            

def init(uri, _have_video, _have_audio):
    global bin, urisourcebin, queue, typefind, have_video, have_audio 

    have_video = _have_video
    have_audio = _have_audio

    logging.info("start  bin_url_to_streams")
    bin = Gst.Bin.new("bin_url_to_streams")
    urisourcebin = Gst.ElementFactory.make("urisourcebin")
    queue = Gst.ElementFactory.make("queue")
    typefind = Gst.ElementFactory.make("typefind")

    bin.add(urisourcebin)
    bin.add(queue)
    bin.add(typefind)
     
    queue.link(typefind) 

    urisourcebin.set_property('uri', uri)
    urisourcebin.connect('pad-added', urisourcebin_pad_add)
    typefind.connect('have-type', typefind_have_type)
    logging.info("init bin_url_to_streams")

