#include "server.h"

using namespace std;

void Socket::set_addr(struct sockaddr_in & addr){
    m_addr.sin_family = addr.sin_family;
    m_addr.sin_port = addr.sin_port;
    m_addr.sin_addr.s_addr = addr.sin_addr.s_addr;
}

bool Socket::set_nonblock_mode(){
    int flags;

    flags = fcntl(m_fd, F_GETFL);
    if(flags<0)
        return false;

    flags |= O_NONBLOCK;

    if(fcntl(m_fd, F_SETFL, flags) < 0)
        return false;
    
    return true;
}

Server::Server(int port, int mode){
    int reuseaddr_on = 1;
    socklen_t addr_len = sizeof(sockaddr);

    m_mode = mode;

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = INADDR_ANY;

    m_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(m_fd < 0)
        error(1, "Failed to Create server socket.");
    if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1)
        error(1, "Failed to set server socket reusable.");

    if(set_nonblock_mode() == false){
        error(1, "Failed to set non block mode");
    }

    if(bind(get_fd(), reinterpret_cast<sockaddr *>(&m_addr), addr_len) < 0)
        error(1, "Failed to bind server socket.");
    if(listen(m_fd, 5) < 0) 
        error(1, "Failed to Listen on server socket.");
}

void Server::run_server(){
    std::thread t1 = std::thread(&Server::msg_handler, this);
    event_set(&m_ev_accept, m_fd, EV_READ|EV_PERSIST, invoke_cb_accept, this);
    event_add(&m_ev_accept, NULL);
    event_dispatch();
    t1.join();
}

void Server::invoke_cb_accept(int fd, short ev, void *ctx){
    return (static_cast<Server*>(ctx))->cb_accept(fd);
}

void Server::invoke_client_handler(int fd, short ev, void *ctx){
    return (static_cast<Server*>(ctx))->client_handler(fd);
}

void Server::cb_accept(int fd){
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(sockaddr);
    struct event *client_evt = (struct event*)calloc(sizeof(struct event), 1);

    client_fd = accept(fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    if(client_fd == -1){
        warn("[!]Accept client failed");
        return;
    }

    Client *client = new Client(client_fd, client_addr);
    if(client->set_nonblock_mode() == false){
        error(1, "Failed to set client socket as non block mode.");
    }

    m_client_mtx.lock();
    m_clients[client_fd] = client;
    m_client_evts[client_fd] = client_evt;
    m_client_mtx.unlock();

    event_set(client_evt, client_fd, EV_READ|EV_PERSIST, invoke_client_handler, this);
    event_add(client_evt, NULL);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);

    std::cout << "Client from " << ip << "connected" << endl;
}

void Server::client_handler(int fd){
	char buf[MSG_SZ];
	int len, wlen;
    Message *msg = nullptr;

    memset(buf, 0, MSG_SZ);
    len = read(fd, buf, sizeof(buf));
	if (len == 0) {
        /* Connection disconnected */
		cout << "Client disconnected." << endl;
        goto close_conn;
	}
	else if (len < 0) {
        /* Unexpected socket error occur */
        warn("Unexpected socket error, shutdown connection");
        goto close_conn;
	}

    switch(m_mode){
        case ECHO:
            msg = new Message(fd, fd, len, buf);
            break;
        case BROADCAST:
            msg = new Message(fd, -1, len, buf);
            break;
    }
    
    if(msg == nullptr){
        error(1, "Failed to create message instance.");
    }
    else{
        m_msg_mtx.lock();
        m_msg_queue.push(msg);
        m_msg_mtx.unlock();
    }
    m_msg_cv.notify_one();
    return;

close_conn:
    m_client_mtx.lock();
    close(fd);
    event_del(m_client_evts[fd]);
    delete(m_clients[fd]);

    m_clients.erase(fd);
    m_client_evts.erase(fd);
    m_client_mtx.unlock();
    return;
}

void Server::msg_handler(){
    const char *content;
    int to_fd;
    int len;

    while(true){
        std::unique_lock<std::mutex> lk(m_msg_mtx);
        m_msg_cv.wait(lk, [&] { return !m_msg_queue.empty(); });
        Message *msg = m_msg_queue.front();
        m_msg_queue.pop();
        lk.unlock();

        to_fd = msg->get_to_fd();
        len = msg->get_len();
        content = msg->get_content();

        m_client_mtx.lock();
        if(to_fd == BROADCAST_FD){
            for(auto fd_client : m_clients){
                write(fd_client.first, content, len);
            }
        }
        else{
            write(to_fd, content, len);
        }
        m_client_mtx.unlock();
    }
}

Client::Client(int fd, struct sockaddr_in & addr){
    m_fd = fd;
    m_addr = addr;
}
