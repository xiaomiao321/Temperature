#ifndef __leds_595_h
#define __leds_595_h
#include "string.h"
#include  "stdio.h"
#include  "stdint.h"
#include "stm32f10x.h"

extern uint16_t leds_flash_timer;

void leds_display_buf_set(uint16_t *buf);
void leds_flash(void);
void leds_595_io_init(void);
void leds_595_test();
#endif
