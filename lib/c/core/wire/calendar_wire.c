/**
 * @file calendar_wire.c
 * @brief The agenda as the phone packs it, and the reader that unpacks it.
 */
#include "wire/calendar_wire.h"

#include "io/bytes_le.h"
#include "text/number_format.h"

bool calendar_wire_decode(const uint8_t *buf, uint16_t len, CalendarStrip *out)
{
    if (!buf || !out || len < 1)
    {
        return false;
    }

    uint8_t count = buf[0];
    if (count > CALENDAR_MAX_SLOTS)
    {
        // the phone's word for how many, pinned to how many there is room for. without this the
        // fill below walks straight off the end of the event array
        count = CALENDAR_MAX_SLOTS;
    }

    // zero-init so the unused slot[count..MAX-1] bytes are deterministic, not stale
    // stack, once the whole strip gets persisted to flash
    CalendarStrip built = {0};

    uint16_t offset = 1;
    for (uint8_t i = 0; i < count; i++)
    {
        // the fixed part of an event is start(4) + end(4) + flags(1) + titleLen(1) = 10 bytes
        if (offset + 10 > len)
        {
            return false; // malformed, the caller keeps the old strip
        }

        CalendarEvent *event = &built.event[i];
        event->start = (time_t)read_i32_le(buf + offset);
        event->end = (time_t)read_i32_le(buf + offset + 4);
        event->all_day = (buf[offset + 8] & 0x01) != 0;
        uint8_t title_len = buf[offset + 9];
        offset += 10;

        if (offset + title_len > len)
        {
            return false; // title runs past the buffer
        }
        copy_bounded(event->title, CAL_TITLE_LEN, buf + offset, title_len);
        offset += title_len;

        // location is length-prefixed the same way, right after the title
        if (offset + 1 > len)
        {
            return false;
        }
        uint8_t loc_len = buf[offset];
        offset += 1;

        if (offset + loc_len > len)
        {
            return false; // location runs past the buffer
        }
        copy_bounded(event->location, CAL_LOC_LEN, buf + offset, loc_len);
        offset += loc_len;

        built.count++;
    }

    // only now, once the whole run has read clean
    *out = built;
    return true;
}
