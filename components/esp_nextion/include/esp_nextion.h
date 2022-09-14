#ifndef ESP_NEXTION_H
#define ESP_NEXTION_H

void nex_text_change_temp(char *buf, int tNumber, uint8_t value);
void nex_number_change_temp(char *buf, int nNumber, uint8_t value);
void nex_pic_change_pic_id(char *buf, int pNumber, uint8_t value);
void nex_button_change_pic_id(char *buf, int bNumber, uint8_t value);
void nex_button_change_pic2_id(char *buf, int bNumber, uint8_t value);
void nex_change_page(char *buf, int value);

#endif
