CC = cc
CFLAGS ?= -Wall -Wextra -O2 -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS ?=

TARGETS = sonnen-server sonnen-client

SERVER_OBJS = server.o protocol.o utils.o
CLIENT_OBJS = client.o protocol.o utils.o
ALL_OBJS = $(SERVER_OBJS) $(CLIENT_OBJS)

HEADERS = sonnen.h protocol.h utils.h


all: $(TARGETS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

sonnen-server: $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

sonnen-client: $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS) $(ALL_OBJS)

.PHONY: all clean
