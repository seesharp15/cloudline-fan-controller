// Minimal test helpers: CHECK keeps going after a failure, the summary sets
// the exit code.
#pragma once

#include <stdio.h>

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                       \
  do {                                                                    \
    g_checks++;                                                           \
    if (!(cond)) {                                                        \
      g_failures++;                                                       \
      fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
    }                                                                     \
  } while (0)

#define CHECK_EQ(a, b)                                                    \
  do {                                                                    \
    g_checks++;                                                           \
    const long long va_ = (long long)(a), vb_ = (long long)(b);           \
    if (va_ != vb_) {                                                     \
      g_failures++;                                                       \
      fprintf(stderr, "  FAIL %s:%d: %s == %s (%lld vs %lld)\n", __FILE__, \
              __LINE__, #a, #b, va_, vb_);                                \
    }                                                                     \
  } while (0)

#define CHECK_NEAR(a, b, tol)                                             \
  do {                                                                    \
    g_checks++;                                                           \
    const double va_ = (double)(a), vb_ = (double)(b);                    \
    if (va_ - vb_ > (tol) || vb_ - va_ > (tol)) {                         \
      g_failures++;                                                       \
      fprintf(stderr, "  FAIL %s:%d: %s ~= %s (%g vs %g, tol %g)\n",      \
              __FILE__, __LINE__, #a, #b, va_, vb_, (double)(tol));       \
    }                                                                     \
  } while (0)

#define TEST(name) static void name()
#define RUN(name)                      \
  do {                                 \
    printf("- %s\n", #name);           \
    name();                            \
  } while (0)

static int summary(const char *suite) {
  printf("%s: %d checks, %d failures\n", suite, g_checks, g_failures);
  return g_failures ? 1 : 0;
}
