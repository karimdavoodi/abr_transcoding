#pragma once

#include <vector>
#include <string>
#include <functional>
#include <gst/gst.h>
#include "bin.h"
using std::string;

/*
 *       urlbin --> typefind --> demux --> 
 *                                          video parser --> video_tee 
 *                                          audio parser --> audio_tee
 */
class Bin_url_to_estreams : public Bin {
    public:
        Bin_url_to_estreams(const std::string& url, GMainLoop* loop): 
            Bin("bin_url_to_estreams", loop), url(url) {}
        bool build(std::function<void(GstPad*)>, std::function<void(GstPad*)>);

        std::function<void(GstPad*)> has_video;
        std::function<void(GstPad*)> has_audio;

    private:
        std::string url;
         

};
