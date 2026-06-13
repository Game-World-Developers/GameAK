#ifndef _CEST_H_
#define _CEST_H_

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#ifdef CEST_ENABLE_SIGNAL_HANDLER
#  include <signal.h>
#endif

#ifdef __OBJC__
#import <objc/objc.h>
#endif

#ifdef __cplusplus
#include <string>
#include <regex>
#include <initializer_list>
#endif

// ============================================================================
// Warning Suppression (enabled by default, can be disabled)
// ============================================================================
#ifndef CEST_SUPPRESS_WARNINGS
#  define CEST_SUPPRESS_WARNINGS 1
#endif

#if CEST_SUPPRESS_WARNINGS
#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#    pragma GCC diagnostic ignored "-Wstrict-prototypes"
#    pragma GCC diagnostic ignored "-Wunused-function"
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#    pragma GCC diagnostic ignored "-Wcast-align"
#  endif
#  ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpedantic"
#    pragma clang diagnostic ignored "-Wstrict-prototypes"
#    pragma clang diagnostic ignored "-Wunused-function"
#    pragma clang diagnostic ignored "-Wunused-parameter"
#    pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#    pragma clang diagnostic ignored "-Wcast-align"
#    pragma clang diagnostic ignored "-Wcovered-switch-default"
#  endif
#  ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable: 4100) // unreferenced formal parameter
#    pragma warning(disable: 4102) // unreferenced label
#    pragma warning(disable: 4189) // local variable is initialized
#    pragma warning(disable: 4201) // nameless struct/union
#    pragma warning(disable: 4214) // bitfield types other than int
#    pragma warning(disable: 4505) // unreferenced local function
#  endif
#endif

// ============================================================================
// Modern Language Version Detection
// ============================================================================
#ifndef __STDC_VERSION__
#  define __STDC_VERSION__ 0
#endif
#define _CEST_C11   (__STDC_VERSION__ >= 201112L)
#define _CEST_C17   (__STDC_VERSION__ >= 201710L)
#define _CEST_C23   (__STDC_VERSION__ >= 202000L)

#ifdef __cplusplus
#  if __cplusplus >= 201703L
#    define _CEST_CPP17 1
#  endif
#  if __cplusplus >= 202002L
#    define _CEST_CPP20 1
#  endif
#  if __cplusplus >= 202302L
#    define _CEST_CPP23 1
#  endif
#endif

#ifdef __OBJC__
#  ifdef __has_feature
#    if __has_feature(objc_arc)
#      define _CEST_OBJC_ARC 1
#    endif
#  endif
#endif

// ============================================================================
// CONFIGURATION MACROS (define before including cest.h)
// ============================================================================
// #define CEST_NO_COLORS             // Disable colored output
// #define CEST_THREAD_SAFE           // Enable thread safety (requires pthreads)
// #define CEST_NO_CLI                // Disable CLI argument parsing
// #define CEST_NO_HOOKS              // Disable beforeEach/afterEach hooks
// #define CEST_ENABLE_ARC            // Enable ARC support for Objective-C
// #define CEST_ENABLE_SKIP           // Enable skip/only test modifiers
// #define CEST_ENABLE_FORK           // Enable test isolation with fork()
// #define CEST_ENABLE_COVERAGE       // Enable gcov coverage integration
// #define CEST_ENABLE_LEAK_DETECTION // Enable memory leak detection (disabled if sanitizer active)
// #define CEST_ENABLE_SIGNAL_HANDLER // Enable crash diagnostics (SIGSEGV, SIGABRT, etc.)
// #define CEST_PREFIX                // Use cest_ prefix on all public macros

// ============================================================================
// Automatic Sanitizer Detection (before any configuration)
// ============================================================================
#ifndef CEST_SANITIZER_FLAGS
#  ifdef __has_feature
#    if __has_feature(address_sanitizer)
#      define CEST_ASAN_ACTIVE 1
#    endif
#    if __has_feature(thread_sanitizer)
#      define CEST_TSAN_ACTIVE 1
#    endif
#    if __has_feature(memory_sanitizer)
#      define CEST_MSAN_ACTIVE 1
#    endif
#    if __has_feature(undefined_sanitizer)
#      define CEST_UBSAN_ACTIVE 1
#    endif
#  endif
#  if defined(__SANITIZE_ADDRESS__) && !defined(CEST_ASAN_ACTIVE)
#    define CEST_ASAN_ACTIVE 1
#  endif
#  if defined(__SANITIZE_THREAD__) && !defined(CEST_TSAN_ACTIVE)
#    define CEST_TSAN_ACTIVE 1
#  endif
#  if defined(__SANITIZE_MEMORY__) && !defined(CEST_MSAN_ACTIVE)
#    define CEST_MSAN_ACTIVE 1
#  endif
#  if defined(__SANITIZE_UNDEFINED__) && !defined(CEST_UBSAN_ACTIVE)
#    define CEST_UBSAN_ACTIVE 1
#  endif

// If any sanitizer is active, disable internal leak detection to avoid conflicts
#  if defined(CEST_ASAN_ACTIVE) || defined(CEST_TSAN_ACTIVE) || defined(CEST_MSAN_ACTIVE) || defined(CEST_UBSAN_ACTIVE)
#    ifdef CEST_ENABLE_LEAK_DETECTION
#      undef CEST_ENABLE_LEAK_DETECTION
#      pragma message("Cest: Leak detection disabled because a sanitizer is active.")
#    endif
#  endif
#endif

// ============================================================================
// Sanitizer Suppression Macros (use before functions that intentionally trigger UB)
// ============================================================================
#if defined(__GNUC__) || defined(__clang__)
#  define CEST_NO_SANITIZE_ADDRESS   __attribute__((no_sanitize("address")))
#  define CEST_NO_SANITIZE_THREAD    __attribute__((no_sanitize("thread")))
#  define CEST_NO_SANITIZE_MEMORY    __attribute__((no_sanitize("memory")))
#  define CEST_NO_SANITIZE_UNDEFINED __attribute__((no_sanitize("undefined")))
#  define CEST_NO_SANITIZE_ALL       __attribute__((no_sanitize("address", "thread", "memory", "undefined")))
#elif defined(_MSC_VER)
#  define CEST_NO_SANITIZE_ADDRESS   __declspec(no_sanitize_address)
#  define CEST_NO_SANITIZE_THREAD    /* not supported */
#  define CEST_NO_SANITIZE_MEMORY    /* not supported */
#  define CEST_NO_SANITIZE_UNDEFINED /* not supported */
#  define CEST_NO_SANITIZE_ALL       /* not supported */
#else
#  define CEST_NO_SANITIZE_ADDRESS
#  define CEST_NO_SANITIZE_THREAD
#  define CEST_NO_SANITIZE_MEMORY
#  define CEST_NO_SANITIZE_UNDEFINED
#  define CEST_NO_SANITIZE_ALL
#endif

// ============================================================================
// Valgrind Integration
// ============================================================================
#ifdef __has_include
#  if __has_include(<valgrind/valgrind.h>)
#    include <valgrind/valgrind.h>
#    define CEST_VALGRIND_ACTIVE RUNNING_ON_VALGRIND
#    if __has_include(<valgrind/memcheck.h>)
#      include <valgrind/memcheck.h>
#      define CEST_VALGRIND_MEMCHECK_AVAILABLE 1
#    endif
#  else
#    define CEST_VALGRIND_ACTIVE 0
#  endif
#else
#  define CEST_VALGRIND_ACTIVE 0
#endif

// Macro to skip a test when running under Valgrind
#define CEST_SKIP_IF_VALGRIND() \
    do { if (CEST_VALGRIND_ACTIVE) { \
        printf("  " CEST_CLR_YELLOW "⊘ %s skipped (Valgrind)" CEST_CLR_RESET "\n", __func__); \
        _cest_global_stats.skipped++; \
        return; \
    } } while(0)

// Macro to expect no Valgrind errors
#ifdef CEST_VALGRIND_MEMCHECK_AVAILABLE
#  define CEST_EXPECT_NO_VALGRIND_ERRORS() \
      do { \
          if (CEST_VALGRIND_ACTIVE) { \
              long errs = VALGRIND_COUNT_ERRORS; \
              if (errs > 0) { \
                  printf("  " CEST_CLR_RED "✕ Valgrind detected %ld errors" CEST_CLR_RESET "\n", errs); \
                  _cest_global_stats.failed++; \
              } else { \
                  printf("  " CEST_CLR_GREEN "✓ No Valgrind errors" CEST_CLR_RESET "\n"); \
              } \
          } \
      } while(0)
#else
#  define CEST_EXPECT_NO_VALGRIND_ERRORS()
#endif

// ============================================================================
// Portability - MSVC and Cross-Platform Support
// ============================================================================
#ifndef CEST_WEAK
#  if defined(_MSC_VER)
#    if defined(__cplusplus)
#      define CEST_WEAK static
#    else
#      define CEST_WEAK __declspec(selectany) extern
#    endif
#  else
#    define CEST_WEAK __attribute__((weak))
#  endif
#endif

// Thread safety support
#ifdef CEST_THREAD_SAFE
#  include <pthread.h>
#  define CEST_LOCK() pthread_mutex_lock(&_cest_mutex)
#  define CEST_UNLOCK() pthread_mutex_unlock(&_cest_mutex)
#else
#  define CEST_LOCK()
#  define CEST_UNLOCK()
#endif

// Windows console color support
#ifdef _WIN32
#  ifndef CEST_NO_COLORS
#    include <windows.h>
#    define _CEST_INIT_COLORS() do { \
        DWORD mode; \
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); \
        if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING); \
    } while(0)
#  endif
#endif

#ifndef _CEST_INIT_COLORS
#  define _CEST_INIT_COLORS()
#endif

// CI Environment Detection
static inline int _cest_is_ci(void) {
    return getenv("CI") != NULL ||
           getenv("GITHUB_ACTIONS") != NULL ||
           getenv("GITLAB_CI") != NULL ||
           getenv("JENKINS_HOME") != NULL;
}

// Internal Math Helpers (To avoid -lm dependency)
#define _CEST_ABS(x) ((x) < 0 ? -(x) : (x))

// Color codes
#ifndef CEST_NO_COLORS
#  define CEST_CLR_RESET   "\033[0m"
#  define CEST_CLR_RED     "\033[31m"
#  define CEST_CLR_GREEN   "\033[32m"
#  define CEST_CLR_YELLOW  "\033[33m"
#  define CEST_CLR_BLUE    "\033[34m"
#  define CEST_CLR_BOLD    "\033[1m"
#  define CEST_CLR_DIM     "\033[90m"
#else
#  define CEST_CLR_RESET   ""
#  define CEST_CLR_RED     ""
#  define CEST_CLR_GREEN   ""
#  define CEST_CLR_YELLOW  ""
#  define CEST_CLR_BLUE    ""
#  define CEST_CLR_BOLD    ""
#  define CEST_CLR_DIM     ""
#endif

// Types and Values
typedef enum {
    CEST_TYPE_INT,
    CEST_TYPE_DOUBLE,
    CEST_TYPE_STR,
    CEST_TYPE_PTR,
    CEST_TYPE_BOOL,
    CEST_TYPE_OBJC_ID,
    CEST_TYPE_REGEX,
    CEST_TYPE_ARRAY,
    CEST_TYPE_CHAR8,
    CEST_TYPE_CHAR16,
    CEST_TYPE_CHAR32,
    CEST_TYPE_BYTE
} cest_type_t;

typedef struct {
    cest_type_t type;
    union {
        long long i;
        double d;
        const char *s;
        const void *p;
        bool b;
        unsigned char u8;
#ifdef __OBJC__
        id obj;
#endif
#ifdef __cplusplus
        std::regex* re;
#endif
    } as;
    int len;
} cest_value_t;

typedef int (*cest_match_fn)(cest_value_t actual, cest_value_t expected, int* diff_pos);

typedef void (*cest_test_fn)(void);

// ============================================================================
// Leak Detection (optional - enable with CEST_ENABLE_LEAK_DETECTION)
// ============================================================================
#ifdef CEST_ENABLE_LEAK_DETECTION
CEST_WEAK long long _cest_alloc_count = 0;
CEST_WEAK long long _cest_free_count = 0;

#define cest_malloc(size) _cest_malloc(size, __FILE__, __LINE__)
#define cest_free(ptr) _cest_free(ptr, __FILE__, __LINE__)

static inline void* _cest_malloc(size_t size, const char* file, int line) {
    (void)file; (void)line;
    _cest_alloc_count++;
    return malloc(size);
}

static inline void _cest_free(void* ptr, const char* file, int line) {
    (void)file; (void)line;
    if (ptr) _cest_free_count++;
    free(ptr);
}
#else
#define cest_malloc(size) malloc(size)
#define cest_free(ptr) free(ptr)
#endif

// ============================================================================
// Forked test execution (isolation for sanitizers/crashes)
// ============================================================================
#ifdef CEST_ENABLE_FORK
#  ifndef _WIN32
#    include <sys/wait.h>
#    include <unistd.h>
static inline int _cest_run_forked_test(cest_test_fn test_fn, char* error_msg, size_t msg_size) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe
        close(pipefd[1]);
        test_fn();
        exit(0);
    } else { // Parent process
        close(pipefd[1]); // Close write end
        int status;
        waitpid(pid, &status, 0);

        // Read stderr from child
        ssize_t bytes_read = read(pipefd[0], error_msg, msg_size - 1);
        if (bytes_read > 0) {
            error_msg[bytes_read] = '\0';
        } else {
            error_msg[0] = '\0';
        }
        close(pipefd[0]);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return -WTERMSIG(status);
        }
        return -2;
    }
}
#  else
// Windows fallback (no fork, just run directly)
static inline int _cest_run_forked_test(cest_test_fn test_fn, char* error_msg, size_t msg_size) {
    (void)error_msg; (void)msg_size;
    test_fn();
    return 0;
}
#  endif
#endif

// ============================================================================
// Hook Identifiers
// ============================================================================
typedef enum {
    CEST_HOOK_NONE         = 0,
    CEST_HOOK_BEFORE_EACH  = 1,
    CEST_HOOK_AFTER_EACH   = 2,
    CEST_HOOK_BEFORE_ALL   = 3,
    CEST_HOOK_AFTER_ALL    = 4
} cest_hook_id_t;

// ============================================================================
// Signal Handler (crash diagnostics without fork)
// ============================================================================
#ifdef CEST_ENABLE_SIGNAL_HANDLER
static volatile const char* _cest_signal_test_name = NULL;
static volatile int _cest_in_hook = CEST_HOOK_NONE;

static const char* _cest_signal_name(int sig) {
    switch(sig) {
        case SIGSEGV: return "SIGSEGV (Segmentation Fault)";
        case SIGABRT: return "SIGABRT (Abort)";
        case SIGFPE:  return "SIGFPE (Floating Point Exception)";
        case SIGBUS:  return "SIGBUS (Bus Error)";
        case SIGILL:  return "SIGILL (Illegal Instruction)";
        default:      return "Unknown Signal";
    }
}

static const char* _cest_hook_name(int hook_id) {
    switch(hook_id) {
        case CEST_HOOK_BEFORE_EACH: return "beforeEach";
        case CEST_HOOK_AFTER_EACH:  return "afterEach";
        case CEST_HOOK_BEFORE_ALL:  return "beforeAll";
        case CEST_HOOK_AFTER_ALL:   return "afterAll";
        default: return "unknown hook";
    }
}

static void _cest_signal_handler(int sig) {
    // Use write() instead of printf() — async-signal-safe
    const char* msg_prefix = "\n  \033[31m✕ CRASH: ";
    const char* msg_signal = _cest_signal_name(sig);
    const char* msg_reset = "\033[0m\n";
    (void)!write(STDERR_FILENO, msg_prefix, strlen(msg_prefix));
    (void)!write(STDERR_FILENO, msg_signal, strlen(msg_signal));
    if (_cest_in_hook) {
        const char* hook_msg = " in hook: ";
        const char* hook_name = _cest_hook_name(_cest_in_hook);
        (void)!write(STDERR_FILENO, hook_msg, strlen(hook_msg));
        (void)!write(STDERR_FILENO, hook_name, strlen(hook_name));
    }
    if (_cest_signal_test_name) {
        const char* test_msg = " during test: ";
        (void)!write(STDERR_FILENO, test_msg, strlen(test_msg));
        (void)!write(STDERR_FILENO, (const char*)_cest_signal_test_name, strlen((const char*)_cest_signal_test_name));
    }
    (void)!write(STDERR_FILENO, msg_reset, strlen(msg_reset));
    _exit(128 + sig);
}

static inline void _cest_install_signal_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = _cest_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND; // One-shot: restore default after first signal
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGFPE,  &sa, NULL);
    sigaction(SIGBUS,  &sa, NULL);
    sigaction(SIGILL,  &sa, NULL);
}
#  define _CEST_SIGNAL_INIT()         _cest_install_signal_handlers()
#  define _CEST_SIGNAL_SET_TEST(name) (_cest_signal_test_name = (name))
#  define _CEST_SIGNAL_SET_HOOK(id)   (_cest_in_hook = (id))
#  define _CEST_SIGNAL_CLEAR_HOOK()   (_cest_in_hook = CEST_HOOK_NONE)
#else
#  define _CEST_SIGNAL_INIT()
#  define _CEST_SIGNAL_SET_TEST(name) ((void)(name))
#  define _CEST_SIGNAL_SET_HOOK(id)   ((void)(id))
#  define _CEST_SIGNAL_CLEAR_HOOK()
#endif

// ============================================================================
// Stats and Globals
// ============================================================================
typedef struct {
    int passed;
    int failed;
    int skipped;
    const char* filter_pattern;
} cest_stats_t;

CEST_WEAK cest_stats_t _cest_global_stats = {0, 0, 0, NULL};
CEST_WEAK const char* _cest_current_test_name = NULL;
CEST_WEAK int _cest_sanitize_flags = 0; // Bitmask for active sanitizers (from CLI)
CEST_WEAK bool _cest_sanitize_errors_as_failures = true;
CEST_WEAK const char* _cest_junit_output = NULL;
CEST_WEAK const char* _cest_json_output = NULL;
CEST_WEAK double _cest_total_time = 0.0;
CEST_WEAK clock_t _cest_suite_start_time = 0;

// Test state for skip/only
typedef enum {
    CEST_TEST_NORMAL,
    CEST_TEST_SKIP,
    CEST_TEST_ONLY
} cest_test_state_t;
CEST_WEAK int _cest_current_test_state = CEST_TEST_NORMAL;

// ============================================================================
// Coverage Support
// ============================================================================
#ifdef CEST_ENABLE_COVERAGE
#  define CEST_COVERAGE_INIT() \
    do { \
        extern void __gcov_flush(void); \
        atexit(__gcov_flush); \
    } while(0)
#  define _CEST_COVERAGE_BEFORE_EACH() \
    do { \
        extern void __gcov_reset(void); \
        __gcov_reset(); \
    } while(0)
#  define _CEST_COVERAGE_AFTER_EACH() \
    do { \
        extern void __gcov_dump(void); \
        __gcov_dump(); \
    } while(0)
#else
#  define CEST_COVERAGE_INIT()
#  define _CEST_COVERAGE_BEFORE_EACH()
#  define _CEST_COVERAGE_AFTER_EACH()
#endif

// ============================================================================
// Value Wrappers
// ============================================================================
static inline cest_value_t cest_int(long long v) { cest_value_t cv; cv.type = CEST_TYPE_INT; cv.as.i = v; cv.len = 0; return cv; }
static inline cest_value_t cest_double(double v) { cest_value_t cv; cv.type = CEST_TYPE_DOUBLE; cv.as.d = v; cv.len = 0; return cv; }
static inline cest_value_t cest_str(const char* v) { cest_value_t cv; cv.type = CEST_TYPE_STR; cv.as.s = v ? v : "NULL"; cv.len = 0; return cv; }
static inline cest_value_t cest_ptr(const void* v) { cest_value_t cv; cv.type = CEST_TYPE_PTR; cv.as.p = v; cv.len = 0; return cv; }
static inline cest_value_t cest_bool(bool v) { cest_value_t cv; cv.type = CEST_TYPE_BOOL; cv.as.b = v; cv.len = 0; return cv; }
static inline cest_value_t cest_array(const void* v, int len) { cest_value_t cv; cv.type = CEST_TYPE_ARRAY; cv.as.p = v; cv.len = len; return cv; }
#ifdef __OBJC__
static inline cest_value_t cest_id(id v) { cest_value_t cv; cv.type = CEST_TYPE_OBJC_ID; cv.as.obj = v; cv.len = 0; return cv; }
#endif

#ifdef __cplusplus
static inline cest_value_t cest_value(bool v) { return cest_bool(v); }
static inline cest_value_t cest_value(int v) { return cest_int(v); }
static inline cest_value_t cest_value(long v) { return cest_int(v); }
static inline cest_value_t cest_value(long long v) { return cest_int(v); }
static inline cest_value_t cest_value(float v) { return cest_double(v); }
static inline cest_value_t cest_value(double v) { return cest_double(v); }
static inline cest_value_t cest_value(char* v) { return cest_str(v); }
static inline cest_value_t cest_value(const char* v) { return cest_str(v); }
static inline cest_value_t cest_value(const std::string& v) { return cest_str(v.c_str()); }
#ifdef __OBJC__
static inline cest_value_t cest_value(id v) { return cest_id(v); }
#endif
template<typename T>
static inline cest_value_t cest_value(T* v) { return cest_ptr((const void*)v); }
#else
#define cest_value(x) \
    _Generic((x), \
        _Bool: cest_bool, \
        int: cest_int, \
        long: cest_int, \
        long long: cest_int, \
        float: cest_double, \
        double: cest_double, \
        char*: cest_str, \
        const char*: cest_str, \
        _CEST_OBJC_GENERIC(x) \
        default: cest_ptr \
    )(x)

#ifdef __OBJC__
#  define _CEST_OBJC_GENERIC(x) id: cest_id,
#else
#  define _CEST_OBJC_GENERIC(x)
#endif
#endif

// ============================================================================
// Core Assertion Implementation
// ============================================================================
static struct {
    const char* file;
    int line;
    const char* actual_expr;
    cest_value_t actual;
    cest_value_t expected;
    const char* expected_expr;
    int valid; // 1 = context is set by expect(), 0 = consumed by assert
} _cest_ctx;

// Reset context to a clean state — prevents stale data leaking between asserts
static inline void _cest_ctx_reset(void) {
    _cest_ctx.file = NULL;
    _cest_ctx.line = 0;
    _cest_ctx.actual_expr = NULL;
    memset(&_cest_ctx.actual, 0, sizeof(cest_value_t));
    memset(&_cest_ctx.expected, 0, sizeof(cest_value_t));
    _cest_ctx.expected_expr = NULL;
    _cest_ctx.valid = 0;
}

static inline void _cest_print_value(cest_value_t v) {
    switch(v.type) {
        case CEST_TYPE_INT: printf("(%s: %lld)", "int", v.as.i); break;
        case CEST_TYPE_DOUBLE: printf("(%s: %g)", "double", v.as.d); break;
        case CEST_TYPE_STR: printf("(%s: \"%s\")", "string", v.as.s); break;
        case CEST_TYPE_PTR: printf("(%s: %p)", "pointer", v.as.p); break;
        case CEST_TYPE_BOOL: printf("(%s: %s)", "bool", v.as.b ? "true" : "false"); break;
        case CEST_TYPE_ARRAY: printf("(%s[%d]: %p)", "array", v.len, v.as.p); break;
#ifdef __OBJC__
        case CEST_TYPE_OBJC_ID: printf("(%s: %p)", "id", (void*)v.as.obj); break;
#else
        case CEST_TYPE_OBJC_ID: printf("(%s: %p)", "id", v.as.p); break;
#endif
#ifdef __cplusplus
        case CEST_TYPE_REGEX: printf("(%s)", "regex"); break;
#else
        case CEST_TYPE_REGEX: printf("(%s)", "regex"); break;
#endif
        default: printf("(unknown)"); break;
    }
}

static inline void _cest_assert_impl(cest_value_t expected, cest_match_fn match, const char* match_name, const char* expected_expr) {
    int diff_pos = -1;
    int passed = match(_cest_ctx.actual, expected, &diff_pos);
    if (passed) {
        printf("  " CEST_CLR_GREEN "✓" CEST_CLR_RESET " %s %s %s\n", _cest_ctx.actual_expr, match_name, expected_expr);
        _cest_global_stats.passed++;
    } else {
        printf("  " CEST_CLR_RED "✕ %s failed" CEST_CLR_RESET "\n", _cest_ctx.actual_expr);
        printf("    " CEST_CLR_GREEN "Expected %s: %s" CEST_CLR_RESET "\n", match_name, expected_expr);
        printf("    " CEST_CLR_RED "Received "); _cest_print_value(_cest_ctx.actual); printf(CEST_CLR_RESET "\n");
        if (_cest_ctx.actual.type == CEST_TYPE_STR && expected.type == CEST_TYPE_STR) {
            if (diff_pos >= 0) {
                printf("    " CEST_CLR_DIM "Diff at position %d: '%c' vs '%c'" CEST_CLR_RESET "\n", 
                       diff_pos, _cest_ctx.actual.as.s[diff_pos], expected.as.s[diff_pos]);
            }
        }
        if (_cest_ctx.actual.type == CEST_TYPE_DOUBLE && expected.type == CEST_TYPE_DOUBLE) {
            double diff = _CEST_ABS(_cest_ctx.actual.as.d - expected.as.d);
            printf("    " CEST_CLR_DIM "Diff: %g" CEST_CLR_RESET "\n", diff);
        }
        if (_cest_ctx.actual.type == CEST_TYPE_ARRAY && expected.type == CEST_TYPE_ARRAY) {
            if (_cest_ctx.actual.len != expected.len) {
                printf("    " CEST_CLR_DIM "Length mismatch: %d vs %d" CEST_CLR_RESET "\n", _cest_ctx.actual.len, expected.len);
            } else {
                printf("    " CEST_CLR_DIM "Arrays differ in content" CEST_CLR_RESET "\n");
            }
        }
        printf("    " CEST_CLR_DIM "(%s:%d)" CEST_CLR_RESET "\n", _cest_ctx.file, _cest_ctx.line);
        _cest_global_stats.failed++;
    }
    fflush(stdout); // Ensure output is visible even if test crashes after this assert
    _cest_ctx.valid = 0; // Mark context as consumed
}

// ============================================================================
// Matcher Logic
// ============================================================================
static inline int match_eq(cest_value_t a, cest_value_t b, int* diff_pos) {
    if (diff_pos) *diff_pos = -1;
    if (a.type != b.type) {
#ifdef __OBJC__
        if ((a.type == CEST_TYPE_PTR && b.type == CEST_TYPE_OBJC_ID) ||
            (a.type == CEST_TYPE_OBJC_ID && b.type == CEST_TYPE_PTR)) {
            const void *p = a.type == CEST_TYPE_PTR ? a.as.p : b.as.p;
            id obj = a.type == CEST_TYPE_OBJC_ID ? a.as.obj : b.as.obj;
            return p == (const void*)obj;
        }
        if ((a.type == CEST_TYPE_PTR || a.type == CEST_TYPE_OBJC_ID) && 
            (b.type == CEST_TYPE_PTR || b.type == CEST_TYPE_OBJC_ID)) {
            const void *p1 = a.type == CEST_TYPE_PTR ? a.as.p : (const void*)a.as.obj;
            const void *p2 = b.type == CEST_TYPE_PTR ? b.as.p : (const void*)b.as.obj;
            return p1 == p2;
        }
#endif
        return 0;
    }
    switch (a.type) {
        case CEST_TYPE_INT: return a.as.i == b.as.i;
        case CEST_TYPE_DOUBLE: return _CEST_ABS(a.as.d - b.as.d) < 0.000001;
        case CEST_TYPE_STR: {
            if (strcmp(a.as.s, b.as.s) == 0) return 1;
            if (diff_pos) {
                const char* p1 = a.as.s;
                const char* p2 = b.as.s;
                while (*p1 && *p2) {
                    if (*p1 != *p2) break;
                    p1++; p2++;
                }
                *diff_pos = (int)(p1 - a.as.s);
            }
            return 0;
        }
        case CEST_TYPE_PTR: return a.as.p == b.as.p;
        case CEST_TYPE_BOOL: return a.as.b == b.as.b;
        case CEST_TYPE_ARRAY: {
            if (a.len != b.len) return 0;
            if (a.len == 0) return 1;
            return memcmp(a.as.p, b.as.p, a.len) == 0;
        }
#ifdef __OBJC__
        case CEST_TYPE_OBJC_ID: return a.as.obj == b.as.obj;
#endif
        default: return 0;
    }
}

static inline int match_gt(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)diff_pos;
    if (a.type == CEST_TYPE_INT && b.type == CEST_TYPE_INT) return a.as.i > b.as.i;
    if (a.type == CEST_TYPE_DOUBLE && b.type == CEST_TYPE_DOUBLE) return a.as.d > b.as.d;
    return 0;
}

static inline int match_lt(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)diff_pos;
    if (a.type == CEST_TYPE_INT && b.type == CEST_TYPE_INT) return a.as.i < b.as.i;
    if (a.type == CEST_TYPE_DOUBLE && b.type == CEST_TYPE_DOUBLE) return a.as.d < b.as.d;
    return 0;
}

static inline int match_in_range(cest_value_t a, cest_value_t min, cest_value_t max, int* diff_pos) {
    (void)diff_pos;
    if (a.type == CEST_TYPE_INT && min.type == CEST_TYPE_INT && max.type == CEST_TYPE_INT) {
        return a.as.i >= min.as.i && a.as.i <= max.as.i;
    }
    if (a.type == CEST_TYPE_DOUBLE && min.type == CEST_TYPE_DOUBLE && max.type == CEST_TYPE_DOUBLE) {
        return a.as.d >= min.as.d && a.as.d <= max.as.d;
    }
    return 0;
}

static inline int match_start_with(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)diff_pos;
    if (a.type == CEST_TYPE_STR && b.type == CEST_TYPE_STR) {
        return strncmp(a.as.s, b.as.s, strlen(b.as.s)) == 0;
    }
    return 0;
}

static inline int match_end_with(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)diff_pos;
    if (a.type == CEST_TYPE_STR && b.type == CEST_TYPE_STR) {
        size_t len_a = strlen(a.as.s);
        size_t len_b = strlen(b.as.s);
        if (len_b > len_a) return 0;
        return strcmp(a.as.s + len_a - len_b, b.as.s) == 0;
    }
    return 0;
}

static inline int match_contain(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)diff_pos;
    if (a.type == CEST_TYPE_STR && b.type == CEST_TYPE_STR) return strstr(a.as.s, b.as.s) != NULL;
    return 0;
}

static inline int match_regex(cest_value_t a, cest_value_t b, int* diff_pos) {
#ifdef __cplusplus
    (void)diff_pos;
    if (a.type == CEST_TYPE_STR && b.type == CEST_TYPE_REGEX) {
        return std::regex_match(a.as.s, *b.as.re);
    }
#else
    (void)a; (void)b; (void)diff_pos;
#endif
    return 0;
}

static inline int match_truthy(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)b; (void)diff_pos;
    switch (a.type) {
        case CEST_TYPE_BOOL:   return a.as.b;
        case CEST_TYPE_INT:    return a.as.i != 0;
        case CEST_TYPE_DOUBLE: return a.as.d != 0.0;
        case CEST_TYPE_STR:    return a.as.s != NULL && strlen(a.as.s) > 0;
        case CEST_TYPE_PTR:    return a.as.p != NULL;
#ifdef __OBJC__
        case CEST_TYPE_OBJC_ID: return a.as.obj != nil;
#endif
        default: return 0;
    }
}

static inline int match_falsy(cest_value_t a, cest_value_t b, int* diff_pos) {
    return !match_truthy(a, b, diff_pos);
}

static inline int match_defined(cest_value_t a, cest_value_t b, int* diff_pos) {
    (void)b; (void)diff_pos;
    if (a.type == CEST_TYPE_PTR && a.as.p == NULL) return 0;
#ifdef __OBJC__
    if (a.type == CEST_TYPE_OBJC_ID && a.as.obj == nil) return 0;
#endif
    return 1;
}

static inline int match_undefined(cest_value_t a, cest_value_t b, int* diff_pos) {
    return !match_defined(a, b, diff_pos);
}

// ============================================================================
// Fluent API Bridge
// ============================================================================
typedef struct {
    void (*_toEqual)(cest_value_t expected, const char* expected_expr);
    void (*_toBe)(cest_value_t expected, const char* expected_expr);
    void (*_toBeGreaterThan)(cest_value_t expected, const char* expected_expr);
    void (*_toBeLessThan)(cest_value_t expected, const char* expected_expr);
    void (*_toContain)(cest_value_t expected, const char* expected_expr);
    void (*_toBeInRange)(cest_value_t min, cest_value_t max, const char* range_expr);
    void (*_toStartWith)(cest_value_t expected, const char* expected_expr);
    void (*_toEndWith)(cest_value_t expected, const char* expected_expr);
    void (*_toBeNull)(void);
    void (*_toBeTruthy)(void);
    void (*_toBeFalsy)(void);
    void (*_toBeCloseTo)(double val, double precision);
    void (*_toEqualArray)(cest_value_t expected, const char* expected_expr);
    void (*_toMatch)(cest_value_t expected, const char* expected_expr);
    void (*_toBeDefined)(void);
    void (*_toBeUndefined)(void);
} _cest_bridge_t;

static inline void b_toEqual(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_eq, "to equal", ee); }
static inline void b_toEqualArray(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_eq, "to equal array", ee); }
static inline void b_toBeGreaterThan(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_gt, "to be greater than", ee); }
static inline void b_toBeLessThan(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_lt, "to be less than", ee); }
static inline void b_toContain(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_contain, "to contain", ee); }
static inline void b_toStartWith(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_start_with, "to start with", ee); }
static inline void b_toEndWith(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_end_with, "to end with", ee); }
static inline void b_toMatch(cest_value_t e, const char* ee) { _cest_assert_impl(e, match_regex, "to match", ee); }
static inline void b_toBeNull(void) { _cest_assert_impl(cest_ptr(NULL), match_eq, "to be null", "NULL"); }
static inline void b_toBeTruthy(void) { _cest_assert_impl(cest_bool(true), match_truthy, "to be", "truthy"); }
static inline void b_toBeFalsy(void) { _cest_assert_impl(cest_bool(false), match_falsy, "to be", "falsy"); }
static inline void b_toBeDefined(void) { _cest_assert_impl(cest_bool(true), match_defined, "to be", "defined"); }
static inline void b_toBeUndefined(void) { _cest_assert_impl(cest_bool(true), match_undefined, "to be", "undefined"); }
static inline void b_toBeCloseTo(double val, double precision) {
    double diff = _CEST_ABS(_cest_ctx.actual.as.d - val);
    if (diff < precision) {
        printf("  " CEST_CLR_GREEN "✓" CEST_CLR_RESET " %s to be close to %g (precision %g)\n", _cest_ctx.actual_expr, val, precision);
        _cest_global_stats.passed++;
    } else {
        printf("  " CEST_CLR_RED "✕ %s to be close to" CEST_CLR_RESET "\n", _cest_ctx.actual_expr);
        printf("    " CEST_CLR_GREEN "Expected to be close to: %g (precision %g)" CEST_CLR_RESET "\n", val, precision);
        printf("    " CEST_CLR_RED "Received (double: %g) (diff %g)" CEST_CLR_RESET "\n", _cest_ctx.actual.as.d, diff);
        printf("    " CEST_CLR_DIM "(%s:%d)" CEST_CLR_RESET "\n", _cest_ctx.file, _cest_ctx.line);
        _cest_global_stats.failed++;
    }
    fflush(stdout);
    _cest_ctx.valid = 0;
}
static inline void b_toBeInRange(cest_value_t min, cest_value_t max, const char* re) {
    int passed = match_in_range(_cest_ctx.actual, min, max, NULL);
    if (passed) {
        printf("  " CEST_CLR_GREEN "✓" CEST_CLR_RESET " %s to be in range %s\n", _cest_ctx.actual_expr, re);
        _cest_global_stats.passed++;
    } else {
        printf("  " CEST_CLR_RED "✕ %s to be in range" CEST_CLR_RESET "\n", _cest_ctx.actual_expr);
        printf("    " CEST_CLR_GREEN "Expected range: %s" CEST_CLR_RESET "\n", re);
        printf("    " CEST_CLR_RED "Received "); _cest_print_value(_cest_ctx.actual); printf(CEST_CLR_RESET "\n");
        printf("    " CEST_CLR_DIM "(%s:%d)" CEST_CLR_RESET "\n", _cest_ctx.file, _cest_ctx.line);
        _cest_global_stats.failed++;
    }
    fflush(stdout);
    _cest_ctx.valid = 0;
}

static _cest_bridge_t _cest_bridge __attribute__((unused)) = {
    ._toEqual = b_toEqual,
    ._toBe = b_toEqual,
    ._toBeGreaterThan = b_toBeGreaterThan,
    ._toBeLessThan = b_toBeLessThan,
    ._toContain = b_toContain,
    ._toBeInRange = b_toBeInRange,
    ._toStartWith = b_toStartWith,
    ._toEndWith = b_toEndWith,
    ._toBeNull = b_toBeNull,
    ._toBeTruthy = b_toBeTruthy,
    ._toBeFalsy = b_toBeFalsy,
    ._toBeCloseTo = b_toBeCloseTo,
    ._toEqualArray = b_toEqualArray,
    ._toMatch = b_toMatch,
    ._toBeDefined = b_toBeDefined,
    ._toBeUndefined = b_toBeUndefined
};

#define expect(x) (_cest_ctx_reset(), _cest_ctx.file = __FILE__, _cest_ctx.line = __LINE__, _cest_ctx.actual_expr = #x, _cest_ctx.actual = cest_value(x), _cest_ctx.valid = 1, _cest_bridge)
#define expect_array(x, len) (_cest_ctx_reset(), _cest_ctx.file = __FILE__, _cest_ctx.line = __LINE__, _cest_ctx.actual_expr = #x, _cest_ctx.actual = cest_array(x, len), _cest_ctx.valid = 1, _cest_bridge)

#define toEqual(x) _toEqual(cest_value(x), #x)
#define toBe(x) _toBe(cest_value(x), #x)
#define toBeGreaterThan(x) _toBeGreaterThan(cest_value(x), #x)
#define toBeLessThan(x) _toBeLessThan(cest_value(x), #x)
#define toContain(x) _toContain(cest_value(x), #x)
#define toBeInRange(min, max) _toBeInRange(cest_value(min), cest_value(max), #min " to " #max)
#define toStartWith(x) _toStartWith(cest_value(x), #x)
#define toEndWith(x) _toEndWith(cest_value(x), #x)
#define toBeNull() _toBeNull()
#define toBeTruthy() _toBeTruthy()
#define toBeFalsy() _toBeFalsy()
#define toBeCloseTo(v, p) _toBeCloseTo(v, p)
#define toEqualArray(x, len) _toEqualArray(cest_array(x, len), #x "(" #x ", " #len ")")
#define toMatch(regex) _toMatch(cest_value(regex), #regex)
#define toBeDefined() _toBeDefined()
#define toBeUndefined() _toBeUndefined()

// ============================================================================
// Custom Matchers (user extensible)
// ============================================================================
#define CEST_MATCHER(name, body) \
    static inline int _cest_match_##name(cest_value_t actual, cest_value_t expected, int* diff_pos) { \
        (void)expected; (void)diff_pos; \
        body \
    } \
    static inline void _cest_assert_##name(const char* expr, cest_value_t actual, const char* file, int line) { \
        _cest_ctx.file = file; _cest_ctx.line = line; \
        _cest_ctx.actual = actual; \
        _cest_ctx.actual_expr = expr; \
        _cest_assert_impl(cest_bool(true), _cest_match_##name, #name, ""); \
    }

// For matchers with arguments
#define CEST_MATCHER_WITH_ARGS(name, arg_decl, body) \
    static inline int _cest_match_##name(cest_value_t actual, cest_value_t expected, int* diff_pos) { \
        (void)diff_pos; \
        arg_decl; \
        body \
    } \
    static inline void _cest_assert_##name(const char* expr, cest_value_t actual, arg_decl, const char* file, int line) { \
        _cest_ctx.file = file; _cest_ctx.line = line; \
        _cest_ctx.actual = actual; \
        _cest_ctx.actual_expr = expr; \
        cest_value_t exp = cest_value(expected); /* assuming 'expected' is a variable name */ \
        _cest_assert_impl(exp, _cest_match_##name, #name, ""); \
    } \
    /* Usage: expect_that(value, name, args) */

// ============================================================================
// Test Runner Macros with Timing
// ============================================================================
static clock_t _cest_test_start_time __attribute__((unused)) = 0;

#ifdef CEST_ENABLE_FORK
#  define CEST_FORK_TEST(block) \
    do { \
        char _cest_sanitizer_output[4096] = {0}; \
        int _cest_fork_result_status = _cest_run_forked_test((cest_test_fn)(block), _cest_sanitizer_output, sizeof(_cest_sanitizer_output)); \
        if (_cest_fork_result_status < 0) { \
            _cest_global_stats.failed++; \
            printf("\n  " CEST_CLR_RED "✕ %s (Crashed/Sanitizer Detected, Signal %d)" CEST_CLR_RESET "\n", _cest_current_test_name, -_cest_fork_result_status); \
            if (_cest_sanitizer_output[0] != '\0') { \
                printf("    " CEST_CLR_DIM "Sanitizer Output:\n%s" CEST_CLR_RESET "\n", _cest_sanitizer_output); \
            } else { \
                printf("    " CEST_CLR_DIM "(No sanitizer output captured)" CEST_CLR_RESET "\n"); \
            } \
        } else if (_cest_fork_result_status > 0) { \
            _cest_global_stats.failed++; \
            printf("\n  " CEST_CLR_RED "✕ %s (Exited with code %d)" CEST_CLR_RESET "\n", _cest_current_test_name, _cest_fork_result_status); \
            if (_cest_sanitizer_output[0] != '\0') { \
                printf("    " CEST_CLR_DIM "Stderr Output:\n%s" CEST_CLR_RESET "\n", _cest_sanitizer_output); \
            } \
        } \
    } while(0)
#else
#  define CEST_FORK_TEST(block) do { block; } while(0)
#endif

#define describe(name, block) do { \
    printf("\n" CEST_CLR_BOLD "● %s" CEST_CLR_RESET "\n", name); \
    fflush(stdout); \
    _CEST_RUN_BEFORE_ALL(); \
    block \
    _CEST_RUN_AFTER_ALL(); \
} while (0)

#define test(name, block) do { \
    if (!_cest_should_run_test(name)) { \
        printf("  " CEST_CLR_DIM "○ %s (filtered)" CEST_CLR_RESET "\n", name); \
        _cest_global_stats.skipped++; \
    } else { \
        _cest_current_test_name = name; \
        _CEST_SIGNAL_SET_TEST(name); \
        _cest_test_start_time = clock(); \
        printf("  %s\n", name); \
        fflush(stdout); \
        _CEST_RUN_BEFORE_EACH(); \
        CEST_FORK_TEST(block); \
        _CEST_RUN_AFTER_EACH(); \
        fflush(stdout); \
    } \
} while (0)

#define bench(name, block) do { \
    clock_t _bench_start = clock(); \
    for(int _i = 0; _i < 1000; _i++) { block } \
    clock_t _bench_end = clock(); \
    double _bench_time = (double)(_bench_end - _bench_start) / CLOCKS_PER_SEC; \
    printf("  " CEST_CLR_BLUE "⚡ %s: %.6fs total, %.6fs avg" CEST_CLR_RESET "\n", name, _bench_time, _bench_time / 1000); \
} while (0)

#define it(name, block) test(name, block)

// ============================================================================
// Hooks
// ============================================================================
#ifndef CEST_NO_HOOKS
typedef void (*cest_hook_fn)(void);
static cest_hook_fn _cest_before_each_fn __attribute__((unused)) = NULL;
static cest_hook_fn _cest_after_each_fn __attribute__((unused)) = NULL;
static cest_hook_fn _cest_before_all_fn __attribute__((unused)) = NULL;
static cest_hook_fn _cest_after_all_fn __attribute__((unused)) = NULL;

#define beforeEach(fn) do { _cest_before_each_fn = fn; } while (0)
#define afterEach(fn) do { _cest_after_each_fn = fn; } while (0)
#define beforeAll(fn) do { _cest_before_all_fn = fn; } while (0)
#define afterAll(fn) do { _cest_after_all_fn = fn; } while (0)

#define _CEST_RUN_BEFORE_EACH() do { \
    _CEST_SIGNAL_SET_HOOK(CEST_HOOK_BEFORE_EACH); \
    if (_cest_before_each_fn) _cest_before_each_fn(); \
    _CEST_SIGNAL_CLEAR_HOOK(); \
    _CEST_COVERAGE_BEFORE_EACH(); \
} while(0)
#define _CEST_RUN_AFTER_EACH() do { \
    _CEST_SIGNAL_SET_HOOK(CEST_HOOK_AFTER_EACH); \
    if (_cest_after_each_fn) _cest_after_each_fn(); \
    _CEST_SIGNAL_CLEAR_HOOK(); \
    _CEST_COVERAGE_AFTER_EACH(); \
} while(0)
#define _CEST_RUN_BEFORE_ALL() do { \
    _CEST_SIGNAL_SET_HOOK(CEST_HOOK_BEFORE_ALL); \
    if (_cest_before_all_fn) _cest_before_all_fn(); \
    _CEST_SIGNAL_CLEAR_HOOK(); \
} while(0)
#define _CEST_RUN_AFTER_ALL() do { \
    _CEST_SIGNAL_SET_HOOK(CEST_HOOK_AFTER_ALL); \
    if (_cest_after_all_fn) _cest_after_all_fn(); \
    _CEST_SIGNAL_CLEAR_HOOK(); \
} while(0)
#else
#define beforeEach(fn)
#define afterEach(fn)
#define beforeAll(fn)
#define afterAll(fn)
#define _CEST_RUN_BEFORE_EACH()
#define _CEST_RUN_AFTER_EACH()
#define _CEST_RUN_BEFORE_ALL()
#define _CEST_RUN_AFTER_ALL()
#endif

// ============================================================================
// Skip/Only tests
// ============================================================================
#ifdef CEST_ENABLE_SKIP
#define _CEST_SKIP_IF_SKIPPED() do { \
    if (_cest_current_test_state == CEST_TEST_SKIP) { \
        printf("  " CEST_CLR_YELLOW "⊘ %s (skipped)" CEST_CLR_RESET "\n", _cest_ctx.actual_expr); \
        _cest_global_stats.skipped++; \
        return; \
    } \
} while (0)

#define _CEST_SKIP_IF_NOT_ONLY() do { \
    int has_only = (_cest_global_stats.filter_pattern != NULL); \
    if (has_only && _cest_current_test_state != CEST_TEST_ONLY) { \
        printf("  " CEST_CLR_DIM "○ %s (only mode - skipped)" CEST_CLR_RESET "\n", _cest_ctx.actual_expr); \
        _cest_global_stats.skipped++; \
        return; \
    } \
} while (0)

#define skip(desc, block) describe(desc "_SKIPPED", { (void)block; })
#define only(desc, block) describe(desc "_ONLY", { (void)block; })
#else
#define _CEST_SKIP_IF_SKIPPED()
#define _CEST_SKIP_IF_NOT_ONLY()
#define skip(desc, block)
#define only(desc, block)
#endif

// ============================================================================
// CLI Arguments and Reports
// ============================================================================
#ifndef CEST_NO_CLI
static const char* _cest_cli_args[32];
static int _cest_cli_count = 0;

static inline void _cest_write_junit(void) {
    if (!_cest_junit_output) return;
    FILE* f = fopen(_cest_junit_output, "w");
    if (!f) return;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<testsuites tests=\"%d\" failures=\"%d\" skipped=\"%d\" time=\"%.3f\">\n",
            _cest_global_stats.passed + _cest_global_stats.failed,
            _cest_global_stats.failed, _cest_global_stats.skipped, _cest_total_time);
    fprintf(f, "  <testsuite name=\"Cest\" tests=\"%d\" failures=\"%d\" skipped=\"%d\" time=\"%.3f\">\n",
            _cest_global_stats.passed + _cest_global_stats.failed,
            _cest_global_stats.failed, _cest_global_stats.skipped, _cest_total_time);
    // Note: For full test details, you would need to store each test result.
    // This is a simplified version.
    fprintf(f, "  </testsuite>\n");
    fprintf(f, "</testsuites>\n");
    fclose(f);
}

static inline void _cest_write_json(void) {
    if (!_cest_json_output) return;
    FILE* f = fopen(_cest_json_output, "w");
    if (!f) return;
    fprintf(f, "{\n");
    fprintf(f, "  \"stats\": {\n");
    fprintf(f, "    \"passed\": %d,\n", _cest_global_stats.passed);
    fprintf(f, "    \"failed\": %d,\n", _cest_global_stats.failed);
    fprintf(f, "    \"skipped\": %d,\n", _cest_global_stats.skipped);
    fprintf(f, "    \"total_time\": %.3f\n", _cest_total_time);
    fprintf(f, "  },\n");
    fprintf(f, "  \"tests\": []\n");
    fprintf(f, "}\n");
    fclose(f);
}

static inline void _cest_parse_cli_args(int argc, char* argv[]) {
    _CEST_INIT_COLORS();
    
    // Note: CI color control is compile-time only.
    // Use -DCEST_NO_COLORS when building for CI environments.
    (void)_cest_is_ci();
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "--sanitize") == 0 && i + 1 < argc) {
                const char* sanitize_list = argv[++i];
                char* list_copy = strdup(sanitize_list);
                if (list_copy) {
                    char* token = strtok(list_copy, ",");
                    while (token) {
                        if (strcmp(token, "address") == 0) _cest_sanitize_flags |= 1;
                        else if (strcmp(token, "thread") == 0) _cest_sanitize_flags |= 2;
                        else if (strcmp(token, "memory") == 0) _cest_sanitize_flags |= 4;
                        else if (strcmp(token, "undefined") == 0) _cest_sanitize_flags |= 8;
                        token = strtok(NULL, ",");
                    }
                    free(list_copy);
                }
            } else if (strcmp(argv[i], "--sanitize-errors-as-failures") == 0) {
                _cest_sanitize_errors_as_failures = true;
            } else if (strcmp(argv[i], "--no-sanitize-errors-as-failures") == 0) {
                _cest_sanitize_errors_as_failures = false;
            } else if (strcmp(argv[i], "--junit") == 0 && i + 1 < argc) {
                _cest_junit_output = argv[++i];
            } else if (strcmp(argv[i], "--json") == 0 && i + 1 < argc) {
                _cest_json_output = argv[++i];
            } else {
                if (_cest_cli_count < (int)(sizeof(_cest_cli_args) / sizeof(_cest_cli_args[0]))) {
                    _cest_cli_args[_cest_cli_count++] = argv[i];
                } else {
                    fprintf(stderr, "Cest: warning: too many CLI arguments (max %d), ignoring '%s'\n",
                            (int)(sizeof(_cest_cli_args) / sizeof(_cest_cli_args[0])), argv[i]);
                }
            }
        } else {
            _cest_global_stats.filter_pattern = argv[i];
        }
    }
}

static inline int _cest_has_cli_flag(const char* flag) {
    for (int i = 0; i < _cest_cli_count; i++) {
        if (strcmp(_cest_cli_args[i], flag) == 0) return 1;
    }
    return 0;
}

static inline int _cest_should_run_test(const char* name) {
    if (!_cest_global_stats.filter_pattern) return 1;
    return strstr(name, _cest_global_stats.filter_pattern) != NULL;
}

#define cest_init(argc, argv) do { _cest_parse_cli_args(argc, argv); _CEST_SIGNAL_INIT(); } while(0)
#else
#define cest_init(argc, argv) do { _CEST_SIGNAL_INIT(); } while(0)
#define _cest_has_cli_flag(flag) (0)
#define _cest_should_run_test(name) (1)
#endif

// ============================================================================
// Enhanced Results
// ============================================================================
static inline int cest_result() {
    _cest_total_time = (double)(clock() - _cest_suite_start_time) / CLOCKS_PER_SEC;
    
    printf("\n" CEST_CLR_BOLD "Test Suites Summary:" CEST_CLR_RESET "\n");
    if (_cest_global_stats.failed == 0) {
        printf("  " CEST_CLR_GREEN "All %d tests passed!" CEST_CLR_RESET, _cest_global_stats.passed);
        if (_cest_global_stats.skipped > 0) printf(" (" CEST_CLR_YELLOW "%d skipped" CEST_CLR_RESET ")", _cest_global_stats.skipped);
        printf("\n");
    } else {
        printf("  " CEST_CLR_GREEN "Passed: %d" CEST_CLR_RESET, _cest_global_stats.passed);
        printf("\n  " CEST_CLR_RED "Failed: %d" CEST_CLR_RESET, _cest_global_stats.failed);
        if (_cest_global_stats.skipped > 0) printf("\n  " CEST_CLR_YELLOW "Skipped: %d" CEST_CLR_RESET, _cest_global_stats.skipped);
        printf("\n");
    }
    
#ifdef CEST_ENABLE_LEAK_DETECTION
    printf("\n" CEST_CLR_BOLD "Memory:" CEST_CLR_RESET "\n");
    if (_cest_alloc_count == _cest_free_count) {
        printf("  " CEST_CLR_GREEN "All %lld allocations freed!" CEST_CLR_RESET "\n", _cest_alloc_count);
    } else {
        long long leaked = _cest_alloc_count - _cest_free_count;
        printf("  " CEST_CLR_RED "Memory leak: %lld allocations not freed" CEST_CLR_RESET "\n", leaked);
    }
#endif
    
    // Write reports
    _cest_write_junit();
    _cest_write_json();
    
    // Post-run command hook
    const char* post_run = getenv("CEST_POST_RUN");
    if (post_run != NULL) {
        printf("\n" CEST_CLR_BOLD "Running post-test command:" CEST_CLR_RESET " %s\n", post_run);
        int ret = system(post_run);
        if (ret == -1) {
            printf(CEST_CLR_RED "Failed to execute post-run command." CEST_CLR_RESET "\n");
        } else {
            printf(CEST_CLR_GREEN "Post-run command executed with exit code %d." CEST_CLR_RESET "\n", WEXITSTATUS(ret));
        }
    }
    
    printf("\n");
    return _cest_global_stats.failed > 0 ? 1 : 0;
}



// ============================================================================
// Optional Namespaced Macros (define CEST_PREFIX before including cest.h)
// ============================================================================
#ifdef CEST_PREFIX
#  define cest_describe(name, block)    describe(name, block)
#  define cest_test(name, block)        test(name, block)
#  define cest_it(name, block)          it(name, block)
#  define cest_expect(x)                expect(x)
#  define cest_expect_array(x, len)     expect_array(x, len)
#  define cest_bench(name, block)       bench(name, block)
#  define cest_beforeEach(fn)           beforeEach(fn)
#  define cest_afterEach(fn)           afterEach(fn)
#  define cest_beforeAll(fn)           beforeAll(fn)
#  define cest_afterAll(fn)            afterAll(fn)
#endif

// ============================================================================
// File-based Test Grouping (describe with automatic filename)
// ============================================================================
#define describe_file(block) \
    describe(_cest_basename(__FILE__), block)

static inline const char* _cest_basename(const char* path) {
    const char* sep = path ? strrchr(path, '/') : NULL;
    return sep ? sep + 1 : (path ? path : "unknown");
}

// ============================================================================
// End Warning Suppression
// ============================================================================
#if CEST_SUPPRESS_WARNINGS
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif
#  ifdef __clang__
#    pragma clang diagnostic pop
#  endif
#  ifdef _MSC_VER
#    pragma warning(pop)
#  endif
#endif

#endif // _CEST_H_
