#include <stdio.h>
#include "esp_nextion.h"
#include "esp_log.h"

#define MAX_BUFFER_SIZE 25

void nex_text_change_temp(char *buf, int tNumber, uint8_t value)
{
    sprintf(buf, "t%d.txt=\"%u%sC\"", tNumber, value, "\xb0");
}

void nex_pic_change_pic_id(char *buf, int pNumber, uint8_t value)
{
    sprintf(buf, "p%d.pic=%d", pNumber, value);
}

void nex_button_change_pic_id(char *buf, int bNumber, uint8_t value)
{
    sprintf(buf, "b%d.pic=%d", bNumber, value);
}

void nex_button_change_pic2_id(char *buf, int bNumber, uint8_t value)
{
    sprintf(buf, "b%d.pic2=%d", bNumber, value);
}

void nex_change_page(char *buf, int value)
{
    sprintf(buf, "page %d", value);
}
