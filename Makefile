OBJS=server.o simple_http.o content.o main.o util.o ring_buffer.o
CFLAGS=-g -I. -Wall -Wextra -lpthread
#DEFINES=-DTHINK_TIME
BIN=server
CC=gcc

%.o:%.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $(BIN) $^

clean:
	rm $(BIN) $(OBJS)

test0:
	./server 8080 0 &
	httperf --port=8080 --server=localhost --num-conns=1
	killall server

test1:
	./server 8085 1 &
	httperf --port=8085 --server=localhost --num-conns=10000 --rate=1000
	killall server

test2:
	./server 8090 2 &
	httperf --port=8090 --server=localhost --num-conns=10000 --rate=1000
	killall server
