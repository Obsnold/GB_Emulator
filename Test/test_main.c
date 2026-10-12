/* test_main.c - test registry and runner.
 *
 * Each test runs in a forked child with its stderr captured, so a test that
 * segfaults is reported as a crashed test and the rest of the suite still
 * runs. That matters here: the bugs this suite is aimed at are memory-safety
 * bugs, and under a sanitizer a single bad read aborts the process.
 *
 *   Build/test_core                  run everything
 *   Build/test_core timer            run tests whose name contains "timer"
 *   Build/test_core --no-fork        run in-process, for gdb
 *
 * Exit status is the number of tests that failed, capped at 125.
 */
#include "test.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define TEST_CAPACITY 512
#define READ_CHUNK    4096

struct test_entry {
    const char *name;
    const char *file;
    test_fn     fn;
};

static struct test_entry g_tests[TEST_CAPACITY];
static int g_test_count = 0;

static int g_failures = 0;     /* failures in the test currently running */
static int g_reported = 0;     /* how many of those have been printed    */

void test_register(const char *name, const char *file, test_fn fn)
{
    if (g_test_count >= TEST_CAPACITY) {
        fprintf(stderr, "test registry full, raise TEST_CAPACITY\n");
        exit(2);
    }
    g_tests[g_test_count].name = name;
    g_tests[g_test_count].file = file;
    g_tests[g_test_count].fn   = fn;
    g_test_count++;
}

void test_record_failure(const char *file, int line, const char *fmt, ...)
{
    g_failures++;

    if (g_reported < TEST_MAX_REPORTED_FAILURES) {
        const char *base = strrchr(file, '/');
        va_list ap;

        fprintf(stderr, "    %s:%d: ", base ? base + 1 : file, line);
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fputc('\n', stderr);
        g_reported++;
    } else if (g_reported == TEST_MAX_REPORTED_FAILURES) {
        fprintf(stderr, "    ... further failures suppressed\n");
        g_reported++;
    }
}

/* Read a file descriptor to EOF. Caller frees. Returns NULL if empty. */
static char *slurp(int fd)
{
    char *buf = NULL;
    size_t len = 0, cap = 0;

    for (;;) {
        ssize_t n;

        if (len + READ_CHUNK + 1 > cap) {
            cap = cap ? cap * 2 : READ_CHUNK * 2;
            buf = realloc(buf, cap);
            if (buf == NULL) {
                return NULL;
            }
        }
        n = read(fd, buf + len, READ_CHUNK);
        if (n > 0) {
            len += (size_t)n;
        } else if (n == 0) {
            break;
        } else if (errno != EINTR) {
            break;
        }
    }

    if (buf != NULL) {
        buf[len] = '\0';
    }
    return (len > 0) ? buf : (free(buf), NULL);
}

/* A sanitizer report in a test's output means the test failed, even if its
 * assertions all passed and it exited cleanly. UBSan is built recoverable so
 * that one bad index does not abort the whole run, which would otherwise hide
 * every test after it. */
static int has_sanitizer_report(const char *text)
{
    if (text == NULL) {
        return 0;
    }
    return strstr(text, "runtime error:") != NULL ||
           strstr(text, "AddressSanitizer") != NULL ||
           strstr(text, "LeakSanitizer") != NULL;
}

/* Outcome of one test: >= 0 is a failure count, < 0 is the negated signal. */
static int run_forked(test_fn fn, char **output)
{
    int fds[2];
    pid_t pid;
    int status = 0;

    *output = NULL;

    if (pipe(fds) != 0) {
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], STDERR_FILENO);
        close(fds[1]);

        g_failures = 0;
        g_reported = 0;
        fn();
        fflush(stderr);

        _exit(g_failures > 120 ? 120 : g_failures);
    }

    close(fds[1]);
    /* Drain before reaping, or a chatty test deadlocks on a full pipe. */
    *output = slurp(fds[0]);
    close(fds[0]);
    waitpid(pid, &status, 0);

    if (WIFSIGNALED(status)) {
        return -WTERMSIG(status);
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

static int run_in_process(test_fn fn)
{
    g_failures = 0;
    g_reported = 0;
    fn();
    return g_failures;
}

static int by_file(const void *a, const void *b)
{
    const struct test_entry *ea = a, *eb = b;
    int c = strcmp(ea->file, eb->file);
    return c ? c : strcmp(ea->name, eb->name);
}

static void print_padded_name(const char *name)
{
    int pad = 52 - (int)strlen(name);

    fprintf(stderr, "  %s ", name);
    while (pad-- > 0) {
        fputc('.', stderr);
    }
    fputc(' ', stderr);
}

int test_run_all(const char *filter, int fork_each)
{
    int passed = 0, failed = 0, crashed = 0, sanitized = 0, skipped = 0;
    const char *last_file = NULL;
    int i;

    qsort(g_tests, (size_t)g_test_count, sizeof(g_tests[0]), by_file);

    for (i = 0; i < g_test_count; i++) {
        char *output = NULL;
        int result;
        const char *base;

        if (filter && !strstr(g_tests[i].name, filter)) {
            skipped++;
            continue;
        }

        base = strrchr(g_tests[i].file, '/');
        base = base ? base + 1 : g_tests[i].file;
        if (last_file == NULL || strcmp(last_file, g_tests[i].file) != 0) {
            fprintf(stderr, "\n[%s]\n", base);
            last_file = g_tests[i].file;
        }

        print_padded_name(g_tests[i].name);
        fflush(stderr);

        if (fork_each) {
            result = run_forked(g_tests[i].fn, &output);
        } else {
            fputc('\n', stderr);
            result = run_in_process(g_tests[i].fn);
        }

        if (result < 0) {
            fprintf(stderr, "CRASH (%s)\n", strsignal(-result));
            crashed++;
        } else if (has_sanitizer_report(output)) {
            fprintf(stderr, "SANITIZER%s\n",
                    result > 0 ? " + assertions" : "");
            sanitized++;
        } else if (result == 0) {
            fprintf(stderr, "ok\n");
            passed++;
        } else {
            fprintf(stderr, "FAIL (%d)\n", result);
            failed++;
        }

        if (output != NULL) {
            fputs(output, stderr);
            free(output);
        }
    }

    fprintf(stderr,
        "\n==========================================================\n"
        "%d ok, %d failed", passed, failed);
    if (sanitized) {
        fprintf(stderr, ", %d sanitizer", sanitized);
    }
    if (crashed) {
        fprintf(stderr, ", %d crashed", crashed);
    }
    if (skipped) {
        fprintf(stderr, ", %d filtered out", skipped);
    }
    fprintf(stderr, "\n");

    return failed + crashed + sanitized;
}

int main(int argc, char *argv[])
{
    const char *filter = NULL;
    int fork_each = 1;
    int i, result;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-fork") == 0) {
            fork_each = 0;
        } else if (argv[i][0] != '-') {
            filter = argv[i];
        }
    }

    result = test_run_all(filter, fork_each);
    return result > 125 ? 125 : result;
}
