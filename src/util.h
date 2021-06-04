#pragma once
#include <iostream>
#include <map>

#include "av_input.h"

struct AVPacket;
struct AVFrame;

namespace Util {
    void read_env_variables(std::map<std::string, std::string>& vars);
    void read_abr_env_variables(std::map<std::string, std::string>& vars);
    std::string generate_content_json_metadata(std::map<std::string, std::string>& vars);
    void  auto_scale_vars(std::map<std::string, std::string>& vars, AvInput& input);
    bool packet_is_audio(AVPacket* pkt);
    bool packet_is_video(AVPacket* pkt);
}
