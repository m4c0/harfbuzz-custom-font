#ifdef __APPLE__
#  include <sys/stat.h>
#  include <unistd.h>
#elif _WIN32
#  define _CRT_SECURE_NO_WARNINGS
#  define _CRT_NONSTDC_NO_WARNINGS
#  include <direct.h>
#  include <process.h>
#else
#  error Unsupported platform
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exists(const char * name) {
  FILE * f = fopen(name, "rb");
  if (f) fclose(f);
  return !!f;
}

static int run(char ** args) {
  assert(args && args[0]);

#ifdef __APPLE__
  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    abort();
  } else if (pid > 0) {
    int sl = 0;
    assert(0 <= waitpid(pid, &sl, 0));
    if (WIFEXITED(sl)) return WEXITSTATUS(sl);
  }
#elif _WIN32
  if (0 == _spawnvp(_P_WAIT, args[0], (const char * const *)args)) {
    return 0;
  }
#endif

  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}
#define RUN(...) do { char * args[] = { __VA_ARGS__, 0 }; if (run(args)) return 1; } while (0)

#ifdef __APPLE__
#  define DEBUG "-g"
#  define MKDIR(x) mkdir(x, 0777)
#elif _WIN32
#  define DEBUG "-gdwarf"
#  define MKDIR(x) mkdir(x)
#endif

#define HB(base) if (!exists("out/"base".o")) RUN("clang++", DEBUG, "-c", "-o", "out/"base".o", "harfbuzz/src/"base".cc");

#define HB_SRCS \
  HB_SRC("hb-aat-layout") \
  HB_SRC("hb-aat-map") \
  HB_SRC("hb-blob") \
  HB_SRC("hb-buffer") \
  HB_SRC("hb-buffer-serialize") \
  HB_SRC("hb-buffer-verify") \
  HB_SRC("hb-common") \
  HB_SRC("hb-draw") \
  HB_SRC("hb-face") \
  HB_SRC("hb-face-builder") \
  HB_SRC("hb-fallback-shape") \
  HB_SRC("hb-font") \
  HB_SRC("hb-number") \
  HB_SRC("hb-outline") \
  HB_SRC("hb-ot-cff1-table") \
  HB_SRC("hb-ot-cff2-table") \
  HB_SRC("hb-ot-face") \
  HB_SRC("hb-ot-font") \
  HB_SRC("hb-ot-layout") \
  HB_SRC("hb-ot-map") \
  HB_SRC("hb-ot-metrics") \
  HB_SRC("hb-ot-shape") \
  HB_SRC("hb-ot-shape-fallback") \
  HB_SRC("hb-ot-shape-normalize") \
  HB_SRC("hb-ot-shaper-arabic") \
  HB_SRC("hb-ot-shaper-default") \
  HB_SRC("hb-ot-shaper-hangul") \
  HB_SRC("hb-ot-shaper-hebrew") \
  HB_SRC("hb-ot-shaper-indic") \
  HB_SRC("hb-ot-shaper-indic-table") \
  HB_SRC("hb-ot-shaper-khmer") \
  HB_SRC("hb-ot-shaper-myanmar") \
  HB_SRC("hb-ot-shaper-syllabic") \
  HB_SRC("hb-ot-shaper-thai") \
  HB_SRC("hb-ot-shaper-use") \
  HB_SRC("hb-ot-shaper-vowel-constraints") \
  HB_SRC("hb-ot-tag") \
  HB_SRC("hb-ot-var") \
  HB_SRC("hb-paint-bounded") \
  HB_SRC("hb-paint-extents") \
  HB_SRC("hb-paint") \
  HB_SRC("hb-set") \
  HB_SRC("hb-shape") \
  HB_SRC("hb-shape-plan") \
  HB_SRC("hb-shaper") \
  HB_SRC("hb-static") \
  HB_SRC("hb-ucd") \
  HB_SRC("hb-unicode") \

int main() {
  MKDIR("out");

#define HB_SRC(x) HB(x);
  HB_SRCS;
#undef HB_SRC
#define HB_SRC(x) "out/"x".o",

  RUN("clang", "-Wall", DEBUG, "-Iharfbuzz/src", "-c", "-o", "out/hello.o", "hello.c");
  RUN("clang++", DEBUG, "-o", "out/hello.exe",
      HB_SRCS
      "out/hello.o");
  return 0;
}

