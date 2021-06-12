#include<iostream>
#include "../third_party/SimpleAmqpClient/SimpleAmqpClient.h"
using namespace AmqpClient;

class MqProducer {
    private:
        AmqpClient::Channel::ptr_t conn = nullptr; 
        std::string queue; 
        std::string host; 
        int port;
        std::string username;
        std::string password; 
        std::string vhost; 
        int frame_max;
        bool connected = false;
    public:
        MqProducer() {}
        MqProducer(
                const std::string &queue, 
                const std::string &host = "127.0.0.1", 
                int port = 5672, 
                const std::string &username = "guest", 
                const std::string &password = "guest", 
                const std::string &vhost = "/", 
                int frame_max = 10*1024*1024)
                : queue(queue),
                host(host),
                port(port),
                username(username),
                password(password),
                vhost(vhost),
                frame_max(frame_max){
                    connect();
                }
        void set_config(
                const std::string &_queue, 
                const std::string &_host = "127.0.0.1", 
                int _port = 5672, 
                const std::string &_username = "guest", 
                const std::string &_password = "guest", 
                const std::string &_vhost = "/", 
                int _frame_max = 10*1024*1024){
            queue = _queue;
            host = _host;
            port = _port;
            username = _username;
            password = _password;
            vhost = _vhost;
            frame_max = _frame_max;
        }

                
        bool connect();
        bool send(const std::string& msg);
        bool send_segment_file(
                const std::string channel_name, 
                const std::string filepath, 
                double start_time);
        bool send_segment_info(
                const std::string channel_name, 
                double start_time, double duration);
};
