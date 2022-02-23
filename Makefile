TARGETS = ringmaster player
CC = g++
CFLAGS= -ggdb3 -Wall
all: $(TARGETS)
ringmaster: ringmaster.cpp potato.hpp
	$(CC) $(CFLAGS) -g -o $@ ringmaster.cpp
player: player.cpp potato.hpp
	$(CC) $(CFLAGS) -g -o $@ player.cpp
clean:
	rm -f $(TARGETS)