/**
 * @file wind.spec.c
 * @brief Host tests for the wind-speed conversions and labels.
 *
 * Each unit has its own integer factor and the maths truncates rather than rounds, so
 * the factors, the truncation, and the km/h fallback are what these tests pin down.
 */
#include "unity.h"

#include "units/wind.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief 100 km/h is about 62 mph, so a wrong factor would show the wrong gust. */
void test_wind_mph_conversion(void)
{
    int result = wind_from_kmh(100, WIND_UNIT_MPH);

    TEST_ASSERT_EQUAL_INT(62, result);
}

/** @brief 100 km/h is about 54 knots, the sailor's reading. */
void test_wind_kts_conversion(void)
{
    int result = wind_from_kmh(100, WIND_UNIT_KTS);

    TEST_ASSERT_EQUAL_INT(54, result);
}

/** @brief 100 km/h is about 27 m/s, and the truncation must land on 27 not 28. */
void test_wind_ms_conversion(void)
{
    int result = wind_from_kmh(100, WIND_UNIT_MS);

    TEST_ASSERT_EQUAL_INT(27, result);
}

/** @brief km/h is the source unit so it must pass straight through untouched. */
void test_wind_kmh_passthrough(void)
{
    int result = wind_from_kmh(100, WIND_UNIT_KMH);

    TEST_ASSERT_EQUAL_INT(100, result);
}

/** @brief The maths truncates, so 10 km/h in mph must floor to 6 rather than round to 7. */
void test_wind_truncates_toward_zero(void)
{
    int result = wind_from_kmh(10, WIND_UNIT_MPH);

    TEST_ASSERT_EQUAL_INT(6, result);
}

/** @brief Each unit must carry its own short label or the number sits next to the wrong tag. */
void test_wind_unit_labels(void)
{
    TEST_ASSERT_EQUAL_STRING("KM/H", wind_unit_label(WIND_UNIT_KMH));
    TEST_ASSERT_EQUAL_STRING("MPH", wind_unit_label(WIND_UNIT_MPH));
    TEST_ASSERT_EQUAL_STRING("KTS", wind_unit_label(WIND_UNIT_KTS));
    TEST_ASSERT_EQUAL_STRING("M/S", wind_unit_label(WIND_UNIT_MS));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_wind_mph_conversion);
    RUN_TEST(test_wind_kts_conversion);
    RUN_TEST(test_wind_ms_conversion);
    RUN_TEST(test_wind_kmh_passthrough);
    RUN_TEST(test_wind_truncates_toward_zero);
    RUN_TEST(test_wind_unit_labels);

    return UNITY_END();
}
