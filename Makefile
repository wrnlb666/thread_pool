CC = gcc
CFLAG = -Wall -Wextra -Wpedantic -std=c2x -g
DIR = src
OBJ = tp.o
LIB = -lpthread
POST_FIX = 
ELF_FILES = 

ifeq ($(OS),Windows_NT)
	POST_FIX = dll
	LIB += -L. -ltp
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAG += -Wno-unused-result
		POST_FIX = so
		LIB += -L. -ltp
		ELF_FILES := $(shell find . -type f -executable -exec sh -c 'file -b {} | grep -q ELF' \; -print)
    endif
endif

all: tp
.PHONY: tp

tp: $(DIR)/tp.c $(DIR)/tp.h
	$(CC) $(CFLAG) -fPIC -shared $< -o lib$@.$(POST_FIX) $(LIB)

test%: test%.c
	$(CC) $(CFLAG) $< -o test $(LIB)

clean:
	rm *.dll *.exe *.o *.bin $(ELF_FILES)