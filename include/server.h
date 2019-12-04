#pragma once

#include <iostream>
#include <string>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <queue>
#include <unordered_map>
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <event.h>

#include "socket.h"
#include "client.h"
#include "message.h"
#include "log.h"

#define MSGWORKERN 4

#define ECHO 0
#define BROADCAST 1

#define BROADCAST_FD (-1)

class Server: public Socket{
    private:
        std::unordered_map<int, Client *> m_clients = {};
        std::unordered_map<int, struct event*> m_client_evts = {};
        std::queue<Message*> m_msg_queue= {};
        std::vector<std::thread> m_msg_worker = {};
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
