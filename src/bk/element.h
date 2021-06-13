#pragma once
#include <iostream>
#include <string>
#include <gst/gst.h>

class Element {
    public:
        std::string get_name() const { return name; }
        std::string get_type() const { return type; }
        GstElement* get_ptr() const { return elm; }
        void set_ptr(GstElement* e)  { elm = e; }
        bool good() { return elm != nullptr; }
        bool make(const std::string&, const std::string&, bool set_state=false);
        bool link_pad_to_static_sink(GstPad* pad, const std::string& sink_name);
        ~Element(){
            //if(elm) gst_object_unref(elm);
            //elm = nullptr;
        }
    private:
        GstElement* elm = nullptr;
        std::string name;
        std::string type;
};
