// ************************************* //
// * Arduino Project RFLink32        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef CREDENTIALS_h
#define CREDENTIALS_h

#include "RFLink.h"

// MQTT Server
#define  MQTT_SERVER "192.168.0.55"
#define  MQTT_PORT 1883
#define  MQTT_ID "RFLink32"
#define  MQTT_USER "addons"
#define  MQTT_PSWD "xxx"

// MQTT Topic
#define MQTT_TOPIC_OUT "rflink/msg"
#define MQTT_TOPIC_IN "rflink/cmd"
#define MQTT_TOPIC_LWT "rflink/lwt"

// OTA
#define AutoOTA_URL "http://domain.com/firmware.bin"

#ifdef CHECK_CACERT
static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIID (...)
-----END CERTIFICATE-----
)EOF";
#endif //CHECK_CACERT

#endif
