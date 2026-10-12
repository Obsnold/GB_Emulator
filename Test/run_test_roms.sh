#!/bin/sh
# run_test_roms.sh - run the third-party test ROMs and compare against the
# recorded baseline in Test/roms/expected.txt.
#
#   usage: run_test_roms.sh <harness> [--update]
#
# Exits non-zero on a regression: a ROM the baseline says passes but no longer
# does, or a ROM named in the baseline that is missing from the bundle. A ROM
# that starts passing is reported but does not fail the run - that is progress,
# and the baseline should be refreshed to lock it in.
#
# With --update, rewrites the baseline from the observed results instead of
# comparing against it.
#
# Exits 0 with a message if the bundle is absent, so `make check` stays usable
# without it.

set -u

HARNESS="${1:-Build/rom_harness}"
MODE="${2:-}"
HERE="$(dirname "$0")"
ROOT="$HERE/roms/bundle"
EXPECTED="$HERE/roms/expected.txt"
FRAMES=3000

if [ ! -x "$HARNESS" ]; then
    echo "harness not built: $HARNESS" >&2
    exit 2
fi
if [ ! -f "$EXPECTED" ]; then
    echo "no baseline at $EXPECTED" >&2
    exit 2
fi
if [ ! -d "$ROOT" ]; then
    echo "no test ROMs in $ROOT - skipping (run: make fetch-test-roms)"
    exit 0
fi

# Observed status for one ROM: pass, fail or timeout.
observe() {
    _out=$("$HARNESS" "$ROOT/$2" --frames "$FRAMES" "--$1" 2>&1 1>/dev/null \
           | head -1 | awk '{print $1}')
    case "$_out" in
        PASS) echo pass ;;
        FAIL) echo fail ;;
        *)    echo timeout ;;
    esac
}

if [ "$MODE" = "--update" ]; then
    TMP=$(mktemp)
    sed -n '/^#/p' "$EXPECTED" > "$TMP"
    grep -v '^#' "$EXPECTED" | grep -v '^[[:space:]]*$' | while read -r _exp suite rel; do
        printf '%s %s %s\n' "$(observe "$suite" "$rel")" "$suite" "$rel"
    done | sort -k2,2 -k3,3 >> "$TMP"
    mv "$TMP" "$EXPECTED"
    echo "baseline refreshed: $EXPECTED"
    exit 0
fi

regressions=0
improvements=0
missing=0
matched=0

while read -r expected suite rel; do
    case "$expected" in ''|\#*) continue ;; esac

    if [ ! -f "$ROOT/$rel" ]; then
        printf 'MISSING   %s\n' "$rel"
        missing=$((missing + 1))
        continue
    fi

    actual=$(observe "$suite" "$rel")

    if [ "$actual" = "$expected" ]; then
        matched=$((matched + 1))
    elif [ "$expected" = "pass" ]; then
        printf 'REGRESSED %-58s was pass, now %s\n' "$rel" "$actual"
        regressions=$((regressions + 1))
    elif [ "$actual" = "pass" ]; then
        printf 'IMPROVED  %-58s was %s, now pass\n' "$rel" "$expected"
        improvements=$((improvements + 1))
    else
        # fail <-> timeout is not a change in outcome, just in how the ROM
        # gave up, so it is reported without counting either way.
        printf 'CHANGED   %-58s was %s, now %s\n' "$rel" "$expected" "$actual"
        matched=$((matched + 1))
    fi
done < "$EXPECTED"

passing=$(grep -c '^pass ' "$EXPECTED")
total=$(grep -vc -e '^#' -e '^[[:space:]]*$' "$EXPECTED")

printf '\n%s of %s ROMs expected to pass; %s as expected, %s regressed, %s improved' \
    "$passing" "$total" "$matched" "$regressions" "$improvements"
[ "$missing" -gt 0 ] && printf ', %s missing' "$missing"
printf '\n'

if [ "$improvements" -gt 0 ]; then
    echo "refresh the baseline to lock the improvements in: make test-roms-update"
fi

[ "$regressions" -eq 0 ] && [ "$missing" -eq 0 ]
