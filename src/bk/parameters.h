#pragma once
#include <iostream>
#include <string>
#include <map>


class Parameters {
    private:
        std::map<std::string, std::string> vars;
    public:
        Parameters(){
            vars = {
                {"ABR_CONTENT_ID", ""},
                {"ABR_INPUT_SOURCE", ""},

                {"ABR_SEGMENT_LEN", "4"},
                {"ABR_OUTPUT_FORMAT", "fmp4"},
                {"ABR_OUTPUT_VCODEC", "h264"},
                {"ABR_OUTPUT_ACODEC", "aac"},
                {"ABR_AUTO_SCALE", "yes"}       // generate all small resolution till 240p (divide 2)
                //ABR_0_SIZE="original"
                //ABR_0_BITRATE="original"
                //ABR_1_SIZE="FHD"
                //ABR_1_BITRATE="6000000"
            };
        }
        void get_environments();
        void get_commandline(const char* argv[]);

        void set_var(const std::string& var, const std::string& val) { 
            vars[var] = val;
        }
        std::string get_var(const std::string& var) { 
            return vars.count(var) ? vars[var] : "";        
        }
        std::string to_string();
};
