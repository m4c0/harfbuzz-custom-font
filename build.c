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

int main() {
  MKDIR("out");

  HB("hb-blob");
  HB("hb-buffer");
  HB("hb-common");
  HB("hb-face");
  HB("hb-fallback-shape");
  HB("hb-number");
  HB("hb-ot-shape");
  HB("hb-set");
  HB("hb-shape");
  HB("hb-shape-plan");
  HB("hb-shaper");
  HB("hb-static");
  HB("hb-ucd");
  HB("hb-unicode");

  RUN("clang", "-Wall", DEBUG, "-Iharfbuzz/src", "-c", "-o", "out/hello.o", "hello.c");
  RUN("clang++", DEBUG, "-o", "out/hello.exe",
      "out/hb-blob.o",
      "out/hb-buffer.o",
      "out/hb-common.o",
      "out/hb-face.o",
      "out/hb-fallback-shape.o",
      "out/hb-number.o",
      "out/hb-ot-shape.o",
      "out/hb-set.o",
      "out/hb-shape.o",
      "out/hb-shape-plan.o",
      "out/hb-shaper.o",
      "out/hb-static.o",
      "out/hb-ucd.o",
      "out/hb-unicode.o",
      "out/hello.o");
  return 0;
}

