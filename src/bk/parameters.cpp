#include "log.h"
#include "parameters.h"
#include <string>
using std::string;

void Parameters::get_environments()
{
        for (const auto &[key, val] : vars)
        {
            const char *new_val = std::getenv(key.c_str());
            if (new_val != NULL)
            {
                vars[key] = new_val;
                LOG(info) << "Set env var: " << key << ":" << new_val;
            }
            else
            {
                LOG(info) << "Not Set env var: " << key << ":" << val;
            }
        }
    if (vars["ABR_AUTO_SCALE"] == "no") {
        for (size_t i = 0; i < 100; ++i)
        {
            string key_size = "ABR_" + std::to_string(i) + "_SIZE";
            string key_bitrate = "ABR_" + std::to_string(i) + "_BITRATE";
            const char *val_size = std::getenv(key_size.c_str());
            const char *val_bitrate = std::getenv(key_bitrate.c_str());
            if (val_size != NULL && val_bitrate != NULL)
            {
                vars[key_size] = val_size;
                vars[key_bitrate] = val_bitrate;
                LOG(info) << "Set env var: " << key_size << ":" << val_size;
                LOG(info) << "Set env var: " << key_bitrate << ":" << val_bitrate;
            }
        }
    }

}
void Parameters::get_commandline(const char* argv[])
{   
    for(size_t i=0; argv[i][0] != '\0'; ++i){
        string str = argv[i];
        auto pos = str.find('=');
        if(pos != std::string::npos){
            vars[ str.substr(0, pos)] = str.substr(pos+1); 
        }
    }
}
std::string Parameters::to_string()
{
    string out;
    for(auto &[key, val] : vars){
        out += key + "=" + val + "\n";
    }
    return out;
}
