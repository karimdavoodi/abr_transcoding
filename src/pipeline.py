import os
import sys
import logging
import gi
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst


loop = None
pipe = None
bus = None
error = False
streams = {
        'audios':[]
        }

def loop_quit(has_error = False):
   global loop, error
   if loop:
       loop.quit()
       error = has_error

def bus_call(bus, message, loop):
    global pipe
    t = message.type
    if t == Gst.MessageType.EOS:
        logging.info("End-of-stream")
        loop.quit()
    elif t == Gst.MessageType.ERROR:
        err, debug = message.parse_error()
        logging.error("Error: %s: %s\n" % (err, debug))
        if pipe:
            os.environ["GST_DEBUG_DUMP_DOT_DIR"] = "/tmp"
            Gst.debug_bin_to_dot_file(pipe, Gst.DebugGraphDetails.ALL, "gst_error");
            logging.info("Make dot file at /tmp/get_error.dot")
        loop.quit()
    return True

def init():
    global pipe, loop, bus

    
    GObject.threads_init()
    Gst.init(None)

    Gst.debug_set_active(True)
    Gst.debug_set_default_threshold(3)
    
    loop = GObject.MainLoop()
    pipe = Gst.Pipeline.new()
    bus = pipe.get_bus()
    bus.add_signal_watch()
    bus.connect ("message", bus_call, loop)

def loop_start():
    global pipe, loop, bus

    pipe.set_state(Gst.State.PLAYING)
    try:
        loop.run()
    except:
        pass
    pipe.set_state(Gst.State.NULL)

