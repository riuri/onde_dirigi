/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* Simple definitions for the Test Anything Protocol
 * https://testanything.org
 */

#define ok() printf("ok %d - %s\n", (int)test_case, __func__)

#define fail(s, ...) do { \
  printf("not ok %d - %s (%s:%d) " s "\n", (int)test_case, __func__, \
    __FILE__, __LINE__, __VA_ARGS__); \
  return; \
} while(0)

#define skip(reason) printf("ok %d - Skipping %s: %s # SKIP\n", \
  (int)test_case, __func__, reason)

#define my_assert(expr) do { \
  if (!(expr)) fail("“%s” is not true", #expr); \
} while(0)
