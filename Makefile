CC = clang
CFLAGS = -Wall -Wextra -Wno-unknown-pragmas -lstdc++

main: main.cpp ./BankSimulator/bank-simulator.hpp ./BankSimulator/bank-simulator.cpp
	$(CC) $(CFLAGS) -o main main.cpp ./BankSimulator/bank-simulator.cpp

clean:
	rm -f ./main