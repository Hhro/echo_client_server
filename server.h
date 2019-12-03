#include <iostream>
#include <string>
#include <fcntl.h>
#include <stdlib.h>
#include <event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include "log.h"

class Socket{
    protected:
        int m_fd;
        struct sockaddr_in m_addr = {};
    
    public:
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

class Server: public Socket{
    private:
        std::unordered_map<int, Client *> m_clients;
        std::unordered_map<int, struct event*> m_client_evts;
        struct event m_ev_accept = {};
        static void invoke_cb_accept(int fd, short ev, void *ctx);
        static void invoke_client_handler(int fd, short ev, void *ctx);
        void cb_accept(int fd);
        void client_handler(int fd);

    public:
        explicit Server(int port);
        const struct event & get_ev_accept() const { return m_ev_accept; }
        void set_ev_accept(struct event & ev);
        void run_server();
};
