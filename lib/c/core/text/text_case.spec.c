/**
 * @file text_case.spec.c
 * @brief Host tests for the in place ascii uppercase helper.
 *
 * It only shifts a through z, so the thing to guard is that everything else (digits,
 * punctuation, an already empty string) is left exactly as it was.
 */
#include "unity.h"

#include "text/text_case.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief Plain lowercase must come back fully uppercased, the core job of the helper. */
void test_text_to_upper_uppercases_lowercase(void)
{
    char text[] = "hello";

    text_to_upper(text);

    TEST_ASSERT_EQUAL_STRING("HELLO", text);
}

/** @brief Digits and punctuation must pass through untouched or a label like "12:00" would get mangled. */
void test_text_to_upper_leaves_non_letters(void)
{
    char text[] = "MixedCase123!";

    text_to_upper(text);

    TEST_ASSERT_EQUAL_STRING("MIXEDCASE123!", text);
}

/** @brief An empty string must stay empty rather than walk off the end of the buffer. */
void test_text_to_upper_empty_string_is_noop(void)
{
    char text[] = "";

    text_to_upper(text);

    TEST_ASSERT_EQUAL_STRING("", text);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_text_to_upper_uppercases_lowercase);
    RUN_TEST(test_text_to_upper_leaves_non_letters);
    RUN_TEST(test_text_to_upper_empty_string_is_noop);

    return UNITY_END();
}
