#include "esp_system.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "ota_client.h"
#include "cayenne.h"
#include "hc595.h"
#include "mhz19.h"

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

	nvs_flash_init();

	//rtc_clk_cpu_freq_set(RTC_CPU_FREQ_160M);

	InitDisplay();
	mhz19_init();
    wifi_init_param();
    ota_init();
    wifi_init(WIFI_MODE_STA);
    Cayenne_Init();
}

