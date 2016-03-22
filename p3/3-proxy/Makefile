all: proxy 

proxy: proxy_main.cpp proxy.cpp
	g++ -std=c++11 -Wall -Wextra $^ -o $@ -lboost_system

clean:
	rm -rf *.o proxy
