/**
 * @file number_format.spec.c
 * @brief Host tests for the thousands-separator formatter.
 *
 * The separator placement, the leading sign, and the buffer guard are the parts that
 * bite. A slip shows a step count with a stray apostrophe or smashes the buffer.
 */
#include "unity.h"

#include <limits.h>

#include "text/number_format.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A five digit count must split into "20'358", the whole reason the helper exists. */
void test_number_group_inserts_separators(void)
{
    char buffer[24];

    number_group(buffer, sizeof(buffer), 20358);

    TEST_ASSERT_EQUAL_STRING("20'358", buffer);
}

/** @brief Under a thousand there is no group to break, so a stray separator must not appear. */
void test_number_group_below_thousand_has_no_separator(void)
{
    char buffer[24];

    number_group(buffer, sizeof(buffer), 999);

    TEST_ASSERT_EQUAL_STRING("999", buffer);
}

/** @brief Exactly one thousand is the first place a separator belongs, so "1'000" must round trip. */
void test_number_group_exact_thousand(void)
{
    char buffer[24];

    number_group(buffer, sizeof(buffer), 1000);

    TEST_ASSERT_EQUAL_STRING("1'000", buffer);
}

/** @brief A negative value must keep its sign ahead of the digits or a below zero reading looks positive. */
void test_number_group_keeps_negative_sign(void)
{
    char buffer[24];

    number_group(buffer, sizeof(buffer), -1234);

    TEST_ASSERT_EQUAL_STRING("-1'234", buffer);
}

/** @brief A seven digit value must carry two separators, proving the grouping repeats not just fires once. */
void test_number_group_multiple_groups(void)
{
    char buffer[24];

    number_group(buffer, sizeof(buffer), 1234567);

    TEST_ASSERT_EQUAL_STRING("1'234'567", buffer);
}

/** @brief The most negative int must format without tripping undefined behaviour on the sign flip. */
void test_number_group_handles_int_min(void)
{
    char buffer[24];

    number_group(buffer, sizeof(buffer), INT_MIN);

    TEST_ASSERT_EQUAL_STRING("-2'147'483'648", buffer);
}

/** @brief A tiny buffer must truncate cleanly and stay terminated rather than overrun the caller. */
void test_number_group_truncates_into_small_buffer(void)
{
    char buffer[3];

    number_group(buffer, sizeof(buffer), 20358);

    TEST_ASSERT_EQUAL_STRING("20", buffer);
}

/** @brief A real reading goes through the caller's format, which is the everyday case. */
void test_fmt_int_or_dash_formats_a_reading(void)
{
    char buffer[16];

    fmt_int_or_dash(buffer, sizeof(buffer), 42, "%d%%");

    TEST_ASSERT_EQUAL_STRING("42%", buffer);
}

/**
 * @brief A negative reading is the store saying it has nothing, and must draw as dashes.
 *
 * Every panel fed by the phone starts out with no reading, so getting this wrong puts a -1 on
 * screen where a humidity or a pressure belongs.
 */
void test_fmt_int_or_dash_draws_dashes_for_no_data(void)
{
    char buffer[16];

    fmt_int_or_dash(buffer, sizeof(buffer), -1, "%d%%");

    TEST_ASSERT_EQUAL_STRING("--", buffer);
}

/** @brief A real zero is a reading and not an absence, so it must not read as dashes. */
void test_fmt_int_or_dash_keeps_a_real_zero(void)
{
    char buffer[16];

    fmt_int_or_dash(buffer, sizeof(buffer), 0, "%d%%");

    TEST_ASSERT_EQUAL_STRING("0%", buffer);
}

/** @brief Any negative reads as no data and not just the -1 the stores happen to use. */
void test_fmt_int_or_dash_treats_every_negative_as_no_data(void)
{
    char buffer[16];

    fmt_int_or_dash(buffer, sizeof(buffer), -1000, "%d");

    TEST_ASSERT_EQUAL_STRING("--", buffer);
}

/** @brief A plain price, the everyday case the rest are corners of. */
void test_fmt_hundredths_writes_two_decimals(void)
{
    char buffer[16];

    fmt_hundredths(buffer, sizeof(buffer), 26174);

    TEST_ASSERT_EQUAL_STRING("261.74", buffer);
}

/**
 * @brief A value under one unit keeps its minus, which is the whole reason the sign is pulled out.
 *
 * -50 cents has a whole part of zero, and zero has no sign to print. Let the whole part carry it
 * and the watchlist shows a stock down 50 cents as up 50 cents.
 */
void test_fmt_hundredths_keeps_the_sign_under_one_unit(void)
{
    char buffer[16];

    fmt_hundredths(buffer, sizeof(buffer), -50);

    TEST_ASSERT_EQUAL_STRING("-0.50", buffer);
}

/** @brief The fraction is padded, so nine cents is .09 and not .9. */
void test_fmt_hundredths_pads_the_fraction(void)
{
    char buffer[16];

    fmt_hundredths(buffer, sizeof(buffer), 909);

    TEST_ASSERT_EQUAL_STRING("9.09", buffer);
}

/** @brief A gain wears a plus so the direction reads without needing the colour. */
void test_fmt_pct_signed_marks_a_gain(void)
{
    char buffer[16];

    fmt_pct_signed(buffer, sizeof(buffer), 125);

    TEST_ASSERT_EQUAL_STRING("+1.25%", buffer);
}

/** @brief A loss under one percent keeps its minus, same trap as the price. */
void test_fmt_pct_signed_marks_a_small_loss(void)
{
    char buffer[16];

    fmt_pct_signed(buffer, sizeof(buffer), -47);

    TEST_ASSERT_EQUAL_STRING("-0.47%", buffer);
}

/** @brief Flat is neither up nor down, so it wears no sign at all. */
void test_fmt_pct_signed_leaves_flat_unsigned(void)
{
    char buffer[16];

    fmt_pct_signed(buffer, sizeof(buffer), 0);

    TEST_ASSERT_EQUAL_STRING("0.00%", buffer);
}

/** @brief Text that fits arrives whole and terminated. */
void test_copy_bounded_copies_what_fits(void)
{
    const uint8_t src[] = { 'A', 'A', 'P', 'L' };
    char dst[12];

    copy_bounded(dst, sizeof(dst), src, sizeof(src));

    TEST_ASSERT_EQUAL_STRING("AAPL", dst);
}

/** @brief Text longer than the room takes what fits and still ends terminated. */
void test_copy_bounded_cuts_what_does_not_fit(void)
{
    const uint8_t src[] = { 'A', 'B', 'C', 'D', 'E', 'F' };
    char dst[4];

    copy_bounded(dst, sizeof(dst), src, sizeof(src));

    TEST_ASSERT_EQUAL_STRING("ABC", dst);
}

/** @brief Filling the buffer exactly must still leave room for the terminator. */
void test_copy_bounded_keeps_room_for_the_terminator(void)
{
    const uint8_t src[] = { 'A', 'B', 'C', 'D' };
    char dst[5];

    copy_bounded(dst, sizeof(dst), src, sizeof(src));

    TEST_ASSERT_EQUAL_STRING("ABCD", dst);
    TEST_ASSERT_EQUAL_CHAR('\0', dst[4]);
}

/** @brief Nothing to copy still terminates, so the caller never reads stale bytes. */
void test_copy_bounded_terminates_an_empty_copy(void)
{
    const uint8_t src[] = { 'X' };
    char dst[4] = { 'o', 'l', 'd', '\0' };

    copy_bounded(dst, sizeof(dst), src, 0);

    TEST_ASSERT_EQUAL_STRING("", dst);
}

/**
 * @brief A buffer with no room at all must be left alone rather than filled.
 *
 * This is the one worth having. cap is a byte and cap - 1 promotes to int, so a cap of zero would
 * come out as -1, the compare against it would be true whatever the length, and the copy would run
 * 255 bytes over a buffer that had no room for one. Nothing passes zero today, which is exactly
 * why it needs saying out loud somewhere that fails when it stops being true.
 */
void test_copy_bounded_writes_nothing_when_there_is_no_room(void)
{
    const uint8_t src[] = { 'A', 'B', 'C' };
    char dst[4] = { 1, 2, 3, 4 };

    copy_bounded(dst, 0, src, sizeof(src));

    TEST_ASSERT_EQUAL_CHAR(1, dst[0]);
    TEST_ASSERT_EQUAL_CHAR(2, dst[1]);
    TEST_ASSERT_EQUAL_CHAR(3, dst[2]);
    TEST_ASSERT_EQUAL_CHAR(4, dst[3]);
}

/** @brief A buffer with room for only a terminator gets one and nothing else. */
void test_copy_bounded_handles_room_for_only_a_terminator(void)
{
    const uint8_t src[] = { 'A', 'B' };
    char dst[2] = { 'x', 'y' };

    copy_bounded(dst, 1, src, sizeof(src));

    TEST_ASSERT_EQUAL_STRING("", dst);
    TEST_ASSERT_EQUAL_CHAR('y', dst[1]);  // nothing past its own room
}

/** @brief Plain text that ends inside its room is worth keeping. */
void test_cstring_is_clean_accepts_plain_text(void)
{
    char s[32] = "London";

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_TRUE(result);
}

/**
 * @brief A place name spelled with an umlaut is text, not damage.
 *
 * The one that matters. UTF-8 puts bytes above 0x7E in a name like Zurich, and a range test that
 * only trusts plain ASCII throws the whole name away, resets the field to its default and saves
 * that back. The user picks a city, it works, and next boot it is London again. Sao Paulo drifts
 * four hours doing it.
 */
void test_cstring_is_clean_accepts_utf8(void)
{
    char s[32] = "Z\xC3\xBCrich";  // Zurich with the umlaut, UTF-8

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_TRUE(result);
}

/** @brief A control byte is what a damaged blob reads like, so it is not worth keeping. */
void test_cstring_is_clean_refuses_a_control_byte(void)
{
    char s[32] = "Lon\x01" "don";

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_FALSE(result);
}

/** @brief DEL sits above the control block but is no more text than the ones below it. */
void test_cstring_is_clean_refuses_del(void)
{
    char s[32] = "Lon\x7F" "don";

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_FALSE(result);
}

/** @brief Space is the first real character, so it must not be taken for a control byte. */
void test_cstring_is_clean_accepts_a_space(void)
{
    char s[32] = "New York";

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_TRUE(result);
}

/** @brief An empty string says nothing, so there is nothing to keep. */
void test_cstring_is_clean_refuses_an_empty_string(void)
{
    char s[32] = "";

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_FALSE(result);
}

/**
 * @brief A string that never ends inside its room is not a string.
 *
 * Reading it would run on past the field into whatever the blob keeps next door.
 */
void test_cstring_is_clean_refuses_one_that_never_ends(void)
{
    char s[4] = { 'a', 'b', 'c', 'd' };  // no terminator anywhere in it

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_FALSE(result);
}

/** @brief Ending on the last byte it has room for is still ending, so it counts. */
void test_cstring_is_clean_accepts_ending_on_the_last_byte(void)
{
    char s[4] = { 'a', 'b', 'c', '\0' };

    bool result = cstring_is_clean(s, sizeof(s));

    TEST_ASSERT_TRUE(result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_number_group_inserts_separators);
    RUN_TEST(test_number_group_below_thousand_has_no_separator);
    RUN_TEST(test_number_group_exact_thousand);
    RUN_TEST(test_number_group_keeps_negative_sign);
    RUN_TEST(test_number_group_multiple_groups);
    RUN_TEST(test_number_group_handles_int_min);
    RUN_TEST(test_number_group_truncates_into_small_buffer);
    RUN_TEST(test_fmt_int_or_dash_formats_a_reading);
    RUN_TEST(test_fmt_int_or_dash_draws_dashes_for_no_data);
    RUN_TEST(test_fmt_int_or_dash_keeps_a_real_zero);
    RUN_TEST(test_fmt_int_or_dash_treats_every_negative_as_no_data);
    RUN_TEST(test_fmt_hundredths_writes_two_decimals);
    RUN_TEST(test_fmt_hundredths_keeps_the_sign_under_one_unit);
    RUN_TEST(test_fmt_hundredths_pads_the_fraction);
    RUN_TEST(test_fmt_pct_signed_marks_a_gain);
    RUN_TEST(test_fmt_pct_signed_marks_a_small_loss);
    RUN_TEST(test_fmt_pct_signed_leaves_flat_unsigned);
    RUN_TEST(test_copy_bounded_copies_what_fits);
    RUN_TEST(test_copy_bounded_cuts_what_does_not_fit);
    RUN_TEST(test_copy_bounded_keeps_room_for_the_terminator);
    RUN_TEST(test_copy_bounded_terminates_an_empty_copy);
    RUN_TEST(test_copy_bounded_writes_nothing_when_there_is_no_room);
    RUN_TEST(test_copy_bounded_handles_room_for_only_a_terminator);
    RUN_TEST(test_cstring_is_clean_accepts_plain_text);
    RUN_TEST(test_cstring_is_clean_accepts_utf8);
    RUN_TEST(test_cstring_is_clean_refuses_a_control_byte);
    RUN_TEST(test_cstring_is_clean_refuses_del);
    RUN_TEST(test_cstring_is_clean_accepts_a_space);
    RUN_TEST(test_cstring_is_clean_refuses_an_empty_string);
    RUN_TEST(test_cstring_is_clean_refuses_one_that_never_ends);
    RUN_TEST(test_cstring_is_clean_accepts_ending_on_the_last_byte);

    return UNITY_END();
}
