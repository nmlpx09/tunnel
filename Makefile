CXX       ?= g++
CXXFLAGS  := -std=c++23 -pedantic -Wall -W -Werror -Wextra -I.
LDFLAGS   := -lcrypto -pthread

PREFIX    ?= /usr
DESTDIR   ?=

TARGET_CLIENT := client/tun
TARGET_SERVER := server/tun

DIRS := client server crypt ips_storage poll socket tun utils

COMMON_OBJ := \
	crypt/crypt.o \
	poll/poll.o \
	socket/socket.o \
	tun/tun.o \
	utils/utils.o

CLIENT_OBJ := client/client.o $(COMMON_OBJ)
SERVER_OBJ := server/server.o ips_storage/ips_storage.o $(COMMON_OBJ)

DEPFLAGS = -MMD -MP
COMPILE  = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

ALL_DEPS := $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d)

.PHONY: all client server clean \
        install install_client install_server \
        uninstall uninstall_client uninstall_server \
        install_termux uninstall_termux \
        install_service uninstall_service \
        debug release

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

define compile_rule
$(1)/%.o: $(1)/%.cpp
	$$(COMPILE)
endef

$(foreach dir,$(DIRS),$(eval $(call compile_rule,$(dir))))

-include $(ALL_DEPS)

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(ALL_DEPS) $(TARGET_CLIENT) $(TARGET_SERVER)

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

install_service: install_server
	install -d $(DESTDIR)/etc/systemd/system
	install -m 0644 server/tunnel.service $(DESTDIR)/etc/systemd/system/tunnel.service

uninstall_service:
	rm -f $(DESTDIR)/etc/systemd/system/tunnel.service

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
