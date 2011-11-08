
CC = gcc
CFLAGS = -Wall -g -Wstrict-prototypes -I. -I../include/ -L../lib/
OBJS = $(addsuffix .o, $(basename $(wildcard *.c)))
#LDFLAGS = -lusb -ldl -linfinity
LDFLAGS = -lusb -liuu -lreadline -lncurses
RM = rm -f

#SWIG_BINS = atrswig ledswig
CLASSIC_BINS = atr led iuuterm

#BINS = $(SWIG_BINS) $(CLASSIC_BINS)
BINS = $(CLASSIC_BINS)

.PHONY : clean 

% : %.c	
	$(CC) $(CFLAGS) -o $@  $< $(LDFLAGS)

all: binaries 
binaries: $(BINS)

clean :
	$(RM) $(BINS) *.o core *~

