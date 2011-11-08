
CC = gcc
CFLAGS = -Wall -O2 -Wstrict-prototypes -I. -Iinclude/ -Llib/  -fPIC
OBJS = $(addsuffix .o, $(basename $(wildcard *.c)))
#LDFLAGS = -lusb -ldl -linfinity
LDFLAGS = -lusb
PROG = iuutool

RM = rm -f

all : $(OBJS) lib test

.PHONY : clean lib plugins test

lib :
	$(MAKE) -C $@

plugins :
	$(MAKE) -C $@

test :
	$(MAKE) -C $@

clean :
	$(RM) $(PROG) *.o core

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

