/**
Proyecto realizado por Emilio Jimenez Prieto
*/

/*
 * dialogos_json.h
 *
 *  Created on: 11 sept. 2020
 *      Author: t126401
 */

#ifndef COMPONENTS_COMUNES_INCLUDE_DIALOGOS_JSON_H_
#define COMPONENTS_COMUNES_INCLUDE_DIALOGOS_JSON_H_

#define MQTT_PARAMETER  "mqttParam"
#define MQTT_BROKER     "mqttBroker"
#define MQTT_PORT       "mqttPort"
#define MQTT_USER       "mqttUser"
#define MQTT_PASS       "mqttPass"
#define MQTT_PREFIX     "mqttPrefix"
#define MQTT_PUBLISH    "mqttPublish"
#define MQTT_SUBSCRIBE  "mqttSubscribe"
#define MQTT_QOS		"mqttQos"
#define MQTT_CERT_TLS	"CERT_TLS"
#define MQTT_TLS		"mqttTls"
#define FREE_MEMORY     "freeMem"
#define UPTIME          "uptime"
#define MAP_MEMORY      "memoryMap"
#define RTCTIME         "rtcTime"
#define ADRESS_BIN      "adressBin"


#define YEAR        "year"
#define MONTH       "month"
#define DAY         "day"
#define HOUR        "hour"
#define MINUTE      "minute"
#define SECOND      "second"
#define WEEKDAY     "weekDay"
#define TIME_VALID  "timeValid"
#define TIME_T      "time_t"
#define NTP_TIMEZONE   "timeZone"
#define NTP_SUMMER_TIME "summerTime"
#define NTP_SERVER_ACTIVE   "ntpServerActive"
#define NTP_SERVER  "ntpServer"
#define SPARES_NTP  "sparesNtp"
#define SPARE       "spare"
#define SPARE_INDEX "spareIndex"
#define CLOCK_STATE "clock_state"
#define NTP_TIME_EXP    "ntpTimeExp"
#define NTP_TIMEOUT "ntpTimeout"
#define NTP_SYNC    "ntpSync"
#define DATE        "date"
#define CLOCK       "clock"
#define DEVICE_STATE   "deviceState"
#define DEVICE          "device"
#define PROGRAM         "program"
#define PROGRAM_ID      "programId"
#define CURRENT_PROGRAM_ID "currentProgramId"
#define ON_GOING            "onGoing"
#define NEXT_PROGRAM_ID "nextProgramId"
#define NEW_PROGRAM_ID  "newProgramId"
#define PROGRAM_TYPE    "programType"
#define PROGRAM_STATE   "programState"
#define PROGRAM_TIME    "programTime"
#define PROGRAMMER_STATE  "programmerState"
#define NUMBER_PROGRAMS   "programNumbers"
#define PROGRAM_ACTION    "estadoRele"
#define PROGRAM_MASK    "programMask"
#define DURATION_PROGRAM    "durationProgram"
#define DESCARGA_OTA        "upgradeOta"
#define CODIGO_OTA          "otaCode"

#define COMANDO             "comando"
#define DLG_COMANDO         "dlgComando"
#define DLG_FECHA           "dlgFecha"
#define DLG_KEY             "token"
#define DLG_PARAMETRO       "dlgParametro"
#define DLG_RESPUESTA       "dlgRespuesta"
#define DLG_GRABAR_CONF     "dlgGrabarConf"
#define DLG_COD_RESPUESTA   "dlgResultCode"
#define DLG_OBSERVACIONES   "dlgObservaciones"
#define DLG_OK              "ok"
#define DLG_NOK             "nok"
#define DLG_OK_CODE     200
#define DLG_NOK_CODE     400
#define UPGRADE_FIRMWARE    "upgradeFirmware"
#define OTA_SERVER          "otaServer"
#define OTA_PORT            "otaPort"
#define OTA_URL             "otaUrl"
#define OTA_FILE            "otaFile"
#define OTA_SW_VERSION      "otaVersion"
#define FIN_UPGRADE			"finUpgrade"
#define ID_DEVICE           "idDevice"

#define APP_COMAND_RELE                 "rele"
#define APP_COMAND_OPERACION_RELE       "opRele"
#define APP_COMAND_ESTADO_RELE          "estadoRele"

#define REINTENTOS_LECTURA        "reintentosLectura"
#define INTERVALO_REINTENTOS           "intervaloReintentos"
#define INTERVALO_LECTURA         "intervaloLectura"
#define MARGEN_TEMPERATURA          "margenTemperatura"
#define RELE                        "rele"
#define TEMPERATURA                 "temperatura"
#define HUMEDAD                     "humedad"
#define UMBRAL_TEMPERATURA          "temperaturaUmbral"
#define UMBRAL_SIGUIENTE            "temperaturaUmbralSig"
#define FECHA_ULTIMA_LECTURA        "ultimaLectura"
#define REPORTE_LECTURA             "reporteLectura"
#define UMBRAL_DEFECTO              "umbralDefecto"
#define MODO                        "modoFuncionamiento"
#define LECTURA                     "lectura"
#define CALIBRAR_SENSOR             "calibrar"
#define CALIBRADO                   "valorCalibrado"

#define CONF_WIFI           "confWifi"
#define CONF_WIFI_ESSID     "essidWifi"
#define CONF_WIFI_PASS      "passwordWifi"


//#define ESTADO_ALARMA				"estadoAlarma"
#define FECHA_ALARMA				"fechaAlarma"

#define TIPO_REPORT                 "tipoReport"

#endif /* COMPONENTS_COMUNES_INCLUDE_DIALOGOS_JSON_H_ */
