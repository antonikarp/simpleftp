.PHONY: client server

all: client server
	mv client/simpleftp-cl .
	mv server/simpleftp-sv .
	
client: 
	+$(MAKE) -C client
server:
	+$(MAKE) -C server


