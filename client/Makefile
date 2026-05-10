CC = g++
CCFLAGS =-O3 -std=c++23 -pedantic -Wall -W -Werror -Wextra -c -I..
DEFINES =

OBJ = main.o ../context/context.o ../common/utils.o ../epoll/epoll.o ../socket/socket.o ../tun/tun.o
LDFLAGS=

RUN = client

client: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(RUN)
