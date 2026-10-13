#TOOLS
CC      = gcc

# Flags - Warning Options
CFLAGS += -Wall
CFLAGS += -Wextra

# GCC's static analyzer. Costs about 40% of a (1 second) build here and found
# real bugs, so it runs on every build. Probed rather than hardcoded so the
# Makefile still works on gcc older than 10, or on a different compiler.
CFLAGS += $(shell $(CC) -fanalyzer -E -x c /dev/null >/dev/null 2>&1 && echo -fanalyzer)

# Output files
OUT_FILE_PATH = Out/gb_emu

#Include Files
CFLAGS += -I./Inc

LINKER_FLAGS = -lSDL2

#Source Folders
SRC_DIR += Src
SRC_DIR += Src/opcodes
SRC_DIR += Src/io_ports
SRC_DIR += Src/os

SRC := $(wildcard $(addsuffix /*.c, $(SRC_DIR)))

OBJECTS := $(patsubst %.c, %.o, $(SRC))

# Make all
all:$(OBJECTS) $(OUT_FILE_PATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<  -o $@

$(OUT_FILE_PATH): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ $(LINKER_FLAGS) -o $@

# Make clean
clean:
	rm -f $(OUT_FILE_PATH) $(OBJECTS)
	rm -rf $(BUILD_DIR)

.PHONY: all clean

# ---------------------------------------------------------------------------
# Tests and analysis
#
# The emulator core links without SDL and without main.c, so the test binaries
# are built from everything in Src/ except main.c and the OS interface layer.
#
#   make check       everything below, in order, reporting every stage
#
#   make analyze     full static analysis sweep (diagnostics only)
#   make cppcheck    cppcheck, if it is installed
#   make test        unit tests
#   make test-asan   unit tests under AddressSanitizer + UBSan
#   make fuzz        random cart images under the sanitizers
#   make web               build the browser version into Build/web
#   make fetch-test-roms   download the pinned prebuilt test ROM bundle
#   make test-roms         run them against Test/roms/expected.txt
#
# Test objects go under Build/ so they never collide with the in-tree .o files
# the default build produces, and so the two can use different flags.
# ---------------------------------------------------------------------------

BUILD_DIR = Build
TEST_DIR  = Test

# Core sources: no main(), no SDL.
CORE_SRC := $(filter-out Src/main.c Src/os/linux_interface.c, $(SRC))

# Unit test sources, minus the standalone harness binary.
TEST_SRC := $(filter-out $(TEST_DIR)/rom_harness.c, $(wildcard $(TEST_DIR)/*.c))

# No -fanalyzer here: the tests deliberately drive out-of-range values at the
# emulator, and analyzing the test code itself is noise.
TEST_CFLAGS  = -Wall -Wextra -I./Inc -I./$(TEST_DIR) -g -O1 -MMD -MP
TEST_LDFLAGS = -lm

# UBSan is made recoverable so a single bad index does not abort the run and
# hide every test after it; the runner counts any sanitizer report as a test
# failure, so nothing is silently tolerated. ASAN halts on error regardless.
SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer \
            -fsanitize-recover=undefined

PLAIN_OBJ := $(addprefix $(BUILD_DIR)/test/, $(CORE_SRC:.c=.o) $(TEST_SRC:.c=.o))
ASAN_OBJ  := $(addprefix $(BUILD_DIR)/asan/, $(CORE_SRC:.c=.o) $(TEST_SRC:.c=.o))

HARNESS_PLAIN_OBJ := $(addprefix $(BUILD_DIR)/test/, \
                       $(CORE_SRC:.c=.o) $(TEST_DIR)/rom_harness.o)
HARNESS_ASAN_OBJ  := $(addprefix $(BUILD_DIR)/asan/, \
                       $(CORE_SRC:.c=.o) $(TEST_DIR)/rom_harness.o)

$(BUILD_DIR)/test/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(BUILD_DIR)/asan/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) $(SAN_FLAGS) -c $< -o $@

$(BUILD_DIR)/test_core: $(PLAIN_OBJ)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) $^ $(TEST_LDFLAGS) -o $@

$(BUILD_DIR)/test_core_asan: $(ASAN_OBJ)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) $(SAN_FLAGS) $^ $(TEST_LDFLAGS) -o $@

$(BUILD_DIR)/rom_harness: $(HARNESS_PLAIN_OBJ)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) $^ $(TEST_LDFLAGS) -o $@

$(BUILD_DIR)/rom_harness_asan: $(HARNESS_ASAN_OBJ)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) $(SAN_FLAGS) $^ $(TEST_LDFLAGS) -o $@

# TEST_FILTER=substring runs only the tests whose name contains it.
test: $(BUILD_DIR)/test_core
	@$(BUILD_DIR)/test_core $(TEST_FILTER)

test-asan: $(BUILD_DIR)/test_core_asan
	@ASAN_OPTIONS=detect_leaks=0 \
	 UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0 \
	 $(BUILD_DIR)/test_core_asan $(TEST_FILTER)

# FUZZ_COUNT / FUZZ_SEED override the defaults.
FUZZ_COUNT ?= 300
FUZZ_SEED  ?= 1

fuzz: $(BUILD_DIR)/rom_harness_asan
	@ASAN_OPTIONS=detect_leaks=0 \
	 $(BUILD_DIR)/rom_harness_asan --fuzz $(FUZZ_COUNT) --seed $(FUZZ_SEED)

harness: $(BUILD_DIR)/rom_harness

# Third-party test ROMs are not vendored; they are fetched as a pinned release
# of c-sp/gameboy-test-roms, which ships prebuilt .gb files so nothing needs an
# assembler. Test/roms/expected.txt records which ones pass today.
TEST_ROMS_VERSION = v7.0
TEST_ROMS_URL = https://github.com/c-sp/gameboy-test-roms/releases/download/$(TEST_ROMS_VERSION)/game-boy-test-roms-$(TEST_ROMS_VERSION).zip

fetch-test-roms:
	@mkdir -p $(TEST_DIR)/roms
	@echo "fetching gameboy-test-roms $(TEST_ROMS_VERSION)"
	@curl -sSLf -o $(TEST_DIR)/roms/bundle.zip "$(TEST_ROMS_URL)"
	@rm -rf $(TEST_DIR)/roms/bundle
	@mkdir -p $(TEST_DIR)/roms/bundle
	@python3 -c "import zipfile,sys; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" \
	    $(TEST_DIR)/roms/bundle.zip $(TEST_DIR)/roms/bundle
	@rm -f $(TEST_DIR)/roms/bundle.zip
	@echo "extracted to $(TEST_DIR)/roms/bundle"

test-roms: $(BUILD_DIR)/rom_harness
	@sh $(TEST_DIR)/run_test_roms.sh $(BUILD_DIR)/rom_harness

test-roms-update: $(BUILD_DIR)/rom_harness
	@sh $(TEST_DIR)/run_test_roms.sh $(BUILD_DIR)/rom_harness --update

# ---------------------------------------------------------------------------
# Web build
#
#   make web      builds Build/web/index.html + .js + .wasm
#
# The same sources as the native build, SDL and all: Emscripten's SDL2 port
# covers everything Src/os/linux_interface.c uses. What differs is the loop -
# the browser owns it, so Src/main.c registers run_frame as a
# requestAnimationFrame callback instead of pacing it itself - and the cart,
# which arrives as a buffer from a file input rather than as a path.
#
# EMCC can point at an emsdk that is not on PATH:
#   make web EMCC=/path/to/emsdk/upstream/emscripten/emcc
# ---------------------------------------------------------------------------

EMCC ?= emcc
WEB_DIR = Web
WEB_OUT = $(BUILD_DIR)/web

WEB_CFLAGS = -I./Inc -O2 -sUSE_SDL=2

WEB_LDFLAGS = -sUSE_SDL=2 \
              -sALLOW_MEMORY_GROWTH=1 \
              -sEXPORTED_FUNCTIONS=_main,_web_load_rom,_web_rom_loaded,_malloc,_free \
              -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,HEAPU8 \
              -sMINIFY_HTML=0 \
              --shell-file $(WEB_DIR)/shell.html

web:
	@mkdir -p $(WEB_OUT)
	$(EMCC) $(WEB_CFLAGS) $(SRC) $(WEB_LDFLAGS) -o $(WEB_OUT)/index.html
	@echo "built $(WEB_OUT)/index.html"

# ---------------------------------------------------------------------------
# Static analysis
#
# Compiles to /dev/null, so this changes no binary. It builds at -O2 on
# purpose: -Warray-bounds and -Wmaybe-uninitialized are flow-sensitive and do
# nothing without optimisation, which is why the unoptimised default build
# never reported the out-of-bounds write in init_mem_map.
#
# Two further sweeps are left out of the default set because each is a
# one-time cleanup rather than a bug report, and together they bury the real
# findings under ~480 lines. Run either on demand:
#
#   narrowing audit (24 implicit conversions, mostly intentional, in the PPU):
#     make analyze ANALYZE_EXTRA=-Wconversion
#
#   prototypes and const-correctness (196 K&R-style `void f()` declarations,
#   plus 256 string literals assigned to the opcode table's `char *`
#   description field, which wants to be `const char *`):
#     make analyze ANALYZE_EXTRA="-Wstrict-prototypes -Wmissing-prototypes \
#                                 -Wold-style-definition -Wwrite-strings"
# ---------------------------------------------------------------------------

ANALYZE_EXTRA ?=

# Shared warning set. Split across two passes below.
ANALYZE_WARNINGS = -Wall -Wextra \
                   -Warray-bounds=2 \
                   -Wmaybe-uninitialized \
                   -Wnull-dereference \
                   -Wtype-limits \
                   -Wsign-compare \
                   -Wlogical-op \
                   -Wduplicated-cond \
                   -Wduplicated-branches \
                   -Wshadow \
                   -Wundef \
                   -Wvla \
                   -Wcast-align \
                   $(ANALYZE_EXTRA)

FANALYZER := $(shell $(CC) -fanalyzer -E -x c /dev/null >/dev/null 2>&1 && echo -fanalyzer)

# Two passes are needed, and neither subsumes the other:
#
#   -O2  turns on the flow-sensitive warnings. -Warray-bounds and
#        -Wmaybe-uninitialized are no-ops without optimisation, which is why
#        the unoptimised default build never reported the out-of-bounds write
#        in init_mem_map.
#
#   -O0  is where -fanalyzer does its best work. At -O2 the optimiser folds
#        away paths before the analyzer explores them: on this tree it reports
#        5 findings at -O0 and only 1 at -O2.
ANALYZE_O2_FLAGS = -I./Inc -O2 $(ANALYZE_WARNINGS)
ANALYZE_SA_FLAGS = -I./Inc -O0 $(ANALYZE_WARNINGS) $(FANALYZER)

ANALYZE_LOG = $(BUILD_DIR)/analyze.log

analyze:
	@mkdir -p $(BUILD_DIR)
	@rm -f $(ANALYZE_LOG)
	@for f in $(SRC); do \
	    $(CC) $(ANALYZE_O2_FLAGS) -c $$f -o /dev/null 2>> $(ANALYZE_LOG); \
	done
	@for f in $(SRC); do \
	    $(CC) $(ANALYZE_SA_FLAGS) -c $$f -o /dev/null 2>> $(ANALYZE_LOG); \
	done
	@cat $(ANALYZE_LOG)
	@n=`grep 'warning:' $(ANALYZE_LOG) 2>/dev/null | sort -u | wc -l`; \
	 echo "analyze: $$n distinct warnings"; \
	 [ "$$n" -eq 0 ]

# cppcheck covers a different class to GCC, and on this tree it found two
# things GCC missed entirely:
#
#   unsignedLessThanZero       `if (g_cart_size <= 0)` on an unsigned, so a
#                              missing ROM file is reported as loaded. GCC's
#                              -Wtype-limits does not flag this.
#   invalidPrintfArgType_sint  %ld against unsigned long in gb_ppu.c. GCC never
#                              sees it because the call sits behind #ifdef
#                              DEBUG, which is off; cppcheck checks every
#                              preprocessor configuration, not just the one
#                              the build selects.
#
# Suppressed by default, each a one-time cleanup rather than a bug:
#
#   constParameterPointer / constVariablePointer   10 const-correctness
#       suggestions. Several are on the dead get_16_* helpers in gb_common.c,
#       which want deleting rather than annotating.
#   unusedFunction in debug_print.c   9 debug helpers whose call sites are
#       commented out in main.c on purpose. The check is still on elsewhere,
#       where it correctly flags get_16_from_8, set_16_from_8, get_16_low,
#       get_16_high, sleep_ns, reverse_bits, get_mem_map_16 and save_raw_file.
#   staticFunction   24 findings, and acting on them would break the tests.
#       cppcheck only sees Src/, so it cannot tell that set_pallets,
#       ppu_oam_search and the draw_*_line functions are called from
#       Test/test_ppu.c. If you would rather make the intent explicit than
#       carry this suppression, declare those seven in Inc/gb_ppu.h and drop
#       the suppression; the rest of the findings are genuine.
#
# To see the suppressed ones:
#   make cppcheck CPPCHECK_EXTRA=--enable=all
# ---------------------------------------------------------------------------

CPPCHECK_EXTRA ?=

CPPCHECK_FLAGS = --enable=warning,style,performance,portability,unusedFunction \
                 --check-level=exhaustive \
                 --std=c99 --platform=unix64 -I Inc \
                 --inline-suppr \
                 --suppress=missingIncludeSystem \
                 --suppress=checkersReport \
                 --suppress=constParameterPointer \
                 --suppress=constVariablePointer \
                 --suppress=unusedFunction:Src/debug_print.c \
                 --suppress=staticFunction \
                 --error-exitcode=1 \
                 $(CPPCHECK_EXTRA)

cppcheck:
	@if command -v cppcheck >/dev/null 2>&1; then \
	    cppcheck $(CPPCHECK_FLAGS) Src/; \
	 else \
	    echo "cppcheck not installed, skipping (apt install cppcheck)"; \
	 fi

# One command before pushing. Runs every stage even if an earlier one fails,
# so a red tree shows you everything at once instead of one problem per run.
check:
	@fail=0; \
	 $(MAKE) --no-print-directory analyze   || fail=1; \
	 $(MAKE) --no-print-directory cppcheck  || fail=1; \
	 $(MAKE) --no-print-directory test      || fail=1; \
	 $(MAKE) --no-print-directory test-asan || fail=1; \
	 $(MAKE) --no-print-directory fuzz      || fail=1; \
	 $(MAKE) --no-print-directory test-roms || fail=1; \
	 echo; \
	 if [ $$fail -eq 0 ]; then \
	     echo "=========== check: all stages passed ==========="; \
	 else \
	     echo "=========== check: stages above reported failures ==========="; \
	 fi; \
	 exit $$fail

.PHONY: test test-asan fuzz harness test-roms test-roms-update \
        fetch-test-roms analyze cppcheck check web

-include $(shell find $(BUILD_DIR) -name '*.d' 2>/dev/null)
