/**
 * @file store_persist.h
 * @brief Shared persist save and restore helpers for the stores.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_stores
 * @{
 */

// every store stamps the first byte of its blob with a tag. the high nibble names the store and
// the low nibble is that store's layout revision. the size on its own cannot tell two blobs apart
// (the calendar snapshot and the weather state happen to be the same size) and reordering two
// fields keeps the size too, so the tag is what makes the guard mean anything. bump a store's low
// nibble whenever its persisted struct changes shape, and an older blob is dropped rather than
// read as the wrong thing
#define STORE_TAG_WEATHER  0x11
#define STORE_TAG_STOCK    0x21
#define STORE_TAG_CALENDAR 0x31
#define STORE_TAG_HEALTH   0x41
#define STORE_TAG_LOCATION 0x51

/**
 * @brief Restore a saved blob into @p state, but only when it is this store's own current layout.
 *
 * Checks the size and then the tag byte. Nothing is written into @p state unless both match, so
 * the defaults the caller applied before calling survive a rejected blob.
 *
 * @param key The persist slot the face handed the store.
 * @param state The struct to fill. Its first field must be the uint8_t tag.
 * @param size sizeof that struct.
 * @param tag The store's STORE_TAG_* value.
 * @return Whether a saved blob was restored.
 */
static inline bool store_restore(uint32_t key, void *state, size_t size, uint8_t tag)
{
    if (!persist_exists(key) || persist_get_size(key) != (int)size)
    {
        return false;
    }

    // read the tag on its own first. reading the whole blob to check it would already have
    // stomped the defaults by the time we found out it was the wrong shape
    uint8_t stored = 0;
    if (persist_read_data(key, &stored, sizeof(stored)) != (int)sizeof(stored) || stored != tag)
    {
        return false;
    }

    return persist_read_data(key, state, size) == (int)size;
}

/**
 * @brief Save a store's blob with its tag stamped in, so the next restore can recognise it.
 *
 * @param key The persist slot the face handed the store.
 * @param state The struct to write. Its first field must be the uint8_t tag.
 * @param size sizeof that struct.
 * @param tag The store's STORE_TAG_* value.
 * @return Whether the whole blob reached flash. False means the cache did not land, so a caller
 *   that tracks a dirty flag can hold it and try again.
 */
static inline bool store_save(uint32_t key, void *state, size_t size, uint8_t tag)
{
    *(uint8_t *)state = tag;
    return persist_write_data(key, state, size) == (int)size;
}

/** @} */
