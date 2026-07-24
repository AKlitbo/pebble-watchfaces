/**
 * @file stock_wire.spec.c
 * @brief Host tests for unpacking the watchlist off the wire.
 *
 * Untrusted bytes going into fixed buffers, with every length being the phone's word rather than
 * ours. The things worth pinning: a count past the array is pinned, a symbol reaching past the
 * message is refused, an over-long symbol is cut but still stepped over whole, and a message that
 * stops halfway leaves the caller's quotes alone instead of half replacing them.
 */
#include "unity.h"

#include "wire/stock_wire.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// build one wire slot: [ok][price int32 LE][pct int16 LE][symLen][sym bytes]
static uint16_t put_slot(uint8_t *buffer, uint16_t offset, bool ok, int32_t price, int16_t pct,
                         const char *symbol)
{
    buffer[offset++] = ok ? 1 : 0;
    buffer[offset++] = price & 0xFF;
    buffer[offset++] = (price >> 8) & 0xFF;
    buffer[offset++] = (price >> 16) & 0xFF;
    buffer[offset++] = (price >> 24) & 0xFF;
    buffer[offset++] = pct & 0xFF;
    buffer[offset++] = (pct >> 8) & 0xFF;

    uint8_t len = (uint8_t)strlen(symbol);
    buffer[offset++] = len;
    memcpy(buffer + offset, symbol, len);
    return offset + len;
}

/** @brief A clean pair of quotes must unpack fully or the watchlist shows blanks. */
void test_reads_clean_quotes(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 2;
    len = put_slot(buffer, len, true, 15012, -125, "AAPL");
    len = put_slot(buffer, len, true, 9800, 340, "MSFT");

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(2, out.count);
    TEST_ASSERT_EQUAL_STRING("AAPL", out.slot[0].symbol);
    TEST_ASSERT_EQUAL_INT(15012, out.slot[0].price_cents);
    TEST_ASSERT_EQUAL_INT(-125, out.slot[0].change_pct);
    TEST_ASSERT_TRUE(out.slot[0].ok);
    TEST_ASSERT_EQUAL_STRING("MSFT", out.slot[1].symbol);
    TEST_ASSERT_EQUAL_INT(340, out.slot[1].change_pct);
}

/** @brief A failed quote carries its status word where the ticker goes, and must read as not ok. */
void test_reads_a_failed_quote(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_slot(buffer, len, false, 0, 0, "RATE LIMIT");

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(out.slot[0].ok);
    TEST_ASSERT_EQUAL_STRING("RATE LIMIT", out.slot[0].symbol);
}

/**
 * @brief A count past what the strip holds must be pinned to it.
 *
 * The buffer has to carry a full strip of real quotes to catch this. Stop the bytes short and the
 * reader refuses on the bounds check whether or not the count was pinned, which looks like a pass
 * and proves nothing.
 */
void test_pins_a_count_past_the_array(void)
{
    uint8_t buffer[128];
    uint16_t len = 0;
    buffer[len++] = 200;
    for (int i = 0; i < STOCK_MAX_SLOTS; i++)
    {
        char sym[8];
        snprintf(sym, sizeof(sym), "S%d", i);
        len = put_slot(buffer, len, true, 1000 + i, (int16_t)i, sym);
    }

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(STOCK_MAX_SLOTS, out.count);
    TEST_ASSERT_EQUAL_STRING("S3", out.slot[STOCK_MAX_SLOTS - 1].symbol);
}

/**
 * @brief A message claiming more quotes than it carries is refused whole.
 *
 * This is the watch side of the sparse-round bug: the phone wrote a count of four and then only
 * managed two, so the walk runs out of bytes partway. Refusing the lot is what keeps a half strip
 * off the screen.
 */
void test_refuses_a_message_that_stops_mid_run(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 4;  // says four
    len = put_slot(buffer, len, true, 15012, -125, "AAPL");
    len = put_slot(buffer, len, true, 9800, 340, "MSFT");  // sends two

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
}

/** @brief A symbol reaching past the message is refused rather than read out of bounds. */
void test_refuses_a_symbol_past_the_message(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 1;
    buffer[len++] = 1;                        // ok
    for (int i = 0; i < 6; i++) { buffer[len++] = 0; }  // price + pct
    buffer[len++] = 40;                       // symLen lies: 40 bytes claimed
    buffer[len++] = 'A';
    buffer[len++] = 'B';

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
}

/**
 * @brief An over-long symbol is cut to fit but the walk still steps past all of it.
 *
 * Cutting the copy and the walk together is the trap: the walk has to move by what the phone said
 * the symbol was, not by what fit. Step short and the reader lands inside the tail of the symbol
 * it just read and takes the rest of it for the next quote.
 */
void test_cuts_an_overlong_symbol_but_steps_past_all_of_it(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 2;
    len = put_slot(buffer, len, true, 100, 0, "WAYTOOLONGTICKER");
    len = put_slot(buffer, len, true, 200, 0, "OK");

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT(STOCK_SYMBOL_LEN - 1, strlen(out.slot[0].symbol));
    // the second quote only reads right if the walk stepped past the whole first symbol
    TEST_ASSERT_EQUAL_STRING("OK", out.slot[1].symbol);
    TEST_ASSERT_EQUAL_INT(200, out.slot[1].price_cents);
}

/** @brief A refused message must leave the caller's quotes untouched. */
void test_leaves_the_quotes_alone_when_it_refuses(void)
{
    StockStrip out = {0};
    out.count = 2;
    strcpy(out.slot[0].symbol, "KEEP");

    uint8_t buffer[4] = { 3, 0x01, 0x02, 0x03 };
    bool result = stock_wire_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_UINT8(2, out.count);
    TEST_ASSERT_EQUAL_STRING("KEEP", out.slot[0].symbol);
}

/** @brief A negative price must survive the trip, since a change can be negative too. */
void test_reads_a_negative_change(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_slot(buffer, len, true, 50, -4700, "X");

    StockStrip out;
    bool result = stock_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(-4700, out.slot[0].change_pct);
}

/** @brief An empty watchlist is a real reading: the phone saying nothing is configured. */
void test_reads_an_empty_watchlist(void)
{
    uint8_t buffer[1] = { 0 };

    StockStrip out;
    bool result = stock_wire_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(0, out.count);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_reads_clean_quotes);
    RUN_TEST(test_reads_a_failed_quote);
    RUN_TEST(test_pins_a_count_past_the_array);
    RUN_TEST(test_refuses_a_message_that_stops_mid_run);
    RUN_TEST(test_refuses_a_symbol_past_the_message);
    RUN_TEST(test_cuts_an_overlong_symbol_but_steps_past_all_of_it);
    RUN_TEST(test_leaves_the_quotes_alone_when_it_refuses);
    RUN_TEST(test_reads_a_negative_change);
    RUN_TEST(test_reads_an_empty_watchlist);

    return UNITY_END();
}
