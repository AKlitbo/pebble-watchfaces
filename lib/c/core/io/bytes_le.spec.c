/**
 * @file bytes_le.spec.c
 * @brief Host tests for the little-endian byte readers.
 *
 * Everything the phone sends arrives through these two functions, so a slip here is a wrong number
 * on every panel fed by a strip. The half worth pinning is the sign: a temperature of -1 and a
 * temperature of 65535 are the same two bytes, and only the return type tells them apart. The other
 * half is the top bit of the top byte, where a missing cast would drag the sign across a shift.
 */
#include "unity.h"

#include "io/bytes_le.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief The everyday case: two bytes low first, the way the phone packs them. */
void test_i16_reads_low_byte_first(void)
{
    const uint8_t bytes[2] = { 0x34, 0x12 };

    int16_t result = read_i16_le(bytes);

    TEST_ASSERT_EQUAL_INT16(0x1234, result);
}

/**
 * @brief The top bit means negative, not a big positive.
 *
 * A stock that moved -1.25 percent packs as -125 hundredths. Read it unsigned and the watchlist
 * shows a stock up 655 percent instead of down one.
 */
void test_i16_sign_extends(void)
{
    const uint8_t bytes[2] = { 0x83, 0xFF };  // -125

    int16_t result = read_i16_le(bytes);

    TEST_ASSERT_EQUAL_INT16(-125, result);
}

/** @brief The most negative value there is, where an off-by-one in the sign shows up. */
void test_i16_reads_the_bottom_of_the_range(void)
{
    const uint8_t bytes[2] = { 0x00, 0x80 };

    int16_t result = read_i16_le(bytes);

    TEST_ASSERT_EQUAL_INT16(-32768, result);
}

/** @brief Every bit set is -1 rather than 65535. */
void test_i16_all_ones_is_minus_one(void)
{
    const uint8_t bytes[2] = { 0xFF, 0xFF };

    int16_t result = read_i16_le(bytes);

    TEST_ASSERT_EQUAL_INT16(-1, result);
}

/** @brief Four bytes low first, which is how a price in cents and an event epoch both arrive. */
void test_i32_reads_low_byte_first(void)
{
    const uint8_t bytes[4] = { 0x78, 0x56, 0x34, 0x12 };

    int32_t result = read_i32_le(bytes);

    TEST_ASSERT_EQUAL_INT32(0x12345678, result);
}

/**
 * @brief The top byte must go up through the shift unsigned before the whole becomes signed.
 *
 * This is the one that breaks quietly. Shift a plain char left 24 and the sign rides along and
 * poisons the bits below it. Every calendar event past 2038 and every negative price lands here.
 */
void test_i32_sign_extends(void)
{
    const uint8_t bytes[4] = { 0x9C, 0xFF, 0xFF, 0xFF };  // -100

    int32_t result = read_i32_le(bytes);

    TEST_ASSERT_EQUAL_INT32(-100, result);
}

/** @brief The most negative value there is, the far edge of the same shift. */
void test_i32_reads_the_bottom_of_the_range(void)
{
    const uint8_t bytes[4] = { 0x00, 0x00, 0x00, 0x80 };

    int32_t result = read_i32_le(bytes);

    TEST_ASSERT_EQUAL_INT32(INT32_MIN, result);
}

/** @brief A real epoch with the top bit clear must survive unchanged. */
void test_i32_reads_a_real_epoch(void)
{
    const uint8_t bytes[4] = { 0x00, 0xF1, 0x53, 0x65 };  // 1700000000 is 0x6553F100

    int32_t result = read_i32_le(bytes);

    TEST_ASSERT_EQUAL_INT32(1700000000, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_i16_reads_low_byte_first);
    RUN_TEST(test_i16_sign_extends);
    RUN_TEST(test_i16_reads_the_bottom_of_the_range);
    RUN_TEST(test_i16_all_ones_is_minus_one);
    RUN_TEST(test_i32_reads_low_byte_first);
    RUN_TEST(test_i32_sign_extends);
    RUN_TEST(test_i32_reads_the_bottom_of_the_range);
    RUN_TEST(test_i32_reads_a_real_epoch);

    return UNITY_END();
}
