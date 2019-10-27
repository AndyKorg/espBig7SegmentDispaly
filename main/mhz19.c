/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"

#include "mhz19.h"
#include "hc595.h"
#include "cayenne.h"
#include "params.h"

static const char *TAG = "MHZ19";

#define MHZ19_UART				UART_NUM_0
#define BUF_SIZE 				(UART_FIFO_LEN*2)
#define RX_BUF_SIZE 			BUF_SIZE
#define TX_BUF_SIZE				BUF_SIZE

#define  QUEUE_SIZE				128
QueueHandle_t qMHZ19;

#define MHZ19_START_BYTE 		0xff
#define MHZ19_SENSOR_NUM 		0x01
#define MHZ19_READ_CMD 			0x86
#define MHZ19_CALIBRATE_ZERO 	0x87 	//Calibrate Zero Point (ZERO)
#define MHZ19_CALIBRATE_SPAN 	0x88 	//Calibrate Span Point (SPAN)
#define MHZ19_CALIBRATE_AUTO 	0x79 	//ON/OFF Auto Calibration
#define MHZ19_DETECT_RANGE 		0x99 	//Detection range setting

#define MHZ19_CHECKSUM_READ 	0x79

#define MHZ19_START_CMD_LEN		9
#define MHZ19_START_CMD			{ MHZ19_START_BYTE, MHZ19_SENSOR_NUM, MHZ19_READ_CMD, 0x00, 0x00, 0x00, 0x00, 0x00, MHZ19_CHECKSUM_READ }
#define	MHZ19_START_CMD_NUM		0
#define	MHZ19_READ_CMD_NUM		1
#define	MHZ19_H_LVL_NUM			2
#define	MHZ19_L_LVL_NUM			3
#define	MHZ19_CRC_NUM			8

static void mhz19_event_task(void *pvParameters) {
	uart_event_t event;
	uint8_t *dtmp = (uint8_t*) malloc(RX_BUF_SIZE);
	uint8_t i, state = 0, rxByte, checkSum = 0;
	uint16_t result = 0;
	tDisplay display;
	uint8_t convert[DISP_DIGIT_MAX + 1];

	for (;;) {
		if (xQueueReceive(qMHZ19, (void*) &event,
				(portTickType) portMAX_DELAY)) {

			switch (event.type) {
			case UART_DATA:
				ESP_LOGE(TAG, "[UART SIZE]: %d", event.size);
				uart_read_bytes(MHZ19_UART, dtmp, event.size, portMAX_DELAY);
				for (i = 0; i < event.size; i++) {
					ESP_LOGE(TAG, "[UART DATA]: %x", *(dtmp + i));
					rxByte = *(dtmp + i);
					if (state == MHZ19_START_CMD_NUM) {
						if (rxByte == MHZ19_START_BYTE)
							ESP_LOGE(TAG, "start ok");
							state++;
					} else if (state == MHZ19_READ_CMD_NUM) {
						if (rxByte == MHZ19_READ_CMD) {
							ESP_LOGE(TAG, "read ok");
							checkSum = rxByte;
							state++;
						}
					} else if (state == MHZ19_H_LVL_NUM) {
						ESP_LOGE(TAG, "h lvl ok");
						result = (uint16_t) rxByte << 8;
						checkSum += rxByte;
						state++;
					} else if (state == MHZ19_L_LVL_NUM) {
						ESP_LOGE(TAG, "l lvl ok");
						result += (uint16_t) rxByte;
						checkSum += rxByte;
						state++;
					} else if ((state > MHZ19_L_LVL_NUM)
							&& (state < MHZ19_CRC_NUM)) { //Ignore
						state++;
						checkSum += rxByte;
					} else if (state == MHZ19_CRC_NUM) {
						ESP_LOGE(TAG, "mhz19 chk clc: %d %x %x %x", result, rxByte, checkSum, ((0xff - checkSum) + 1));
						if (((0xff - checkSum) + 1) == rxByte) {
							if (result < 9999) {
								sprintf((char*) convert, "%04d", result);
								display.digit[DISP_DIGIT_1] =
										convert[DISP_DIGIT_1];
								display.digit[DISP_DIGIT_2] =
										convert[DISP_DIGIT_2];
								display.digit[DISP_DIGIT_3] =
										convert[DISP_DIGIT_3];
								display.digit[DISP_DIGIT_4] =
										convert[DISP_DIGIT_4];
								Show(display, DISP_DIGIT_ONLY_MASK);
								CayenneChangeInteger(&cayenn_cfg, PARAM_CHANAL_CAYEN, PARAM_NAME_SENSOR, CAYENNE_CO2, result);
							}
						}
						state = 0;
					} else {
						state = 0;
					}
				}
				break;
			case UART_FIFO_OVF:
				ESP_LOGI(MHZ19_UART, "hw fifo overflow");
				break;
			case UART_BUFFER_FULL:
				break;
			case UART_PARITY_ERR:
				break;
			case UART_FRAME_ERR:
				break;
			default:
				ESP_LOGI(MHZ19_UART, "uart event type: %d", event.type);
				break;
			}
		}
	}

}

static void mhz19_start_task(void *pvParameters) {
#define START_CMD_LEN	9
	const uint8_t start[START_CMD_LEN] = MHZ19_START_CMD;
	portTickType xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, 10000);
		uart_write_bytes(MHZ19_UART, (const char*) start, START_CMD_LEN);
	}
}

void mhz19_init(void) {

	uart_config_t uart_config = { .baud_rate = 9600, //
			.data_bits = UART_DATA_8_BITS, //
			.parity = UART_PARITY_DISABLE, //
			.stop_bits = UART_STOP_BITS_1, //
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE //
			};
	uart_param_config(MHZ19_UART, &uart_config);
	if (uart_driver_install(MHZ19_UART, RX_BUF_SIZE, TX_BUF_SIZE, QUEUE_SIZE,
			&qMHZ19, 0) == ESP_OK) {
		xTaskCreate(mhz19_event_task, "mhz19", 2048, NULL, 12, NULL);
		xTaskCreate(mhz19_start_task, "mhz19_strt", 2048, NULL, 12, NULL);
		ESP_LOGE(TAG, "start");
	}
}

