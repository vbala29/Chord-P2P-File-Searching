# name of the program to build
#
PROG=chord

# Remove -DNDEBUG during development if assert(3) is used
#
override CPPFLAGS += -DNDEBUG -DPROMPT=$(PROMPT)

CC = clang++ -lpthread -lssl -lcrypto -std=c++14

CFLAGS = -Wall -Werror -g

UTIL_SRCS = $(wildcard ./util/*.cpp)
UTIL_OBJS = $(UTIL_SRCS:.c=.o)

util : $(UTIL_OBJS)
	$(CC) -c $^

CHORD_SRCS = $(wildcard ./src/*.cpp)
CHORD_OBJS = $(CHORD_SRCS:.c=.o)

src : $(CHORD_OBJS) $(OBJS)
	$(CC) -c $^

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.c=.o)

chord : $(CHORD_OBJS) $(UTIL_OBJS) $(OBJS)
	$(CC) -o $@ $^

all : util src chord
	mv *.o ./bin

clean :
	$(RM) ./bin/*
	rm -r chord
