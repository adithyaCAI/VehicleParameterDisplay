/* SH1106 module section 4.4.2 initialization and PRD two-column addressing. */
#include "sh1106.h"
#include <stddef.h>
#include <string.h>
bool sh1106_init(sh1106_send_t send, uint8_t address)
{
    uint8_t commands[] = {0x00, 0xAE, 0xD5, 0x80, 0xA8, 0x3F,
        0xD3, 0x00, 0x40, 0xAD, 0x8B, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xFF, 0xD9, 0x1F, 0xDB, 0x40, 0x33, 0xA6};
    return (send != NULL) && send(address, commands, sizeof(commands));
}
bool sh1106_page(sh1106_send_t send, uint8_t address, uint8_t page, const uint8_t *pixels)
{
    enum { PAGE_COUNT = 8, CHUNK_BYTES = 64, RAM_OFFSET = 2 };
    uint8_t commands[] = {0x00, (uint8_t)(0xB0U | page), RAM_OFFSET, 0x10};
    uint8_t packet[CHUNK_BYTES + 1];
    if ((send == NULL) || (pixels == NULL) || (page >= PAGE_COUNT))
    {
        return false;
    }
    if (!send(address, commands, sizeof(commands)))
    {
        return false;
    }
    packet[0] = 0x40;
    for (uint16_t column = 0U; column < 128U; column += CHUNK_BYTES)
    {
        memcpy(&packet[1], &pixels[column], CHUNK_BYTES);
        if (!send(address, packet, sizeof(packet)))
        {
            return false;
        }
    }
    return true;
}
bool sh1106_enable(sh1106_send_t send, uint8_t address)
{
    uint8_t command[] = {0x00, 0xAF};
    return (send != NULL) && send(address, command, sizeof(command));
}
