/* =========================================================================
    Unity - A Test Framework for C
    ThrowTheSwitch.org
    Copyright (c) 2007-25 Mike Karlesky, Mark VanderVoord, & Greg Williams
    SPDX-License-Identifier: MIT
========================================================================= */

#ifndef UNITY_INTERNALS_H
#define UNITY_INTERNALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Basic Unity configuration for our OS testing
#define UNITY_INT_WIDTH 32
#define UNITY_LONG_WIDTH 32
#define UNITY_POINTER_WIDTH 32

// Unity types
typedef unsigned int UNITY_UINT;
typedef unsigned long UNITY_ULONG;
typedef int UNITY_INT;
typedef long UNITY_LONG;
typedef unsigned char UNITY_UINT8;
typedef unsigned short UNITY_UINT16;
typedef unsigned int UNITY_UINT32;
typedef signed char UNITY_INT8;
typedef signed short UNITY_INT16;
typedef signed int UNITY_INT32;

// Unity constants
#define UNITY_MAX_TEST_LINE_LENGTH 256
#define UNITY_MAX_DETAILS_LENGTH 256

// Unity internals structure
struct UNITY_STORAGE_T
{
    const char* TestFile;
    unsigned long TestLine;
    unsigned long CurrentTestFailed;
    unsigned long TestFailures;
    unsigned long TestIgnores;
    jmp_buf AbortFrame;
};

extern struct UNITY_STORAGE_T Unity;

// Unity function declarations
void UnityBegin(const char* filename);
int UnityEnd(void);
void UnityConcludeTest(void);
void UnityDefaultTestRun(void (*func)(void), const char* name, int line);

// Assertion functions
void UnityAssertEqualNumber(const UNITY_INT expected,
                           const UNITY_INT actual,
                           const char* msg,
                           const UNITY_LINE_TYPE line,
                           const UNITY_DISPLAY_STYLE_T style);

void UnityAssertEqualString(const char* expected,
                           const char* actual,
                           const char* msg,
                           const UNITY_LINE_TYPE line);

void UnityAssertEqualMemory(const void* expected,
                           const void* actual,
                           const UNITY_UINT32 length,
                           const UNITY_UINT32 depth,
                           const char* msg,
                           const UNITY_LINE_TYPE line);

void UnityAssertNumbersArrayWithin(const UNITY_UINT delta,
                                  const void* expected,
                                  const void* actual,
                                  const UNITY_UINT32 num_elements,
                                  const char* msg,
                                  const UNITY_LINE_TYPE line,
                                  const UNITY_DISPLAY_STYLE_T style,
                                  const UNITY_FLAGS_T flags);

void UnityFail(const char* message, const UNITY_LINE_TYPE line);
void UnityIgnore(const char* message, const UNITY_LINE_TYPE line);

// Helper macros and types
#define UNITY_LINE_TYPE unsigned int
#define UNITY_DISPLAY_STYLE_T unsigned char
#define UNITY_FLAGS_T unsigned char

#define UNITY_DISPLAY_STYLE_INT    0
#define UNITY_DISPLAY_STYLE_UINT   1
#define UNITY_DISPLAY_STYLE_HEX8   2
#define UNITY_DISPLAY_STYLE_HEX16  3
#define UNITY_DISPLAY_STYLE_HEX32  4
#define UNITY_DISPLAY_STYLE_CHAR   5

#define UNITY_ARRAY_TO_VAL 0
#define UNITY_ARRAY_TO_ARRAY 1

// Memory assertion helper
#define UNITY_TEST_ASSERT_EQUAL_MEMORY(expected, actual, length, line, message) \
    UnityAssertEqualMemory((expected), (actual), (length), 1, (message), (line))

// String assertion helper
#define UNITY_TEST_ASSERT_EQUAL_STRING(expected, actual, line, message) \
    UnityAssertEqualString((expected), (actual), (message), (line))

// Number assertion helpers
#define UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, line, message) \
    UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (line), UNITY_DISPLAY_STYLE_INT)

#define UNITY_TEST_ASSERT_EQUAL_UINT(expected, actual, line, message) \
    UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (line), UNITY_DISPLAY_STYLE_UINT)

#define UNITY_TEST_ASSERT_EQUAL_HEX8(expected, actual, line, message) \
    UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (line), UNITY_DISPLAY_STYLE_HEX8)

#define UNITY_TEST_ASSERT_EQUAL_HEX16(expected, actual, line, message) \
    UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (line), UNITY_DISPLAY_STYLE_HEX16)

#define UNITY_TEST_ASSERT_EQUAL_HEX32(expected, actual, line, message) \
    UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (line), UNITY_DISPLAY_STYLE_HEX32)

// Null assertion helpers
#define UNITY_TEST_ASSERT_NULL(pointer, line, message) \
    UnityAssertEqualNumber((UNITY_INT)NULL, (UNITY_INT)(pointer), (message), (line), UNITY_DISPLAY_STYLE_UINT)

#define UNITY_TEST_ASSERT_NOT_NULL(pointer, line, message) \
    UnityAssertEqualNumber((UNITY_INT)NULL, (UNITY_INT)(pointer), (message), (line), UNITY_DISPLAY_STYLE_UINT)

// Truth assertion helpers
#define UNITY_TEST_ASSERT(condition, line, message) \
    if (!(condition)) { UnityFail((message), (line)); }

#define UNITY_TEST_ASSERT_TRUE(condition, line, message) \
    UNITY_TEST_ASSERT((condition), (line), (message))

#define UNITY_TEST_ASSERT_FALSE(condition, line, message) \
    UNITY_TEST_ASSERT(!(condition), (line), (message))

// Fail and ignore helpers
#define UNITY_TEST_FAIL(line, message) \
    UnityFail((message), (line))

#define UNITY_TEST_IGNORE(line, message) \
    UnityIgnore((message), (line))

// Test function macros
#define RUN_TEST(func) \
    UnityDefaultTestRun(func, #func, __LINE__)

#define TEST_CASE(...)
#define TEST_RANGE(...)
#define TEST_MATRIX(...)

#endif /* UNITY_INTERNALS_H */