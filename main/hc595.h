/*
 * hc595.h
 *
 */

#ifndef APP_USER_INCLUDE_HC595_H_
#define APP_USER_INCLUDE_HC595_H_

#include <stdlib.h>
#include <stddef.h>
#include <esp_err.h>

#define DISP_DIGIT_1		0
#define DISP_DIGIT_2		1
#define DISP_DIGIT_3		2
#define DISP_DIGIT_4		3
#define DISP_DIGIT_MAX		4

typedef struct {
	uint8_t digit[DISP_DIGIT_MAX];
	uint8_t flush[DISP_DIGIT_MAX];
} tDisplay;

#define DISP_DIGIT1_MASK		(1<<0)
#define DISP_DIGIT2_MASK		(1<<1)
#define DISP_DIGIT3_MASK		(1<<2)
#define DISP_DIGIT4_MASK		(1<<3)
#define DISP_DIGIT_ONLY_MASK	(DISP_DIGIT4_MASK | DISP_DIGIT3_MASK | DISP_DIGIT2_MASK | DISP_DIGIT1_MASK)

#define DISP_FLASH1_MASK		(1<<4)	//STA GOT IP on
#define DISP_FLASH2_MASK		(1<<5)	//Cayenn on
#define DISP_FLASH3_MASK		(1<<6)
#define DISP_FLASH4_MASK		(1<<7)	//AP on
#define DISP_FLASH_ONLY_MASK	(DISP_FLASH4_MASK | DISP_FLASH3_MASK | DISP_FLASH2_MASK | DISP_FLASH1_MASK)

typedef uint8_t tDisplayMask;

void InitDisplay(void);
esp_err_t Show(tDisplay value, tDisplayMask mask);

#endif /* APP_USER_INCLUDE_HC595_H_ */
