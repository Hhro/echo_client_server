#include "server.h"

using namespace std;

void usage(){
    cout << "Usage: ./echo_server [port]" << endl;
    cout << "Example: ./echo_server 31337" << endl;
}

int main(int argc, char *argv[]){

    if(argc != 2){
        usage();
        exit(1);
    }

    event_init();

    Server srv= Server(atoi(argv[1]));
    srv.run_server();

    return 0;
}