#pragma once
#include <string>
#include <amqp.h>
using std::string;

class AmqpClient {
    private:
        string host;
        int    port;
        string user;
        string passwd;  
        
        amqp_socket_t *socket = NULL;
        amqp_connection_state_t conn;

    public:
        void set_host(const string& _host, const string& _port,
                     const string& _user, const string& _passwd);
        void close();
        bool connect();
        bool send_queue(const string& queue, const char* msg, int len);
        ~AmqpClient();
};
