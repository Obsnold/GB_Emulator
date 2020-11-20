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

#Source Files
SRC += Src/*.c
SRC += Src/opcodes/*.c

# Make all
all:$(OUT_FILE_PATH)

$(OUT_FILE_PATH): $(SRC)
	$(CC) $(CFLAGS) $^ $(LINKER_FLAGS) -o $@

# Make clean
clean:
	rm -f $(OUT_FILE_PATH)

.PHONY: all clean