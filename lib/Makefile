CC = gcc
RM = rm -f
CFLAGS = -I../include -Wall -fPIC
OBJS = $(addsuffix .o, $(basename $(wildcard *.c)))


all : libiuu.a tcl

.PHONY : iuu.so clean tcl

libiuu.a : $(OBJS)
	$(AR) cr $@ $?

iuu.so :
	swig -tcl -o iuutcl_wrap.c ./iuu.i
	gcc -fpic -c -I../include iuu.c iuutcl.c iuutcl_wrap.c
	gcc -shared -I../include iuu.o iuutcl.o iuutcl_wrap.o -o iuu.so -lusb

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

tcl :
	$(MAKE) -C $@

clean :
	$(RM) *.o *.a *.so *tcl*.c
