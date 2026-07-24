# Octo-Shell
A secure shell with builtin encrypted file transfer among NITTians.

## Features
- A minimal custom shell (`octo-shell`) supporting built-in commands (`cd`, `exit`) and external command execution
- `nittalk`: a built-in encrypted file transfer utility (XOR stream cipher with an LCG-based keystream)
- `tripwire`: a raw-socket packet sniffer that alerts on traffic from non-whitelisted IPs

## Compilation
```bash
make clean
make
```

This produces two executables in the project root: `octo-shell` and `tripwire`.

## How to Run

### Octo-Shell
Run the shell:
```bash
./octo-shell
```

Inside the shell, use the `nittalk` command for file transfer:

Listen for an incoming file:
```bash
nittalk -listen <port>
```

Send a file to a peer:
```bash
nittalk -s <IP> -f <filepath>
```

### Tripwire
Tripwire requires raw socket access, so it must be run with elevated privileges:
```bash
sudo ./tripwire <teammate_ip>
```
Any TCP packet arriving from an IP other than `<teammate_ip>` will trigger an alert.

## Directories & Files

### `src/`
Contains all implementation source files:
- `shell.c` — shell loop, command parsing, and built-in commands
- `network.c` — `nittalk` send/listen logic and socket handling
- `crypto.c` — XOR encryption/decryption and Diffie-Hellman helper functions
- `tripwire.c` — raw-socket packet monitoring

### `include/`
Contains all header files:
- `shell.h`
- `network.h`
- `crypto.h`
- `tripwire.h`

It is better practice to keep declarations in header files, separate from implementation.

### `Makefile`
Builds the project into the `octo-shell` and `tripwire` executables.

## Notes
- `nittalk` transfers are capped at 10 MB per file.
- The XOR keystream key used for encryption/decryption is currently a fixed, hardcoded value — intended for demonstration purposes, not production security.
