#include <stdio.h>
#include <stdlib.h>

#include "student.h"
#include "reference.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// Unit test functions.

int test_smallest();
int test_mean();
int test_swap();
int test_squaredPrimes();
int test_sort();
int test_negate_fibonacci();

// Utility functions used by the unit tests.
bool arrays_differ(int arrray1[], int array2[], int length);
char *array_to_string(int array[], int length);

int main() {
  int score = 0;
  
  score += test_smallest();
  score += test_mean();
  score += test_swap();
  score += test_squaredPrimes();
  score += test_sort();
  score += test_negate_fibonacci();

  printf("Total score: %d/%d\n", score, 10);
}

int test_smallest() {
  {
    printf("running %s: single-element-array: ", __func__);

    int in[] = {-1234};
    int out = smallest(in, ARRAY_SIZE(in));
    int expected = ref_smallest(in, ARRAY_SIZE(in));

    if (out != expected) {
      printf("failed:\n  got: %d\n  expected: %d\n", out, expected);
      return 0;
    }
    printf("passed\n");
  }
  {
    printf("running %s: regular-array: ", __func__);

    int in[] = {123, 135, -1234, 0, 55};
    int out = smallest(in, ARRAY_SIZE(in));
    int expected = ref_smallest(in, ARRAY_SIZE(in));

    if (out != expected) {
      printf("failed:\n  got: %d\n  expected: %d\n", out, expected);
      return 0;
    }
    printf("passed\n");
  }
  return 1;
}

int test_mean() {
  {
    printf("running %s: empty-array: ", __func__);

    int in[] = {};
    double out = mean(in, ARRAY_SIZE(in));
    double expected = ref_mean(in, ARRAY_SIZE(in));

    if (out != expected) {
      printf("failed:\n  got: %f\n  expected: %f\n", out, expected);
      return 0;
    }
    printf("passed\n");
  }
  {
    printf("running %s: regular-array: ", __func__);

    int in[] = {-5, 1, 3, 58, 29823};
    double out = mean(in, ARRAY_SIZE(in));
    double expected = ref_mean(in, ARRAY_SIZE(in));

    if (out != expected) {
      printf("failed:\n  got: %f\n  expected: %f\n", out, expected);
      return 0;
    }
    printf("passed\n");
  }
  return 1;
}

int test_swap() {
  printf("running %s: ", __func__);

  int a = 1, b = 2;
  swap(&a, &b);
  if (a != 2 || b != 1) {
    printf("failed:\n  got: a=%d b=%d\n  expected: a=2 b=1\n", a, b);
    return 0;
  }
  printf("passed\n");
  return 1;
}

int test_sort() {
  {
    printf("running %s: empty-array: ", __func__);
    int out[]      = {};
    int expected[] = {};

    sort(out, ARRAY_SIZE(out));
    ref_sort(expected, ARRAY_SIZE(expected));

    if (arrays_differ(out, expected, ARRAY_SIZE(out))) {
      char *out_s = array_to_string(out, ARRAY_SIZE(out));
      char *expected_s = array_to_string(expected, ARRAY_SIZE(expected));

      printf("failed:\n  got: %s\n  expected: %s\n", out_s, expected_s);

      free(out_s);
      free(expected_s);
      return 0;
    }
    printf("passed\n");
  }
  {
    printf("running %s: regular-array: ", __func__);
    int out[]      = {8, 2, 2, 3234, 234, 234, 1, 5, -15, 32, 87234};
    int expected[] = {8, 2, 2, 3234, 234, 234, 1, 5, -15, 32, 87234};

    sort(out, ARRAY_SIZE(out));
    ref_sort(expected, ARRAY_SIZE(expected));

    if (arrays_differ(out, expected, ARRAY_SIZE(out))) {
      char *out_s = array_to_string(out, ARRAY_SIZE(out));
      char *expected_s = array_to_string(expected, ARRAY_SIZE(expected));

      printf("failed:\n  got: %s\n  expected: %s\n", out_s, expected_s);

      free(out_s);
      free(expected_s);
      return 0;
    }
    printf("passed\n");
  }
  {
    printf("running %s: sorted-array: ", __func__);
    int out[]      = {-5, -1, 2, 3, 7, 198, 28349};
    int expected[] = {-5, -1, 2, 3, 7, 198, 28349};

    sort(out, ARRAY_SIZE(out));
    ref_sort(expected, ARRAY_SIZE(expected));

    if (arrays_differ(out, expected, ARRAY_SIZE(out))) {
      char *out_s = array_to_string(out, ARRAY_SIZE(out));
      char *expected_s = array_to_string(expected, ARRAY_SIZE(expected));

      printf("failed:\n  got: %s\n  expected: %s\n", out_s, expected_s);

      free(out_s);
      free(expected_s);
      return 0;
    }
    printf("passed\n");
  }
  return 2;
}

int test_squaredPrimes() {
  printf("running %s: ", __func__);
  int out[]      = {0, 1, 2, -5, 11, 13, 15, 27, 3779, 32};
  int expected[] = {0, 1, 2, -5, 11, 13, 15, 27, 3779, 32};

  squaredPrimes(out, ARRAY_SIZE(out));
  ref_squaredPrimes(expected, ARRAY_SIZE(expected));

  if (arrays_differ(out, expected, ARRAY_SIZE(out))) {
    char *out_s = array_to_string(out, ARRAY_SIZE(out));
    char *expected_s = array_to_string(expected, ARRAY_SIZE(expected));

    printf("failed:\n  got: %s\n  expected: %s\n", out_s, expected_s);

    free(out_s);
    free(expected_s);
    return 0;
  }

  printf("passed\n");
  return 2;
}

int test_negate_fibonacci() {
  printf("running %s: ", __func__);
  int out[]      = {0, 1, 4, 7, 11, 13, 21, 34, 58, 61, 100};
  int expected[] = {0, 1, 4, 7, 11, 13, 21, 34, 58, 61, 100};

  negate_fibonacci(out, ARRAY_SIZE(out));
  ref_negate_fibonacci(expected, ARRAY_SIZE(expected));

  if (arrays_differ(out, expected, ARRAY_SIZE(out))) {
    char *out_s = array_to_string(out, ARRAY_SIZE(out));
    char *expected_s = array_to_string(expected, ARRAY_SIZE(expected));

    printf("failed:\n  got: %s\n  expected: %s\n", out_s, expected_s);

    free(out_s);
    free(expected_s);
    return 0;
  }

  printf("passed\n");
  return 3;
}

bool arrays_differ(int array1[], int array2[], int length) {
  for (int i = 0; i < length; ++i)
    if (array1[i] != array2[i])
      return true;
  return false;
}

char *array_to_string(int array[], int length) {
  char *p = (char *)malloc(length * 10);
  for (int i = 0, n = 0; i < length; ++i) {
    n += sprintf(p + n, "%d ", array[i]);
  }
  return p;
}
