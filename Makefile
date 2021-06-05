.PHONY: client server

all: common client server 
	mv client/simpleftp-cl .
	mv server/simpleftp-sv .
	
common:
	+$(MAKE) -C common
	
client: 
	+$(MAKE) -C client

server:
	+$(MAKE) -C server


