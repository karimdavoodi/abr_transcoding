#include "log.h"
#include "bin.h"

Bin::Bin(const std::string &bin_name, GMainLoop* _loop) noexcept
{
    loop = _loop;
    name = bin_name;
    bin.set_ptr(gst_bin_new(name.c_str()));
    
    if (!bin.good())
    {
        LOG(error) << "Can't create bin!";
        return;
    }

}
GstElement* Bin::add_element(const std::string &plugin, const std::string &pname, bool set_state)
{
    Element element;

    if (pname.empty())
    {
        LOG(error) << "Name of element is empty!";
        return nullptr;
    }
    if (!element.make(plugin, pname, set_state))
    {
        LOG(error) << "Element type not found:" << plugin;
        return nullptr;
    }
    elements[pname] = element;

    if(!gst_bin_add(GST_BIN(bin.get_ptr()), element.get_ptr())){
        LOG(error) << "Can't add Element " << pname << " to bin " << name;
        return nullptr;
    }
    return element.get_ptr();
    
}
Bin::~Bin()
{

    // if (bin.get_ptr())
    // {
    //     gst_element_set_state(GST_ELEMENT(bin.get_ptr()), GST_STATE_NULL);
    //     gst_object_unref(bin.get_ptr());
    // }
}
