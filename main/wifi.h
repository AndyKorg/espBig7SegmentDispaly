/*
 * wifi.h
 *
 *  Created on: 2 июн. 2019 г.
 *      Author: Administrator
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include <string.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "esp_wifi_types.h"

#define WIFI_SETTING_POSSEBLE
#ifdef WIFI_SETTING_POSSEBLE
#define WIFI_MODE_IN			GPIO_Pin_12				//Pin for select mode wifi
#define AP_STATE				(gpio_get_level(WIFI_MODE_IN_NUM))
#define WIFI_MODE_IN_NUM		GPIO_NUM_12
#define WIFI_MODE_OUT			GPIO_Pin_14				//Этот пин прижать к земле, используется как земля для WIFI_MODE_IN
#define WIFI_MODE_OUT_NUM		GPIO_NUM_14
#endif

#define AP_ON					0
#define AP_OFF					1
#ifndef AP_STATE
#define AP_STATE				AP_OFF
#endif
#define AP_IsOn()				(AP_STATE == AP_ON)
#define AP_IsOff()				(AP_STATE == AP_OFF)

#define AP_SSID					"mh-z19"
#define AP_PASS					""						//Если пусто, то без пароля

#define OTA_CHECK_PERIOD_MIN	(24*60)					//Period checked update application, minute

extern wifi_sta_config_t wifi_sta_param;

void wifi_init_param(void);								//Прочитать параметры wifi
void wifi_init(wifi_mode_t mode);
bool wifi_isOn();										//wifi включен
bool wifi_AP_isOn();									//AP включен, действительно тольео если wifi_isOn() != 0
bool wifi_ap_count_client();							//количество клиентов подключенных к ap


#endif /* MAIN_WIFI_H_ */
