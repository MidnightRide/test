#include <core/logger.h>
#include <math/mmath.h>

/**
 * @brief Expects expected to be equal to actual.
 */
#define expect_should_be(expected, actual)                                                              \
if (actual != expected)                                                                             \
{                                                                                                   \
MERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
return false;                                                                                   \
}

/**
 * @brief Expects expected to not be equal to actual.
 */
#define expect_should_not_be(expected, actual)                                                                   \
if (actual == expected)                                                                                      \
{                                                                                                            \
MERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
return false;                                                                                            \
}

/**
 * @brief Expects expected to be actual within a given epsilon.
 */
#define expect_float_to_be(expected, actual)                                                        \
if (mabs(expected - actual) > 0.001f)                                                           \
{                                                                                               \
MERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
return false;                                                                               \
}

/**
 * @brief Expects actual to be true.
 */
#define expect_to_be_true(actual)                                                      \
if (actual != true)                                                                \
{                                                                                  \
MERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__, __LINE__); \
return false;                                                                  \
}

/**
 * @brief Expects actual to be false.
 */
#define expect_to_be_false(actual)                                                      \
if (actual != false)                                                                \
{                                                                                  \
MERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__, __LINE__); \
return false;                                                                  \
}