CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++98 -D_CRT_SECURE_NO_WARNINGS

TARGETS = Sender.exe Receiver.exe

SRC_SENDER = Sender/Sender.cpp
SRC_RECEIVER = Receiver/Receiver.cpp

OBJ_SENDER = Sender/Sender.o
OBJ_RECEIVER = Receiver/Receiver.o

all: $(TARGETS)

Sender.exe: $(OBJ_SENDER)
	$(CXX) $(CXXFLAGS) -o $@ $^

Receiver.exe: $(OBJ_RECEIVER)
	$(CXX) $(CXXFLAGS) -o $@ $^

Sender/Sender.o: $(SRC_SENDER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

Receiver/Receiver.o: $(SRC_RECEIVER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJ_SENDER) $(OBJ_RECEIVER) $(TARGETS)

.PHONY: all clean