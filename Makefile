# name of the program to build
#
PROG=chord

# Remove -DNDEBUG during development if assert(3) is used
#
override CPPFLAGS += -DNDEBUG -DPROMPT=$(PROMPT)

CC = clang++ -lpthread  -lssl -lcrypto -std=c++14

CFLAGS = -Wall -Werror -g

TP_SRCS = $(wildcard ./lib/*.cpp)
TP_OBJS = $(TP_SRCS:.c=.o)

tp : $(TP_OBJS)
	$(CC) -c $^

CHORD_SRCS = $(wildcard ./src/*.cpp)
CHORD_OBJS = $(CHORD_SRCS:.c=.o)

src : $(CHORD_OBJS) $(OBJS)
	$(CC) -c $^

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.c=.o)

chord : $(CHORD_OBJS) $(TP_OBJS) $(OBJS)
	$(CC) -o $@ $^
	mv $(CHORD_OBJS) ./bin
	mv $(TP_OBJS) ./bin
	mv $(OBJS) ./bin

all : tp src chord

 
clean :
	$(RM) $(TP_OBJS) $(TEST_OBJS) $(PROG) ./bin/*


# TEST_SRCS = $(wildcard ./tests/*.cpp)
# TEST_OBJS = $(TEST_SRCS:.c=.o)

# test : $(OBJS) $(TEST_OBJS) $(TP_OBJS)
# 	$(CC) -o $@ $^ 