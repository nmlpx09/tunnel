# tunnel

Simple UDP tunnel with AES-128-GCM encryption. No sessions, no handshakes — stateless and lightweight.

## How It Works

The tunnel creates a virtual TUN network interface and forwards all IP traffic through an encrypted UDP connection between client and server. The server NATs outgoing traffic to the internet and relays responses back through the tunnel.

```
[ App on Client ] -> [ TUN ] -> [ encrypt (AES-GCM) ] -> [ UDP ] -> [ decrypt ] -> [ TUN ] -> [ Server NAT ] -> Internet
                                                                                                         ^
                                                                                                         |
[ App on Client ] <- [ TUN ] <- [ decrypt (AES-GCM) ] <- [ UDP ] <- [ encrypt ] <- [ TUN ] <- [ Server NAT ] <- Internet
```

Each encrypted packet format: `[IV(12)][ciphertext][tag(16)]`

Each side runs two threads:
- **TX thread** — polls the TUN device, encrypts outgoing packets, sends via UDP
- **RX thread** — polls the UDP socket, decrypts incoming packets, writes to TUN

The server maintains an IP storage that dynamically maps source IPs from decrypted IPv4 packets to the client's external IP:port, enabling bidirectional NAT traversal without manual configuration.

## Features

- AES-128-GCM authenticated encryption with random IV per packet (OpenSSL EVP)
- Authentication tag verifies packet integrity and authenticity
- TUN device with `IFF_NO_PI` — raw IP packets, no protocol header overhead
- Linux epoll-based I/O multiplexing
- Non-blocking UDP sockets
- Automatic client IP tracking on the server side
- Stateless — no sessions, handshakes, or connection state
- C++23 with `-O3` release builds
- Installable on Linux and rooted Android (Termux)
- systemd service with restart support

## Requirements

- g++ 14.2.0+
- make
- libssl-dev (OpenSSL)

## Supported Systems

- Linux
- Rooted Android with Termux

## Build

```bash
make              # build both client and server
make client       # build client only
make server       # build server only
make debug        # build with debug symbols
make clean        # remove build artifacts
```

## Install

### Linux

```bash
sudo make install_client   # install client to /usr/bin
sudo make install_server   # install server to /usr/bin
sudo make uninstall        # remove
```

### Server with systemd

```bash
sudo make install_service  # install server binary, shell script, systemd unit
sudo systemctl daemon-reload
sudo systemctl enable tunnel
sudo systemctl start tunnel
```

Restart:

```bash
sudo systemctl reload tunnel   # or: sudo tun r
```

```bash
sudo make uninstall_service  # remove systemd unit
```

### Termux

```bash
make install_termux        # install client to Termux
make uninstall_termux      # remove
```

## Configuration

All configuration is done via environment variables:

| Variable       | Description                          |
|----------------|--------------------------------------|
| `TUN_DEVICE`   | TUN device name (e.g. `tun0`)       |
| `TUN_MTU`      | MTU size, max 1444                   |
| `REMOTE_IP`    | Server IP address (client only)      |
| `REMOTE_PORT`  | Server UDP port (client only)        |
| `LOCAL_PORT`   | Local UDP port (server only)         |
| `KEYS_FILE`    | Path to file with AES-128 key in base64 |

### Keys File

The keys file must contain the AES-128 key on the first line, base64-encoded:

```
<base64-encoded-16-byte-key>
```

Generate with OpenSSL:

```bash
openssl rand -base64 16 > /etc/tunnel/keys
```

Copy the same keys file to both client and server.

## Usage

Before running, edit the variables in `client/tun.sh` and `server/tun.sh` (set `REMOTE_IP`, ports, etc.).

### Start

```bash
sudo tun c
```

### Stop

```bash
sudo tun d
```

### Restart (server only)

```bash
sudo tun r
```

## Architecture

```
.
├── client/        # Client entry point and shell scripts
├── server/        # Server entry point, shell scripts, systemd unit
├── crypt/         # AES-128-GCM encryption/decryption (OpenSSL EVP)
├── ips_storage/   # Client IP tracking (server side)
├── poll/          # epoll-based I/O multiplexing
├── socket/        # UDP socket handling
├── tun/           # TUN device interface (IFF_NO_PI)
├── utils/         # Utility functions (IPv4 validation, config, key loading)
├── configs.h      # Compile-time constants
├── errors.h       # Error codes and category
└── types.h        # Common types (TBuffer, TBufferView, TConf)
```
