# name of the program to build
#
PROG=tp_test

# Remove -DNDEBUG during development if assert(3) is used
#
override CPPFLAGS += -DNDEBUG -DPROMPT=$(PROMPT)

CC = clang++ -std=c++11

CFLAGS = -Wall -Werror -g

TP_SRCS = $(wildcard ./lib/*.cpp)
TP_OBJS = $(TP_SRCS:.c=.o)

tp : $(TP_OBJS)
	$(CC) -c $^

TEST_SRCS = $(wildcard ./tests/*.cpp)
TEST_OBJS = $(TEST_SRCS:.c=.o)

test : $(TEST_OBJS) $(TP_OBJS)
	$(CC) -o $@ $^
 

all : 

clean :
	$(RM) $(TP_OBJS) $(TEST_OBJS) $(PROG) ./bin/*