CC=g++
CCFLAGS=-O3 -std=c++20 -pedantic -Wall -W -Werror -Wextra -c -I..
DEFINES=

OBJ=client.o ../context/context.o ../crypt/crypt.o ../poll/poll.o ../socket/socket.o ../tun/tun.o ../utils/utils.o
LDFLAGS=-lcrypto -lpthread

RUN=tun

all: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(RUN)

install:
	strip $(RUN)
	cp $(RUN) /usr/bin/$(RUN)0
	cp $(RUN).sh /usr/bin/$(RUN)

uninstall:
	rm /usr/bin/$(RUN)0
	rm /usr/bin/$(RUN)

install_termux:
	cp $(RUN) /data/data/com.termux/files/usr/bin/$(RUN)0
	cp $(RUN)_termux.sh /data/data/com.termux/files/usr/bin/$(RUN)

uninstall_termux:
	rm /data/data/com.termux/files/usr/bin/$(RUN)0
	rm /data/data/com.termux/files/usr/bin/$(RUN)
