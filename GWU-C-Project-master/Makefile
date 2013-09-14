OBJS=server.o simple_http.o content.o util.o ring_buffer.o linked_list.o main.o
CFLAGS=-g -I. -Wall -Wextra -lpthread -m32 --std=c99
#DEFINES=-DTHINK_TIME
BIN=server
CC=gcc

%.o:%.c
	$(CC) $(DEFINES) -o $@ -c $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) $(DEFINES) -o $(BIN) $^ $(CFLAGS)

clean:
	rm $(BIN) $(OBJS)

test0:
	./server 8080 0 &
	httperf --port=8080 --server=localhost --num-conns=1
	killall server

test1:
	./server 8089 1 &
	httperf --port=8089 --server=localhost --num-conns=1000 --burst-len=100
	killall server

test2:
	./server 8030 2 &
	httperf --port=8030 --server=localhost --num-conns=1000 --burst-len=100
	killall server
