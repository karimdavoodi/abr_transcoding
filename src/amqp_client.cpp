#include <string>
#include <amqp.h>
#include <sys/time.h>

#include <amqp_tcp_socket.h>
#include <boost/log/trivial.hpp>

#include "amqp_client.h"
using std::string;
void AmqpClient::set_host(const string& _host, const string& _port,
        const string& _user, const string& _passwd)
{
    host = _host;
    port = stoi(_port);
    user = _user;
    passwd = _passwd;
}
bool amq_ret_ok(amqp_rpc_reply_t x)
{
  switch (x.reply_type) {
    case AMQP_RESPONSE_NORMAL:
      return true;
    case AMQP_RESPONSE_NONE:
      BOOST_LOG_TRIVIAL(error) << "missing RPC reply type!";
      break;
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
      BOOST_LOG_TRIVIAL(error) << amqp_error_string2(x.library_error);
      break;
    case AMQP_RESPONSE_SERVER_EXCEPTION:
      BOOST_LOG_TRIVIAL(error) << "amqp server error!";
      break;
  }
  return false;
}
bool AmqpClient::connect()
{
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    amqp_rpc_reply_t ret;

    conn = amqp_new_connection();
    socket = amqp_tcp_socket_new(conn);

    if (!socket) {
        BOOST_LOG_TRIVIAL(error) << "Can't new amqp socket";
        return false;
    }
    
    int status = amqp_socket_open_noblock(socket, host.c_str(), port, &tv);
    if (status) {
        BOOST_LOG_TRIVIAL(error) << "Can't open amqp socket";
        return false;
    }

    ret = amqp_login(conn, "/", 0, 131072, 0, 
            AMQP_SASL_METHOD_PLAIN, user.c_str(), passwd.c_str());
    if(!amq_ret_ok(ret)){
        BOOST_LOG_TRIVIAL(error) << "Can't login on amqp";
        return false;
    } 

    amqp_channel_open(conn, 1);
    ret = amqp_get_rpc_reply(conn);
    if(!amq_ret_ok(ret)){
        BOOST_LOG_TRIVIAL(error) << "Can't get rpc reply";
        return false;
    } 
    return true;
}
bool AmqpClient::send_queue(const string& queue, const char* msg, int len)
{
    if(socket == NULL) connect();
    amqp_bytes_t message_bytes;
    message_bytes.bytes = (void*) msg;
    message_bytes.len = len;
    int ret = amqp_basic_publish(
            conn, 
            1, 
            amqp_cstring_bytes("amq.direct"),
            amqp_cstring_bytes(queue.c_str()), 
            0, 
            0, 
            NULL,
            message_bytes);
    if(ret < 0 ){
        BOOST_LOG_TRIVIAL(error) << "Can't send message to amqp, reset connection!";
        close();
        return false;
    }
    return true;
}
void AmqpClient::close()
{
    amqp_rpc_reply_t ret;
    if(socket == NULL) return;
    ret = amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amq_ret_ok(ret);
    ret = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amq_ret_ok(ret);
    amqp_destroy_connection(conn);
    socket = NULL;
}
AmqpClient::~AmqpClient()
{
    close();
}

