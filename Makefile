COMPILER 	= gcc
FLAGS 		= -pedantic -std=c99 -Wall
LINK_FLAGS 	= $(FLAGS) -pthread
DEBUG 		= -g
QUERIES 	= query1.csv query2.csv query3.csv

all: client server

client: client.c common.c
	$(COMPILER) common.c client.c $(LINK_FLAGS) -o client

server: server.c common.c
	$(COMPILER) common.c server.c $(LINK_FLAGS) -o server

clean:
	rm -rf client server