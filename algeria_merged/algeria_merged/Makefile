CC      = gcc
CFLAGS  = -Wall -std=c99 -Iinclude
LIBS    = libraylib.a -lm -lpthread -ldl
SRCS_DATA = src/file_parser.c src/linked_list.c src/stack.c src/tree.c src/recursion.c

# ---------- GUI (default) ----------
gui: src/gui.c $(SRCS_DATA)
	$(CC) $(CFLAGS) -o algeria_gui src/gui.c $(SRCS_DATA) $(LIBS)

# ---------- Terminal version ----------
terminal: src/main.c $(SRCS_DATA)
	$(CC) $(CFLAGS) -o algeria_terminal src/main.c $(SRCS_DATA)

# ---------- Both ----------
all: gui terminal

clean:
	rm -f algeria_gui algeria_terminal

.PHONY: gui terminal all clean
