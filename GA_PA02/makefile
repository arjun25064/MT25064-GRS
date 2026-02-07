# MT25064
CC = gcc
CFLAGS = -O2 -Wall -pthread

TARGETS = MT25064_Part_A1_server MT25064_Part_A2_server MT25064_Part_A3_server MT25064_client

all: $(TARGETS)

MT25064_Part_A1_server: MT25064_Part_A1_server.c
	$(CC) $(CFLAGS) $< -o $@

MT25064_Part_A2_server: MT25064_Part_A2_server.c
	$(CC) $(CFLAGS) $< -o $@

MT25064_Part_A3_server: MT25064_Part_A3_server.c
	$(CC) $(CFLAGS) $< -o $@

MT25064_client: MT25064_client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS) *.o *.csv *.tmp *.txt

.PHONY: all clean