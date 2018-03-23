all: ./../out/node

CFLAGS = -g -std=c++11

./../out/node:  ./../out/com.o ./../out/node.o 
	mkdir  -p ./../out
	g++ $(CFLAGS) ./../out/node.o ./../out/com.o -o ./../out/node -lboost_system -pthread

./../out/node.o: node.cpp
	mkdir  -p ./../out
	g++ $(CFLAGS) -c node.cpp -o ./../out/node.o

./../out/com.o: com.hpp com.cpp 
	mkdir -p ./../out
	g++ $(CFLAGS) -c com.cpp -o ./../out/com.o

./../out/raft.o: raft.hpp raft.cpp
	mkdir -p ./../out
	g++ $(CFLAGS) -c raft.cpp -o ./../out/raft.o


clean:
	rm -rf ./../out
