SRV = echo_server
CLI = echo_client
CPP = g++

all: echo_server.o echo_client.o
	$(CPP) -o $(SRV) echo_server.o -L ../lib -lasync -levent -lpthread
	$(CPP) -o $(CLI) echo_client.o -L ../lib -lasync -levent -lpthread

%.o: %.cpp
	$(CPP) -c $< -o $@ -I../include

clean:
	rm *.o $(SRV) $(CLI)
