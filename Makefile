cc = gcc
src = src/shell.c src/network.c src/crypto.c
result = octo-shell
cflags = -Wall -Wextra -std=gnu99 -Iinclude -g
ldflags = -lm
all: $(result)
$(result): $(src)
	$(cc) $(cflags) -o $@ $^ $(ldflags)
clean:
	rm -f $(result)
.PHONY: all clean
