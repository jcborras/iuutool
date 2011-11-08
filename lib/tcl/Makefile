CC = gcc
RM = rm -f
SWIG = swig -tcl

CFLAGS = -I../include -Wall -fPIC
SFLAGS = -Wall -Wallkw
OBJS = $(addsuffix .o, $(basename $(wildcard *.c)))


# If you get compilation errors because you don't have SWIG or Tcl/Tk
# installed please uncomment the following line
#all: clean

iuu.so:
	$(SWIG) $(SFLAGS) -o iuutcl.c ../iuu.i
	$(CC) -fpic -c -I../../include ../iuu.c iuutcl.c
	$(CC) -shared -I../../include iuu.o iuutcl.o -o iuu.so -lusb

#%.o : %.c
#	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY : clean iuu.so

clean :
	$(RM) *.o *.a *.so *tcl*.c
