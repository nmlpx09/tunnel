CXX       ?= g++
CXXFLAGS  := -std=c++20 -pedantic -Wall -W -Werror -Wextra -I.
LDFLAGS   := -lcrypto -lpthread

PREFIX    ?= /usr
DESTDIR   ?=

TARGET_CLIENT := client/tun
TARGET_SERVER := server/tun

COMMON_OBJ := \
	crypt/crypt.o \
	poll/poll.o \
	socket/socket.o \
	tun/tun.o \
	utils/utils.o

CLIENT_OBJ := client/client.o $(COMMON_OBJ)
SERVER_OBJ := server/server.o ips_storage/ips_storage.o $(COMMON_OBJ)

DEPFLAGS  = -MMD -MP
COMPILE   = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

ALL_TARGETS := $(TARGET_CLIENT) $(TARGET_SERVER)

.PHONY: all client server clean install install_client install_server \
        uninstall uninstall_client uninstall_server \
        install_termux uninstall_termux debug release

ifdef DEBUG
CXXFLAGS += -g -O0 -DDEBUG
else
CXXFLAGS += -O3
endif

all: client server

client: $(TARGET_CLIENT)

server: $(TARGET_SERVER)

$(TARGET_CLIENT): $(CLIENT_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(TARGET_SERVER): $(SERVER_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

client/%.o: client/%.cpp
	$(COMPILE)

server/%.o: server/%.cpp
	$(COMPILE)

context/%.o: context/%.cpp
	$(COMPILE)

crypt/%.o: crypt/%.cpp
	$(COMPILE)

ips_storage/%.o: ips_storage/%.cpp
	$(COMPILE)

poll/%.o: poll/%.cpp
	$(COMPILE)

socket/%.o: socket/%.cpp
	$(COMPILE)

tun/%.o: tun/%.cpp
	$(COMPILE)

utils/%.o: utils/%.cpp
	$(COMPILE)

-include $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d)

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) \
	      $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d) \
	      $(TARGET_CLIENT) $(TARGET_SERVER)

install: install_client install_server

install_client: client
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 -s $(TARGET_CLIENT) $(DESTDIR)$(PREFIX)/bin/tun0
	install -m 0755 client/tun.sh $(DESTDIR)$(PREFIX)/bin/tun

install_server: server
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 -s $(TARGET_SERVER) $(DESTDIR)$(PREFIX)/bin/tun0
	install -m 0755 server/tun.sh $(DESTDIR)$(PREFIX)/bin/tun

uninstall: uninstall_client uninstall_server

uninstall_client:
	rm -f $(DESTDIR)$(PREFIX)/bin/tun0 $(DESTDIR)$(PREFIX)/bin/tun

uninstall_server:
	rm -f $(DESTDIR)$(PREFIX)/bin/tun0 $(DESTDIR)$(PREFIX)/bin/tun

TERMUX_PREFIX := /data/data/com.termux/files/usr

install_termux: client server
	install -d $(TERMUX_PREFIX)/bin
	install -m 0755 $(TARGET_CLIENT) $(TERMUX_PREFIX)/bin/tun0
	install -m 0755 client/tun_termux.sh $(TERMUX_PREFIX)/bin/tun

uninstall_termux:
	rm -f $(TERMUX_PREFIX)/bin/tun0 $(TERMUX_PREFIX)/bin/tun

debug:
	$(MAKE) DEBUG=1 all

release:
	$(MAKE) all
