# tunnel

Simple UDP tunnel with AES-128-CBC encryption. No sessions or handshakes — stateless and lightweight.

## How It Works

The tunnel creates a virtual TUN network interface and forwards all IP traffic through an encrypted UDP connection between client and server. The server NATs outgoing traffic to the internet and relays responses back through the tunnel.

```
[ Client ] --(encrypted UDP)--> [ Server ] --(NAT)--> Internet
```

## Requirements

- g++ 10.2.1+
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
| `TUN_MTU`      | MTU size, max 1460                   |
| `REMOTE_IP`    | Server IP address (client only)      |
| `REMOTE_PORT`  | Server UDP port (client only)        |
| `LOCAL_PORT`   | Local UDP port (server only)         |
| `KEYS_FILE`    | Path to file with AES key and IV in base64 |

### Keys File

The keys file must contain the AES-128 key on the first line and the IV on the second line, both base64-encoded:

```
<base64-encoded-16-byte-key>
<base64-encoded-16-byte-iv>
```

Generate with OpenSSL:

```bash
openssl rand -base64 16 > /etc/tunnel/keys
openssl rand -base64 16 >> /etc/tunnel/keys
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

## Architecture

```
.
├── client/        # Client entry point and shell scripts
├── server/        # Server entry point and shell scripts
├── crypt/         # AES-128-CBC encryption/decryption (OpenSSL EVP)
├── ips_storage/   # Client IP tracking (server side)
├── poll/          # epoll-based I/O multiplexing
├── socket/        # UDP socket handling
├── tun/           # TUN device interface
├── utils/         # Utility functions
├── configs.h      # Compile-time constants
├── errors.h       # Error codes and category
└── types.h        # Common types (TBytes, TConf)
```
