all: main

main: main.o shell.o parser.o cmd.o
	clang++ -std=c++14 -Wall -Wextra $^ -o $@

%.o: %.cpp
	clang++ -c -std=c++14 -Wall -Wextra $< -o $@


clean:
	rm -rf *.o main

