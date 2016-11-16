CC = gcc -o
CFLAGS  = -g -Wall

SRCDIR =./src

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=*.o)


default: all


all:  $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o bin/quine_mccluskey_solver -lm

clean: 
	$(RM) all *.o *~