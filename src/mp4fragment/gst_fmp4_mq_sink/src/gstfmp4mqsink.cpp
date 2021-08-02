/*
 *    Copyright (c) 2020 Karim <<karimdavoodi@gmail.com>>
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy
 *    of this software and associated documentation files (the "Software"), to deal
 *    in the Software without restriction, including without limitation the rights
 *    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the Software is
 *    furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */

#include <gst/gst.h>
#include "gstfmp4mqsink.hpp"
#include "amqp_client.hpp"
#include "bento4_util.hpp"

    

GST_DEBUG_CATEGORY_STATIC (gst_fmp4mqsink_debug);
#define GST_CAT_DEFAULT gst_fmp4mqsink_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("video/quicktime")
        );

#define gst_fmp4mqsink_parent_class parent_class
G_DEFINE_TYPE (GstFmp4MqSink, gst_fmp4mqsink, GST_TYPE_ELEMENT);

static void gst_fmp4mqsink_set_property (GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec);
static void gst_fmp4mqsink_get_property (GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec);

static gboolean gst_fmp4mqsink_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_fmp4mqsink_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);

static GstStateChangeReturn gst_fmp4mqsink_change_state (GstElement *element, 
        GstStateChange transition)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    GstFmp4MqSink *filter = GST_FMP4MQSINK (element);
    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            amqp_client_init(filter);
            break;
        default:
            break;
    }
    ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
    if (ret == GST_STATE_CHANGE_FAILURE)
        return ret;
    switch (transition) {
        case GST_STATE_CHANGE_READY_TO_NULL:
            amqp_client_close(filter);
            break;
        default:
            break;
    }
    return ret;
}
static void gst_fmp4mqsink_class_init (GstFmp4MqSinkClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;

    gobject_class->set_property = gst_fmp4mqsink_set_property;
    gobject_class->get_property = gst_fmp4mqsink_get_property;
    gstelement_class->change_state = gst_fmp4mqsink_change_state;

    g_object_class_install_property (gobject_class, PROP_CONTENT_ID,
            g_param_spec_string ("content_id", "Content ID", 
                "Content ID of stream", "", G_PARAM_READWRITE ));
    g_object_class_install_property (gobject_class, PROP_MQ_HOST,
            g_param_spec_string ("mq_host", "RabbitMq host", 
                "Host address of RabbitMq server", "", G_PARAM_READWRITE ));
    g_object_class_install_property (gobject_class, PROP_MQ_PORT,
            g_param_spec_string ("mq_port", "RabbitMq port", 
                "Port of RabbitMq server", "", G_PARAM_READWRITE ));
    g_object_class_install_property (gobject_class, PROP_MQ_USER,
            g_param_spec_string ("mq_user", "RabbitMq user", 
                "User of RabbitMq server", "", G_PARAM_READWRITE ));
    g_object_class_install_property (gobject_class, PROP_MQ_PORT,
            g_param_spec_string ("mq_pass", "RabbitMq password", 
                "Password of RabbitMq server", "", G_PARAM_READWRITE ));

    gst_element_class_set_details_simple(gstelement_class,
            "Mp4 fragment to RabbitMq",
            "Sink",
            "Push fmp4 fragment to RabbitMq",
            "Karim Davoodi <<karimdavoodi@gmail.com>>");

    gst_element_class_add_pad_template (gstelement_class,
            gst_static_pad_template_get (&sink_factory));
}

static void gst_fmp4mqsink_init (GstFmp4MqSink * filter)
{
    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    gst_pad_set_event_function (filter->sinkpad,
            GST_DEBUG_FUNCPTR(gst_fmp4mqsink_sink_event));
    gst_pad_set_chain_function (filter->sinkpad,
            GST_DEBUG_FUNCPTR(gst_fmp4mqsink_chain));
    GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    filter->adapter = gst_adapter_new();
    filter->content_id[0] = '\0';
    filter->mq_host[0] = '\0';
    filter->mq_user[0] = '\0';
    filter->mq_pass[0] = '\0';
    filter->mq_port = 0;
}

    static void
gst_fmp4mqsink_set_property (GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec)
{
    GstFmp4MqSink *filter = GST_FMP4MQSINK (object);

    switch (prop_id) {
        case PROP_CONTENT_ID:
            strncpy(filter->content_id, g_value_get_string (value), STR_LEN);
            break;
        case PROP_MQ_HOST:
            strncpy(filter->mq_host, g_value_get_string (value), STR_LEN);
            break;
        case PROP_MQ_USER:
            strncpy(filter->mq_user, g_value_get_string (value), STR_LEN);
            break;
        case PROP_MQ_PASS:
            strncpy(filter->mq_pass, g_value_get_string (value), STR_LEN);
            break;
        case PROP_MQ_PORT:
            filter->mq_port = g_value_get_int (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void gst_fmp4mqsink_get_property (GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec)
{
    GstFmp4MqSink *filter = GST_FMP4MQSINK (object);

    switch (prop_id) {
        case PROP_CONTENT_ID:
            g_value_set_string(value, filter->content_id);
            break;
        case PROP_MQ_HOST:
            g_value_set_string(value, filter->mq_host);
            break;
        case PROP_MQ_PORT:
            g_value_set_int(value, filter->mq_port);
            break;
        case PROP_MQ_USER:
            g_value_set_string(value, filter->mq_user);
            break;
        case PROP_MQ_PASS:
            g_value_set_string(value, filter->mq_pass);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static gboolean gst_fmp4mqsink_sink_event (GstPad * pad, GstObject * parent, 
        GstEvent * event)
{
    gboolean ret;
    GstFmp4MqSink *filter;

    filter = GST_FMP4MQSINK (parent);

    switch (GST_EVENT_TYPE (event)) {
        default:
            ret = gst_pad_event_default (pad, parent, event);
            break;
    }
    return ret;
}


static GstFlowReturn gst_fmp4mqsink_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    GstFmp4MqSink *filter  = GST_FMP4MQSINK (parent);
    GstAdapter     *adapter = filter->adapter;
    GstFlowReturn   ret = GST_FLOW_OK;
    GstMapInfo map;

    static bool parse_moov = false;

    auto len = gst_buffer_get_size(buf);
    GST_LOG_OBJECT(filter, "got buffer size:%ld", len);

    gst_adapter_push (adapter, buf);
    
    if(!parse_moov && len > 8){
        GstBuffer* buffer = gst_adapter_take_buffer(adapter, 8);
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        
        ///
        gst_buffer_unmap(buffer, &map);
    }
    return ret;
}

static gboolean fmp4mqsink_init (GstPlugin * fmp4mqsink)
{
    GST_DEBUG_CATEGORY_INIT (gst_fmp4mqsink_debug, "fmp4mqsink",
            0, "Sink fmp4 to MQ");

    return gst_element_register (fmp4mqsink, "fmp4mqsink", GST_RANK_NONE,
            GST_TYPE_FMP4MQSINK);
}

#define PACKAGE "fmp4mqsink"

GST_PLUGIN_DEFINE (
        GST_VERSION_MAJOR,
        GST_VERSION_MINOR,
        fmp4mqsink,
        "Sink fmp4 to MQ",
        fmp4mqsink_init,
        "1.0.1",
        "LGPL",
        "Gstreamer",
        "https://github.com/karimdavoodi/gst_fmp4mqsink"
        )
