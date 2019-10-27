/*
 * ��������� ���� ��������� html-���������� � ���� ������ ��� �� �����
 * ��������������� � ������ � �������.
 */

#ifndef MAIN_PARAMS_H_
#define MAIN_PARAMS_H_

#include <stddef.h>
#include "esp_err.h"
#include "version.h"

//define method save
enum paramType_t{
  PARAM_TYPE_NONE = -1,	//no save to nvs
  PARAM_TYPE_WIFI = 0,
  PARAM_TYPE_CAEN = 1,
  PARAM_TYPE_OTA = 2
};

//params possible
enum param_Num_t{
  PARAM_SSID_NAME_NUM = 0,
  PARAM_PASWRD_NUM,
  PARAM_MQTT_HOST_NUM,
  PARAM_MQTT_PORT_NUM,
  PARAM_MQTT_USER_NUM,
  PARAM_MQTT_PASS_NUM,
  PARAM_MQTT_CLIENT_ID_NUM,
  PARAM_MQTT_MODEL_NAME_NUM,
  PARAM_APP_VER,							//������ ���������� � ���� ������
  PARAM_OTA_SERVER_IP						//IP http ������� � ����������� ��������, ���� ������ 80
};

#define	PARAM_CHANAL_CAYEN			0				//����� ������ ������������� ����������, ������������ ������� cayenn
#define PARAM_NAME_SENSOR			"CO2normalLess1000ppm"			//��� ������� ��� ������
#define PARAM_CHANAL_LED_UPDATE		3				//����������
#define PARAM_NAME_LED_UPDATE		"LedUpdate"
#define PARAM_NAME_VERSION			"version"		//������ ����������

typedef struct{
  uint8_t pos;
  enum paramType_t paramType;
} seechRecord_t;

/*
 * �������� ������� ��������� ���� ����������
 * ���������� ��������� ESP_OK ���� ������ �� ������
 * � ������������ �������� ������� ������� ������� � ������ ����������
 */
esp_err_t getFirstVarName(seechRecord_t* sr);
/*
 * ����� ��������� ������� ������ ����������
 * ���������� ��������� �� ��� ���������� ��� null ���� ��������� ����� ������.
 * ����������� ������� ������� �� ���� �������
 */
char* getNextVarName(seechRecord_t* sr);

/*
 * ���������� �������� ��������� ��������� � ������� sr ������ ����������
 * �� ��������
 */
esp_err_t saveValue(seechRecord_t sr, char* value);

/*
 * ��������� � ������ toStr �������� ���������� valName ���� ������.
 * ����������� lenVal �� ���������� ���������� ��������
 * ���������� ��������� �� ����� ���������� ������
 */
char* putsValue(char* toStr, char* varName, size_t *lenVal);

#endif /* MAIN_PARAMS_H_ */
