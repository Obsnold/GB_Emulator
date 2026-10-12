#!/bin/sh
# run_test_roms.sh - run every .gb under Test/roms/ through the headless harness.
#
# Third-party test ROMs are not committed to this repository; see
# Test/roms/README.md for where to get them. If none are present this exits 0
# so that `make test-all` stays usable without them.

set -u

HARNESS="${1:-Build/rom_harness}"
ROM_DIR="$(dirname "$0")/roms"

if [ ! -x "$HARNESS" ]; then
    echo "harness not built: $HARNESS" >&2
    exit 2
fi

ROMS=$(find "$ROM_DIR" -name '*.gb' 2>/dev/null | sort)

if [ -z "$ROMS" ]; then
    echo "no test ROMs in $ROM_DIR - skipping (see $ROM_DIR/README.md)"
    exit 0
fi

pass=0
fail=0

for rom in $ROMS; do
    if "$HARNESS" "$rom" --frames 600 >/dev/null 2>&1; then
        pass=$((pass + 1))
        printf 'PASS    %s\n' "$(basename "$rom")"
    else
        fail=$((fail + 1))
        printf 'FAIL    %s\n' "$(basename "$rom")"
        "$HARNESS" "$rom" --frames 600 2>&1 | sed 's/^/        /'
    fi
done

printf '\n%d passed, %d failed\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
