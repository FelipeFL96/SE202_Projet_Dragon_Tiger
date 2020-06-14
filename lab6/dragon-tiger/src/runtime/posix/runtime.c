#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

__attribute__((noreturn))
static void error(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void __print_err(const char *s) {
  fprintf(stderr, "%s\n", s);
}

void __print(const char *s) {
  fprintf(stdout, "%s\n", s);
}

void __print_int(const int32_t i) {
  fprintf(stdout, "%d\n", i);
}

void __flush(void) {
  fflush(stdout);
}

const char *__getchar(void) {
  char* str = (char*) malloc(2*sizeof(char));
  str[0] = (char) getchar();
  str[1] = '\0';
  return str;
}

int32_t __ord(const char *s) {
  if (s[0] == '\0')
    return -1;
  return (int) s[0];
}

const char *__chr(int32_t i) {
  if (i < 0 || i > 255)
    error("chr: character out of range");
  char c[2] = {(char) i, '\0'};
  char* s = (char*) malloc(2*sizeof(char));
  strcpy(s, c);
  return s;
}

int32_t __size(const char *s) {
  int i;
  for (i = 0; s[i] != '\0'; i++);
  return i;
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  if (length < 0)
    error("substring: length must be positive");
  if (strlen(s) - first < length)
    error("substring: requested substring out of bounds");
  char* str = (char*) malloc(length*sizeof(char));
  strncpy(str, &s[first], length);
  return str;
}

const char *__concat(const char *s1, const char *s2) {
  char* s = (char*) malloc((strlen(s1)+strlen(s2))*sizeof(char));
  strcat(s, s1);
  strcat(s, s2);
  return s;
}

int32_t __strcmp(const char *s1, const char *s2) {
  return strcmp(s1, s2);
}

int32_t __streq(const char *s1, const char *s2) {
  return 0 == strcmp(s1, s2);
}

int32_t __not(int32_t i) {
  if (i == 1) {
    return 0;
  }
  else {
    return 1;
  }
}

void __exit(int32_t c) {
  exit(c);
}