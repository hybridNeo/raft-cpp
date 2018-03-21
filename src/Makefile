all: ./../out/node


./../out/node:  ./../out/com.o ./../out/node.o 
	mkdir  -p ./../out
	g++ -std=c++11 ./../out/node.o ./../out/com.o -o ./../out/node -lboost_system -pthread

./../out/node.o: node.cpp
	mkdir  -p ./../out
	g++ -std=c++11 -c node.cpp -o ./../out/node.o

./../out/com.o: com.hpp com.cpp 
	mkdir -p ./../out
	g++ -std=c++11 -c com.cpp -o ./../out/com.o


clean:
	rm -rf ./../out
