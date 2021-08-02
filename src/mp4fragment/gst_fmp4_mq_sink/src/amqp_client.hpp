#ifndef _AMQP_CLIENT_H_
#define _AMQP_CLIENT_H_
#include "gstfmp4mqsink.hpp"
int amqp_client_init(GstFmp4MqSink *filter);
int amqp_client_close(GstFmp4MqSink *filter);
#endif
