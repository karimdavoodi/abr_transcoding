import os
import sys
import time
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib
SEG_PATH = './segments/'
ftyp = bytearray()
moov = bytearray()
moof = bytearray()
mdat = bytearray()
current_buff = bytearray()
m3u8_file = open('play.m3u8', 'wt')
m3u8_file.write("#EXTM3U\n" +
        "#EXT-X-VERSION:3\n" +
        "#EXT-X-TARGETDURATION:4\n" +
        "#EXT-X-PLAYLIST-TYPE:VOD\n" +
        "#EXT-X-I-FRAMES-ONLY\n"
        # "#EXT-X-DEFINE:NAME=\"PATH\",VALUE=\"%s\"\n" % SEG_PATH
        # Most player don't support EXT-X-DEFINE
        );

def read_header(data, start = 0):
    if len(data) < 8 : 
        return 0, ""
    size = int.from_bytes(data[start:start+4], "big")
    htype = data[start+4:start+8]
    #print("Header:",size, htype)
    return size, htype

def write_prev(box1, box2, file_name):
    print(f"Write {file_name}")
    if not os.path.exists(SEG_PATH):
        os.mkdir(SEG_PATH)
    with open(SEG_PATH +  file_name, "wb") as f:
        f.write(box1)
        f.write(box2)

def process_moof(moof):
    ''' 
    ref: gst-plugins-good/gst/isomp4/atoms.h
    ref: https://observablehq.com/@benjamintoofer/iso-base-media-file-format
    [moof] size=8+248
      [mfhd] size=12+4
        sequence number = 34
      [traf] size=8+224
        [tfhd] size=12+12, flags=28
          track ID = 1
          default sample duration = 100
          default sample flags = 100c0
        [tfdt] size=12+4
          base media decode time = 198000
        [trun] size=12+172, flags=a05
          sample count = 20
          data offset = 264
          first sample flags = 40
    '''
    print("process moof size:", len(moof)) 
    msize, t_moof = read_header(moof)
    if t_moof != b'moof' : 
        print("Invalid moof box")
        return -1, -1 

    frag_number = 0
    frag_sample_duration = 0
    frag_sample_size = 0
    frag_decode_time = 0
    frag_sample_count = 0
    track_id = 0

    size = 0
    idx = 8
    i = 0
    while i < 10: # Max box read
        i += 1
        idx += size 
        size, b_type = read_header(moof, idx)
        print(f"read {str(b_type)}:{size}")
        if b_type == b'mfhd':
            # mfhd = size(4) + type(4) + version(1) + flags(3) + frag_number(4)
            frag_number = int.from_bytes(moof[idx+12:idx+16], "big")
        elif b_type == b'traf':
            size = 8 # inc for inner box
        elif b_type == b'tfhd':
            track_id = int.from_bytes(moof[idx+12:idx+16], "big")
            flag = int.from_bytes(moof[idx+9:idx+12], "big")
            skip = 0
            if flag & 0x000001: skip += 8 # skip base_data_offset
            if flag & 0x000002: skip += 4 # skip sample_description_index
            if flag & 0x000008: 
                frag_sample_duration = int.from_bytes(moof[idx+16+skip:idx+16+skip+4], "big")
                skip += 4
            if flag & 0x000010: 
                frag_sample_size = int.from_bytes(moof[idx+16+skip:idx+16+skip+4], "big")
        elif b_type == b'tfdt':
            frag_decode_time = int.from_bytes(moof[idx+12:idx+16], "big")
            # TODO: start_time = frag_decode_time / time_scale 
            #                    (read time_scale from mdhd in moov)
        elif b_type == b'trun':
            frag_sample_count = int.from_bytes(moof[idx+12:idx+16], "big")
            break
    print(f"dur {frag_sample_duration}, count {frag_sample_count}, time {frag_decode_time}")
    return frag_number, frag_decode_time , (frag_sample_duration* frag_sample_count)/3000


def process_data(data):
    global ftyp, moof, moov, mdat, current_buff, m3u8_file

    current_buff += data 

    #if len(current_buff) > 8:
    current_size, current_type = read_header(current_buff) 

    if len(current_buff) >= current_size:
        print(f"Current: size {current_size}, type {current_type}")
        buff = current_buff[:current_size].copy()
        current_buff = current_buff[current_size:]
        if current_type == b'ftyp':   
            ftyp = buff
        elif current_type == b'moov': 
            moov = buff
        elif current_type == b'moof': 
            moof = buff
            if len(moov) != 0 and len(ftyp) != 0:
                write_prev(ftyp, moov, "init.mp4")
                m3u8_file.write("#EXT-X-MAP:URI=\"%sinit.mp4\"\n" % SEG_PATH)
                #m3u8_file.write("#EXT-X-MAP:URI=\"{$PATH}init.mp4\"\n")
                ftyp.clear()
                moov.clear()
        elif current_type == b'mdat': 
            mdat = buff
            frag_id, frag_time, frag_dur = process_moof(moof)
            seg_name = f"fragment_{frag_id}_{frag_time}_{frag_dur}.mp4"
            write_prev(moof, mdat, seg_name )
            m3u8_file.write("#EXTINF:%f,\n%s%s\n"%(frag_dur, SEG_PATH, seg_name));
            moof.clear()
            mdat.clear()
            #sys.exit(0)
        else:
            # ignore box
            pass
        

def on_buffer(sink, data):
    sample = sink.emit("pull-sample") 
        
    buffer = sample.get_buffer()
    #caps_format = sample.get_caps().get_structure(0) 
    size, offset, maxsize = buffer.get_sizes()
    frame_data = bytearray(buffer.extract_dup(offset, size))
    #print("Get data:", len(frame_data))
    process_data(frame_data)
    return Gst.FlowReturn.OK


def bus_call(bus, message, loop):
    global pipe
    t = message.type
    if t == Gst.MessageType.EOS:
        loop.quit()
    elif t == Gst.MessageType.ERROR:
        err, debug = message.parse_error()
        loop.quit()
    return True

pipeline_cmd =(
        "videotestsrc pattern=ball num-buffers=2000 " + 
        "! queue " +
        "! x264enc key-int-max=60 " +
        "! video/x-h264, framerate=(fraction)30/1, profile=baseline "
        "! mp4mux fragment-duration=2000 " +
        "! appsink emit-signals=True name=sink sync=False "
        ) 

loop = GLib.MainLoop()
print(pipeline_cmd)
Gst.init(sys.argv)
pipeline = Gst.parse_launch(pipeline_cmd)
appsink = pipeline.get_by_name('sink')
appsink.connect("new-sample", on_buffer, None)
pipeline.set_state(Gst.State.PLAYING)
bus = pipeline.get_bus()
bus.add_signal_watch()
bus.connect ("message", bus_call, loop)
loop.run()
m3u8_file.write("\n#EXT-X-ENDLIST\n")
m3u8_file.close()
