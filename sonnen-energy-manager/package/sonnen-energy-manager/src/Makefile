.PHONY: clean

all: sonnen-client sonnen-server

sonnen-server: sonnen-server.c
	$(CC) -o '$@' '$<'

sonnen-client: sonnen-client.c
	$(CC) -o '$@' '$<'

clean:
	rm sonnen-server sonnen-client
