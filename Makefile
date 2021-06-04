all: simpleftp-cl simpleftp-sv
simpleftp-cl: ./client/simpleftp-cl.c ./client/helper-cl.c
	gcc -Wall -std=gnu99 -o simpleftp-cl ./client/simpleftp-cl.c ./client/helper-cl.c

simpleftp-sv: ./server/simpleftp-sv.c ./server/helper-sv.c
	gcc -Wall -std=gnu99 -o simpleftp-sv ./server/simpleftp-sv.c ./server/helper-sv.c -lpthread
