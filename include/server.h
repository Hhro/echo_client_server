#include <iostream>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <stdlib.h>
#include <event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <queue>
#include <unordered_map>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <signal.h>
#include <poll.h>
#include "log.h"

#define ECHO 0
#define BROADCAST 1

#define BROADCAST_FD (-1)

#define MSG_SZ 8192

class Socket{
    protected:
        int m_fd;
        struct sockaddr_in m_addr = {};
    
    public:
        Socket() {}
        const int get_fd() const { return m_fd; }
        const struct sockaddr_in & get_addr() const { return m_addr; }
        void set_fd(int fd);
        void set_addr(struct sockaddr_in & addr);
        bool set_nonblock_mode();
};

class Client: public Socket{
    public:
        explicit Client(int fd, struct sockaddr_in & addr);
};

class Message{
    private:
        int m_from_fd;
        int m_to_fd;
        int m_len;
        char m_content[MSG_SZ];
    public:
        Message(int from_fd, int to_fd, int len, char *content):
            m_from_fd(from_fd), m_to_fd(to_fd), m_len(len){
                memcpy(m_content, content, m_len);
             }
        const int get_to_fd() const { return m_to_fd; }
        const int get_from_fd() const { return m_from_fd; }
        const int get_len() const { return m_len; }
        const char* get_content() const { return m_content; }
};

class Server: public Socket{
    private:
        std::unordered_map<int, Client *> m_clients = {};
        std::unordered_map<int, struct event*> m_client_evts = {};
        std::queue<Message*> m_msg_queue= {};
        std::mutex m_msg_mtx = {};
        std::mutex m_client_mtx = {};
        std::condition_variable m_msg_cv = {};
        struct event m_ev_accept = {};
        int m_mode;
        static void invoke_cb_accept(int fd, short ev, void *ctx);
        static void invoke_client_handler(int fd, short ev, void *ctx);
        void cb_accept(int fd);
        void client_handler(int fd);
        void msg_handler();
        void wait_msg();

    public:
        explicit Server(int port, int mode);
        const struct event & get_ev_accept() const { return m_ev_accept; }
        void set_ev_accept(struct event & ev);
        void run_server();
};
