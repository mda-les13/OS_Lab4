CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17 -D_CRT_SECURE_NO_WARNINGS

TARGETS = Sender.exe Receiver.exe

SRC_SENDER = OS_Lab4/Sender.cpp
SRC_RECEIVER = OS_Lab4.Receiver/Receiver.cpp

OBJ_SENDER = OS_Lab4/Sender.o
OBJ_RECEIVER = OS_Lab4.Receiver/Receiver.o

all: $(TARGETS)

Sender.exe: $(OBJ_SENDER)
	$(CXX) $(CXXFLAGS) -o $@ $^

Receiver.exe: $(OBJ_RECEIVER)
	$(CXX) $(CXXFLAGS) -o $@ $^

Sender/Sender.o: $(SRC_SENDER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

Receiver/Receiver.o: $(SRC_RECEIVER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_SENDER) $(OBJ_RECEIVER) $(TARGETS)

.PHONY: all clean