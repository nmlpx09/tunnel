CC = g++
CCFLAGS =-O3 -std=c++20 -pedantic -Wall -W -Werror -Wextra -c -I..
DEFINES =

OBJ =main.o ../context/context.o ../common/utils.o ../epoll/epoll.o ../socket/socket.o ../tun/tun.o
LDFLAGS=-lpthread

RUN = tclient

all: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(RUN)

install:
	strip $(RUN)
	cp $(RUN) /usr/bin/
	cp $(RUN).sh /usr/bin/

uninstall:
	rm /usr/bin/$(RUN)
	rm /usr/bin/$(RUN).sh
