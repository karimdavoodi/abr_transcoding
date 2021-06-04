#include <boost/format/format_fwd.hpp>
#include <iostream>
#include <map>
#include <boost/format.hpp>

#include "util.h"
#include "config.h"
#include "av_input.h"

using namespace std;

namespace Util
{

    void read_env_variables(std::map<std::string, std::string> &env_vars)
    {
        for (const auto &[key, val] : env_vars)
        {
            const char *new_val = std::getenv(key.c_str());
            if (new_val != NULL)
            {
                env_vars[key] = new_val;
                LOG(info) << "Set env var: " << key << ":" << new_val;
            }
            else
            {
                LOG(info) << "Not Set env var: " << key << ":" << val;
            }
        }
    }
    void read_abr_env_variables(std::map<std::string, std::string> &env_vars)
    {
        for (size_t i = 0; i < 100; ++i)
        {
            string key_size = "ABR_" + to_string(i) + "_SIZE";
            string key_bitrate = "ABR_" + to_string(i) + "_BITRATE";
            const char *val_size = std::getenv(key_size.c_str());
            const char *val_bitrate = std::getenv(key_bitrate.c_str());
            if (val_size != NULL && val_bitrate != NULL)
            {
                env_vars[key_size] = val_size;
                env_vars[key_bitrate] = val_bitrate;
                LOG(info) << "Set env var: " << key_size << ":" << val_size;
                LOG(info) << "Set env var: " << key_bitrate << ":" << val_bitrate;
            }
        }
    }
    std::string generate_content_json_metadata(std::map<std::string, std::string> &vars)
    {
        string out;
        auto out_fmt = boost::format("{"
                "\"id\":\"%s\",\n"
                "\"input\":\"%s\",\n"
                "\"segment_duration\":\"%s\",\n"
                "\"format\":\"%s\",\n"
                "\"vcodec\":\"%s\",\n"
                "\"acodec\":\"%s\",\n") 
            % vars["ABR_CONTENT_ID"]
            % vars["ABR_INPUT_SOURCE"]
            % vars["ABR_SEGMENT_LEN"]
            % vars["ABR_OUTPUT_FORMAT"]
            % vars["ABR_OUTPUT_VCODEC"]
            % vars["ABR_OUTPUT_ACODEC"];
        out = out_fmt.str();
        string v_abr;
        for (size_t i = 0; i < 100; ++i)
        {
            string key_size = "ABR_" + to_string(i) + "_SIZE";
            string key_bitrate = "ABR_" + to_string(i) + "_BITRATE";
            if(vars.count(key_size)){
                auto abr = (boost::format("{\n"
                        "\t\"size\":\"%s\",\n"
                        "\t\"bitrate\":\"%s\",\n"
                        " }") % vars[key_size] % vars[key_bitrate]).str();
                if(!v_abr.empty()) 
                    v_abr += ",";
                v_abr += abr;
            }else break;
        }
        out = out + "\"ABR\":[" + v_abr + "]";
        LOG(info) << "Record:" << out;
        return out;
    }
    void auto_scale_vars(std::map<std::string, std::string> &env_vars, AvInput &input)
    {
        if (!input.get_width() || !input.get_height())
        {
            LOG(info) << "Input resolution not probe!";
            return;
        }
        env_vars["ABR_0_SIZE"] = to_string(input.get_width()) +
                                 "x" + to_string(input.get_height());
        env_vars["ABR_0_BITRATE"] = to_string(input.get_bitrate());

        int i = 1;
        int h = input.get_height() / 2;
        int w = input.get_width() / 2;
        int b = input.get_bitrate() / 2;
        while (h >= 240)
        {
            string key_size = "ABR_" + to_string(i) + "_SIZE";
            string key_bitrate = "ABR_" + to_string(i) + "_BITRATE";
            env_vars[key_size] = to_string(w) + "x" + to_string(h);
            env_vars[key_bitrate] = to_string(b);
            LOG(info) << "Get var " << key_size << ":" << env_vars[key_size];
            LOG(info) << "Get var " << key_bitrate << ":" << env_vars[key_bitrate];
            h = h / 2;
            w = w / 2;
            i++;
        }
    }
    bool packet_is_audio(AVPacket *pkt) { return true; }
    bool packet_is_video(AVPacket *pkt) { return true; }

}
