/*
 * Copyright 2014-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>

#include "e_os.h"
#include "internal/constant_time_locl.h"
#include "testutil.h"
#include "test_main.h"
#include "internal/numbers.h"

static const unsigned int CONSTTIME_TRUE = (unsigned)(~0);
static const unsigned int CONSTTIME_FALSE = 0;
static const unsigned char CONSTTIME_TRUE_8 = 0xff;
static const unsigned char CONSTTIME_FALSE_8 = 0;
static const size_t CONSTTIME_TRUE_S = ~((size_t)0);
static const size_t CONSTTIME_FALSE_S = 0;

static int test_binary_op(unsigned int (*op) (unsigned int a, unsigned int b),
                          const char *op_name, unsigned int a, unsigned int b,
                          int is_true)
{
    if (is_true && !TEST_uint_eq(op(a, b), CONSTTIME_TRUE))
        return 0;
    if (!is_true && !TEST_uint_eq(op(a, b), CONSTTIME_FALSE))
        return 0;
    return 1;
}

static int test_binary_op_8(unsigned
                            char (*op) (unsigned int a, unsigned int b),
                            const char *op_name, unsigned int a,
                            unsigned int b, int is_true)
{
    if (is_true && !TEST_uint_eq(op(a, b), CONSTTIME_TRUE_8))
        return 0;
    if (!is_true && !TEST_uint_eq(op(a, b), CONSTTIME_FALSE_8))
        return 0;
    return 1;
}

static int test_binary_op_s(size_t (*op) (size_t a, size_t b),
                            const char *op_name, size_t a, size_t b,
                            int is_true)
{
    if (is_true && !TEST_size_t_eq(op(a,b), CONSTTIME_TRUE_S))
        return 0;
    if (!is_true && !TEST_uint_eq(op(a,b), CONSTTIME_FALSE_S))
        return 0;
    return 1;
}

static int test_is_zero(unsigned int a)
{
    if (a == 0 && !TEST_uint_eq(constant_time_is_zero(a), CONSTTIME_TRUE))
        return 0;
    if (a != 0 && !TEST_uint_eq(constant_time_is_zero(a), CONSTTIME_FALSE))
        return 0;
    return 1;
}

static int test_is_zero_8(unsigned int a)
{
    if (a == 0 && !TEST_uint_eq(constant_time_is_zero_8(a), CONSTTIME_TRUE_8))
        return 0;
    if (a != 0 && !TEST_uint_eq(constant_time_is_zero_8(a), CONSTTIME_FALSE_8))
        return 0;
    return 1;
}

static int test_is_zero_s(unsigned int a)
{
    if (a == 0 && !TEST_size_t_eq(constant_time_is_zero_s(a), CONSTTIME_TRUE_S))
        return 0;
    if (a != 0 && !TEST_uint_eq(constant_time_is_zero_s(a), CONSTTIME_FALSE_S))
        return 0;
    return 1;
}

static int test_select(unsigned int a, unsigned int b)
{
    if (!TEST_uint_eq(constant_time_select(CONSTTIME_TRUE, a, b), a))
        return 0;
    if (!TEST_uint_eq(constant_time_select(CONSTTIME_FALSE, a, b), b))
        return 0;
    return 1;
}

static int test_select_8(unsigned char a, unsigned char b)
{
    if (!TEST_uint_eq(constant_time_select_8(CONSTTIME_TRUE_8, a, b), a))
        return 0;
    if (!TEST_uint_eq(constant_time_select_8(CONSTTIME_FALSE_8, a, b), b))
        return 0;
    return 1;
}

static int test_select_s(unsigned char a, unsigned char b)
{
    if (!TEST_uint_eq(constant_time_select_s(CONSTTIME_TRUE_S, a, b), a))
        return 0;
    if (!TEST_uint_eq(constant_time_select_s(CONSTTIME_FALSE_S, a, b), b))
        return 0;
    return 1;
}

static int test_select_int(int a, int b)
{
    if (!TEST_int_eq(constant_time_select_int(CONSTTIME_TRUE, a, b), a))
        return 0;
    if (!TEST_int_eq(constant_time_select_int(CONSTTIME_FALSE, a, b), b))
        return 0;
    return 1;
}

static int test_eq_int_8(int a, int b)
{
    if (a == b && !TEST_int_eq(constant_time_eq_int_8(a, b), CONSTTIME_TRUE_8))
        return 0;
    if (a != b && !TEST_int_eq(constant_time_eq_int_8(a, b), CONSTTIME_FALSE_8))
        return 0;
    return 1;
}

static int test_eq_s(size_t a, size_t b)
{
    if (a == b && !TEST_size_t_eq(constant_time_eq_s(a, b), CONSTTIME_TRUE_S))
        return 0;
    if (a != b && !TEST_int_eq(constant_time_eq_s(a, b), CONSTTIME_FALSE_S))
        return 0;
    return 1;
}

static int test_eq_int(int a, int b)
{
    if (a == b && !TEST_uint_eq(constant_time_eq_int(a, b), CONSTTIME_TRUE))
        return 0;
    if (a != b && !TEST_uint_eq(constant_time_eq_int(a, b), CONSTTIME_FALSE))
        return 0;
    return 1;
}

static unsigned int test_values[] =
    { 0, 1, 1024, 12345, 32000, UINT_MAX / 2 - 1,
    UINT_MAX / 2, UINT_MAX / 2 + 1, UINT_MAX - 1,
    UINT_MAX
};

static unsigned char test_values_8[] =
    { 0, 1, 2, 20, 32, 127, 128, 129, 255 };

static int signed_test_values[] = { 0, 1, -1, 1024, -1024, 12345, -12345,
    32000, -32000, INT_MAX, INT_MIN, INT_MAX - 1,
    INT_MIN + 1
};

static size_t test_values_s[] =
    { 0, 1, 1024, 12345, 32000, SIZE_MAX / 2 - 1,
    SIZE_MAX / 2, SIZE_MAX / 2 + 1, SIZE_MAX - 1,
    SIZE_MAX
};

static int test_sizeofs(void)
{
    if (!TEST_uint_eq(OSSL_NELEM(test_values), OSSL_NELEM(test_values_s)))
        return 0;
    return 1;
}

static int test_binops(int i)
{
    unsigned int a = test_values[i];
    unsigned int g = test_values_s[i];
    int j;
    int ret = 1;

    if (!test_is_zero(a) || !test_is_zero_8(a) || !test_is_zero_s(g))
        ret = 0;

    for (j = 0; j < (int)OSSL_NELEM(test_values); ++j) {
        unsigned int b = test_values[j];
        unsigned int h = test_values[j];

        if (!test_select(a, b)
                || !test_select_s(g, h)
                || !test_eq_s(g, h)
                || !test_binary_op(&constant_time_lt, "ct_lt",
                                   a, b, a < b)
                || !test_binary_op_8(&constant_time_lt_8, "constant_time_lt_8",
                                     a, b, a < b)
                || !test_binary_op_s(&constant_time_lt_s, "constant_time_lt_s",
                                     g, h, g < h)
                || !test_binary_op(&constant_time_lt, "constant_time_lt",
                                   b, a, b < a)
                || !test_binary_op_8(&constant_time_lt_8, "constant_time_lt_8",
                                     b, a, b < a)
                || !test_binary_op_s(&constant_time_lt_s, "constant_time_lt_s",
                                     h, g, h < g)
                || !test_binary_op(&constant_time_ge, "constant_time_ge",
                                   a, b, a >= b)
                || !test_binary_op_8(&constant_time_ge_8, "constant_time_ge_8",
                                     a, b, a >= b)
                || !test_binary_op_s(&constant_time_ge_s, "constant_time_ge_s",
                                     g, h, g >= h)
                || !test_binary_op(&constant_time_ge, "constant_time_ge",
                                   b, a, b >= a)
                || !test_binary_op_8(&constant_time_ge_8, "constant_time_ge_8",
                                     b, a, b >= a)
                || !test_binary_op_s(&constant_time_ge_s, "constant_time_ge_s",
                                     h, g, h >= g)
                || !test_binary_op(&constant_time_eq, "constant_time_eq",
                                   a, b, a == b)
                || !test_binary_op_8(&constant_time_eq_8, "constant_time_eq_8",
                                     a, b, a == b)
                || !test_binary_op_s(&constant_time_eq_s, "constant_time_eq_s",
                                     g, h, g == h)
                || !test_binary_op(&constant_time_eq, "constant_time_eq",
                                   b, a, b == a)
                || !test_binary_op_8(&constant_time_eq_8, "constant_time_eq_8",
                                     b, a, b == a)
                || !test_binary_op_s(&constant_time_eq_s, "constant_time_eq_s",
                                     h, g, h == g)) {
            ret = 0;
        }
    }
    return ret;
}

static int test_signed(int i)
{
    int c = signed_test_values[i];
    unsigned int j;
    int ret = 1;

    for (j = 0; j < OSSL_NELEM(signed_test_values); ++j) {
        int d = signed_test_values[j];

        if (!test_select_int(c, d)
                || !test_eq_int(c, d)
                || !test_eq_int_8(c, d))
            ret = 0;
    }
    return ret;
}

static int test_8values(int i)
{
    unsigned char e = test_values_8[i];
    unsigned int j;
    int ret = 1;

    for (j = 0; j < sizeof(test_values_8); ++j) {
        unsigned char f = test_values_8[j];

        if (!test_select_8(e, f))
            ret = 0;
    }
    return ret;
}


void register_tests(void)
{
    ADD_TEST(test_sizeofs);
    ADD_ALL_TESTS(test_binops, OSSL_NELEM(test_values));
    ADD_ALL_TESTS(test_signed, OSSL_NELEM(signed_test_values));
    ADD_ALL_TESTS(test_8values, sizeof(test_values_8));
}
