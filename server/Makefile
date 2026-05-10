CC = g++
CCFLAGS =-O3 -std=c++20 -pedantic -Wall -W -Werror -Wextra -c -I..
DEFINES =

OBJ =main.o ../context/context.o ../common/utils.o ../epoll/epoll.o ../ips_storage/ips_storage.o ../socket/socket.o ../tun/tun.o
LDFLAGS=-lpthread

RUN = server

server: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(RUN)
