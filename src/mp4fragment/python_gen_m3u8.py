
import m3u8
from  mpegdash.parser import MPEGDASHParser

def parse_dash():

    mpd = MPEGDASHParser.parse('/tmp/dash/p1.mpd')
    MPEGDASHParser.write(mpd, '/tmp/pi.mpd')
    return
    '''

    for period in mpd.periods:
        for adapt_set in period.adaptation_sets:
            for rep in adapt_set.representations:
                for temp in rep.segment_templates:
                    for timel in temp.segment_timelines:
    '''
                        
    
parse_dash()

def gen_playlist():
    playlist = m3u8.M3U8()
    playlist.version = 6
    playlist.media_sequence = 0
    playlist.target_duration = 10
    playlist.playlist_type = 'VOD'
    playlist.is_endlist = True
    for i in range(5):
        segment = m3u8.Segment(uri='1.ts',duration=4.5, title = f"name_{i}")
        playlist.add_segment(segment)
    print(playlist.dumps())

def gen_master_playlist():
    variant_m3u8 = m3u8.M3U8()
    subtitles = m3u8.Media('english_sub.m3u8', 'SUBTITLES', 'subs', 'en',
                           'English', 'YES', 'YES', 'NO', None)
    variant_m3u8.add_media(subtitles)
    
    low_playlist = m3u8.Playlist(
        uri='video-800k.m3u8',
        stream_info={'bandwidth': 800000,
                     'program_id': 1,
                     'resolution': '624x352',
                     'codecs': 'avc1.4d001f, mp4a.40.5',
                     'subtitles': 'subs'},
        media=[subtitles],
        base_uri='http://example.com/'
    )
    high_playlist = m3u8.Playlist(
        uri='video-1200k.m3u8',
        stream_info={'bandwidth': 1200000,
                     'program_id': 1,
                     'codecs': 'avc1.4d001f, mp4a.40.5',
                     'subtitles': 'subs'},
        media=[subtitles],
        base_uri='http://example.com/'
    )
    low_iframe_playlist = m3u8.IFramePlaylist(
        uri='video-800k-iframes.m3u8',
        iframe_stream_info={'bandwidth': 151288,
                            'program_id': 1,
                            'closed_captions': None,
                            'resolution': '624x352',
                            'codecs': 'avc1.4d001f'},
        base_uri='http://example.com/'
    )
    high_iframe_playlist = m3u8.IFramePlaylist(
        uri='video-1200k-iframes.m3u8',
        iframe_stream_info={'bandwidth': 193350,
                            'codecs': 'avc1.4d001f'},
        base_uri='http://example.com/'
    )

    variant_m3u8.add_playlist(low_playlist)
    variant_m3u8.add_playlist(high_playlist)
    variant_m3u8.add_iframe_playlist(low_iframe_playlist)
    variant_m3u8.add_iframe_playlist(high_iframe_playlist)

    print(variant_m3u8.dumps())

#gen_playlist()
#gen_master_playlist()
