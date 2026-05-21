CC=g++
CCFLAGS=-Ofast -std=c++20 -pedantic -Wall -W -Werror -Wextra -c -I..
DEFINES=

OBJ=server.o ../context/context.o ../crypt/crypt.o ../poll/poll.o ../ips_storage/ips_storage.o ../socket/socket.o ../tun/tun.o ../utils/utils.o
LDFLAGS=-lcrypto -lpthread

RUN=tserver

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
