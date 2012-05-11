# Makefile for plcfetch

CC = gcc
CFLAGS += -g -Wall
LINK = gcc
LINKFLAGS = -g -o -Wall -lodbc -L/usr/lib/odbc
NAME = plcfetch

OPTIMIZE= -O3 -g -Wall -D_POSIX -D_SYSV -D_SELECT -D_HAVE_MACROS -D__USE_GNU

CFLAGS= $(OPTIMIZE)

OBJS= conf.o sysdep1.o plcfetch.o utils.o serial.o odbc.o

all: cscope ctags indent $(NAME) strip

$(NAME): $(OBJS)
	        $(CC) $(CFLAGS) $(LINKFLAGS) -o $(NAME) $(OBJS)

$(NAME).o: $(NAME).c

clean :
	rm -f *~ core *.o a.out plcfetch

cscope:
	cscope -bR

ctags:
	ctags -R

strip:
	strip plcfetch

indent:
	indent -kr -i8 -l80 *.c *.h
