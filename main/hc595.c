/*
 * hc595.c
 *
 */
#include <stdlib.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>

#include "hc595.h"

#define DISP_SPI_MOSI		GPIO_Pin_5				//Display SPI MOSI
#define DISP_SPI_MOSI_NUM	GPIO_NUM_5				//Display SPI MOSI
#define DISP_SPI_CLK		GPIO_Pin_4				//Display SPI CLK
#define DISP_SPI_CLK_NUM	GPIO_NUM_4				//Display SPI CLK
#define DISP_SPI_UPD		GPIO_Pin_13				//Display SPI update output pins
#define DISP_SPI_UPD_NUM	GPIO_NUM_13				//Display SPI update output pins

#define DispClckLow()		gpio_set_level(DISP_SPI_CLK_NUM, 0)
#define DispClckHight()		gpio_set_level(DISP_SPI_CLK_NUM, 1)

#define DispUpdLow()		gpio_set_level(DISP_SPI_UPD_NUM, 0)
#define DispUpdHight()		gpio_set_level(DISP_SPI_UPD_NUM, 1)
#define DispDataSet(x)		gpio_set_level(DISP_SPI_MOSI_NUM, x?1:0)

#define DISP_REFRESH		5 						//Display refersh period, ms
#define DISP_FLASH_PERIOD	500						//Flash digit period, ms

//Сегменты индикатора
#define sA 	0b10000000	// A
#define sB 	0b00000010 	// B
#define sC 	0b00000001 	// C
#define sD 	0b00000100	// D
#define sE 	0b00010000	// E
#define sF 	0b00100000	// F
#define sG 	0b01000000	// G
#define sDP 0b00001000	// DP

//Знакогенератор
uint8_t Font[] = 				// Символ
		{ 0b10110111 	// 0
				, 0b00000011 	// 1
				, 0b11010110	// 2
				, 0b11000111	// 3
				, 0b01100011	// 4
				, 0b11100101	// 5
				, 0b11110101	// 6
				, 0b10000011 	// 7
				, 0b11110111 	// 8
				, 0b11100111 	// 9
				, 0b11110100 	// E A
				, 0b01010000 	// r B
				, 0b00000000 	// пока пусто C
				, 0b00000000 	// пока пусто D
				, 0b00000000 	// пока пусто E
				, 0b00000000 	// пока пусто F
		};

xQueueHandle qDisplay;

inline void out_word_disp(uint16_t Value) {
	uint16_t i;

	for (i = 0; i < 16; i++) {
		DispDataSet((Value & (1 << i)));
		DispClckLow();
		DispClckHight();
		DispClckLow();
	}
}

//Вывод одного разряда
static void refreshDisplay(void *pvParameters) {
	portTickType xLastWakeTime = xTaskGetTickCount();
	tDisplay digitShow;
	uint16_t digit = 0, flashCount = 0;
	uint8_t flash = 0;

	digitShow.digit[DISP_DIGIT_1] = 0;
	digitShow.digit[DISP_DIGIT_2] = 0;
	digitShow.digit[DISP_DIGIT_3] = 0;
	digitShow.digit[DISP_DIGIT_4] = 0;
	digitShow.flush[DISP_DIGIT_1] = 0;
	digitShow.flush[DISP_DIGIT_2] = 0;
	digitShow.flush[DISP_DIGIT_3] = 0;
	digitShow.flush[DISP_DIGIT_4] = 0;

	for (;;) {

		xQueueReceive(qDisplay, &digitShow, 0);

		DispUpdHight();

		if ((digitShow.flush[digit] & (1 << digit)) && flash) { //Мигание
			out_word_disp(((1 << digit) << 8) & 0xff00);
		} else {
			out_word_disp(
					(Font[digitShow.digit[DISP_DIGIT_MAX-1 - digit] & 0x0f] & 0x00ff)
							| (((1 << digit) << 8) & 0xff00));
		}
		DispUpdLow();
		digit++;
		if (digit == DISP_DIGIT_MAX) {
			digit = 0;
		}
		flashCount++;
		if (flashCount == DISP_FLASH_PERIOD/DISP_REFRESH){
			flashCount = 0;
			flash ^= 0xff;
		}
		vTaskDelayUntil(&xLastWakeTime, 1);
	}
}

esp_err_t Show(tDisplay value, tDisplayMask mask){

	esp_err_t res = ESP_ERR_NO_MEM;
	static tDisplay currentDisplay;
	currentDisplay.digit[DISP_DIGIT_1] = (mask & DISP_DIGIT1_MASK)?value.digit[DISP_DIGIT_1]:currentDisplay.digit[DISP_DIGIT_1];
	currentDisplay.digit[DISP_DIGIT_2] = (mask & DISP_DIGIT2_MASK)?value.digit[DISP_DIGIT_2]:currentDisplay.digit[DISP_DIGIT_2];
	currentDisplay.digit[DISP_DIGIT_3] = (mask & DISP_DIGIT3_MASK)?value.digit[DISP_DIGIT_3]:currentDisplay.digit[DISP_DIGIT_3];
	currentDisplay.digit[DISP_DIGIT_4] = (mask & DISP_DIGIT4_MASK)?value.digit[DISP_DIGIT_4]:currentDisplay.digit[DISP_DIGIT_4];

	currentDisplay.flush[DISP_DIGIT_1] = (mask & DISP_FLASH1_MASK)?value.flush[DISP_DIGIT_1]:currentDisplay.flush[DISP_DIGIT_1];
	currentDisplay.flush[DISP_DIGIT_2] = (mask & DISP_FLASH2_MASK)?value.flush[DISP_DIGIT_2]:currentDisplay.flush[DISP_DIGIT_2];
	currentDisplay.flush[DISP_DIGIT_3] = (mask & DISP_FLASH3_MASK)?value.flush[DISP_DIGIT_3]:currentDisplay.flush[DISP_DIGIT_3];
	currentDisplay.flush[DISP_DIGIT_4] = (mask & DISP_FLASH4_MASK)?value.flush[DISP_DIGIT_4]:currentDisplay.flush[DISP_DIGIT_4];

	if (xQueueSend(qDisplay, &currentDisplay, 1/portTICK_PERIOD_MS) != errQUEUE_FULL){
		res = ESP_OK;
	}
	return res;
}

void InitDisplay(void) {

	//hc595 output
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = (DISP_SPI_MOSI | DISP_SPI_CLK | DISP_SPI_UPD | GPIO_Pin_14);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);

	DispClckLow();
	DispUpdLow();

	qDisplay = xQueueCreate(3, sizeof(tDisplay));
	if (qDisplay) {
		xTaskCreate(refreshDisplay, "refDisplay", 4096, NULL, 0, NULL);
	}
}
