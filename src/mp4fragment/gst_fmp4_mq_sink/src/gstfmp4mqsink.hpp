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
#ifndef __GST_FMP4MQSINK_H__
#define __GST_FMP4MQSINK_H__

#include <gst/gst.h>
#include <gst/base/base.h>


G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_FMP4MQSINK \
  (gst_fmp4mqsink_get_type())
#define GST_FMP4MQSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FMP4MQSINK,GstFmp4MqSink))
#define GST_FMP4MQSINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FMP4MQSINK,GstFmp4MqSinkClass))
#define GST_IS_FMP4MQSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FMP4MQSINK))
#define GST_IS_FMP4MQSINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FMP4MQSINK))

typedef struct _GstFmp4MqSink      GstFmp4MqSink;
typedef struct _GstFmp4MqSinkClass GstFmp4MqSinkClass;

#define STR_LEN 128
enum
{
  PROP_0,
  PROP_CONTENT_ID,
  PROP_MQ_HOST,
  PROP_MQ_PORT,
  PROP_MQ_USER,
  PROP_MQ_PASS
};

struct _GstFmp4MqSink
{
  GstElement element;

  GstPad *sinkpad;
  GstAdapter* adapter;

  char content_id[STR_LEN];
  char mq_host[STR_LEN];
  int  mq_port;
  char mq_user[STR_LEN];
  char mq_pass[STR_LEN];
};

struct _GstFmp4MqSinkClass 
{
  GstElementClass parent_class;
};

GType gst_fmp4mqsink_get_type (void);

G_END_DECLS
#endif /* __GST_FMP4MQSINK_H__ */
