#include "server.h"

void usage(){
	printf("syntax : echo_client <host> <port>\n");
	printf("sample : echo_client 127.0.0.1 31337\n");
}

void do_recv(int sockfd){
	char buf[MSG_SZ];
	int len;
	memset(buf, 0, sizeof(buf));

	while (true) {
		len = read(sockfd, buf, sizeof(buf) - 1);
		if (len<=0){
			perror("recv failed");
			break;
		}
		write(1, buf, len);
	}
}

void do_send(int sockfd){
	char buf[MSG_SZ];
	int len;
	memset(buf, 0, sizeof(buf));

	while (true) {
		len = read(0, buf, sizeof(buf) - 1);

		ssize_t sent = write(sockfd, buf, len);
		if (sent == 0) {
			perror("send failed");
			break;
		}
	}
}

int main(int argc, char *argv[]) {
	if(argc != 3){
		usage();
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("connect failed");
		return -1;
	}
	printf("connected\n");

	std::thread rcvr(do_recv, sockfd);
	std::thread sndr(do_send, sockfd);

	rcvr.join();
	sndr.join();


	close(sockfd);
}
