cc = gcc
src = src/shell.c src/network.c src/crypto.c
result = octo-shell
trip_src = src/tripwire.c
trip_result = tripwire

cflags = -Wall -Wextra -std=gnu99 -Iinclude -g
ldflags = -lm
all: $(result) $(trip_result)
$(result): $(src)
	$(cc) $(cflags) -o $@ $^ $(ldflags)
$(trip_result): $(trip_src)
	$(cc) $(cflags) -o $@ $^ $(ldflags)
clean:
	rm -f $(result) $(trip_result)
.PHONY: all clean
