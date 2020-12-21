# https://stackoverflow.com/questions/30573481/path-include-and-src-directory-makefile

EXE = RNG
CC=gcc
SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS += -Iinclude
CFLAGS += -Wall
LDFLAGS += -Llib
LDLIBS += -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)

# CC=gcc
# CFLAGS=$(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
# DEPS = config.h typedefinitions.h
# OBJ = cdev.o typedefinitions.o
#
# %.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)
#
# cdev: $(OBJ)
# 	$(CC) -o $@ $^ $(CFLAGS)

# http://www.gnu.org/software/make/manual/make.html
