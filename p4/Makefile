all: indexer

indexer: indexer.cpp
	clang++ -std=c++11 -Wall -Wextra $^ -o $@

clean:
	rm -rf *.o proxy
