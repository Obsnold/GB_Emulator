#TOOLS
CC      = gcc

# Flags - Warning Options
CFLAGS += -Wall
CFLAGS += -Wextra

# Output files
OUT_FILE_PATH = Out/gb_emu

#Include Files
CFLAGS += -I./Inc

LINKER_FLAGS = -lSDL2

#Source Folders
SRC_DIR += Src
SRC_DIR += Src/opcodes

SRC := $(wildcard $(addsuffix /*.c, $(SRC_DIR)))

OBJECTS := $(patsubst %.c, %.o, $(SRC))

# Make all
all:$(OBJECTS) $(OUT_FILE_PATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<  -o $@

$(OUT_FILE_PATH): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LINKER_FLAGS) -o $@

# Make clean
clean:
	rm -f $(OUT_FILE_PATH) $(OBJECTS)

.PHONY: all clean