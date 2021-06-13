#pragma once

#include <map>
#include <string>
#include <gst/gst.h>
#include "element.h"

class Bin {
    public:
        Bin() = delete;
        Bin(const std::string&, GMainLoop*) noexcept;
        GstElement* add_element(const std::string& type, const std::string& name, 
                                                bool set_state = false);
        int elements_count(){ return elements.size(); }
        Element& get_element(const std::string& name) { return elements[name]; }
        Element& get_bin() { return bin; }
        std::string get_name() const { return name; }
        bool good() { return bin.get_ptr() == nullptr ? false : true; }
        void main_loop_quit(){ if(loop) g_main_loop_quit(loop); }
        virtual ~Bin();

    protected:
        GMainLoop* loop;
        std::string name;
        Element bin;
        std::map<std::string ,Element> elements;
};
