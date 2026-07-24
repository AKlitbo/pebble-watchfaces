/**
 * @file astro.spec.c
 * @brief Host tests for the pure astronomy math.
 *
 * The Julian date hangs off an exact epoch anchor, so a drifted constant shows the wrong day
 * count. The J2000 anchor value is what these tests pin down.
 */
#include "unity.h"

#include "clock/astro.h"

void setUp(void) {}
void tearDown(void) {}

// 2000-01-01 12:00:00 UT, the J2000 epoch, as a Unix timestamp
#define J2000_UNIX 946728000L

/** @brief The J2000 epoch must read exactly JD 2451545.00 or every Julian readout is off. */
void test_jd_at_j2000_is_2451545(void)
{
    int32_t result = astro_jd_centi(J2000_UNIX);

    TEST_ASSERT_EQUAL_INT32(245154500, result);
}

/** @brief Half a day before the epoch is midnight, JD 2451544.50, the .5 the formula bakes in. */
void test_jd_at_j2000_midnight_is_half(void)
{
    int32_t result = astro_jd_centi(J2000_UNIX - 43200);

    TEST_ASSERT_EQUAL_INT32(245154450, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_jd_at_j2000_is_2451545);
    RUN_TEST(test_jd_at_j2000_midnight_is_half);

    return UNITY_END();
}
