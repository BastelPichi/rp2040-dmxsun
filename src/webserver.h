#ifndef WEBSERVER_H
#define WEBSERVER_H

/**
 * @file
*/

#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

extern "C" {
  #include <b64/cencode.h>
  #include <b64/cdecode.h>
}

#include "tusb_lwip_glue.h"

#include "version.h"

#include <string>
#include <map>

#ifdef __cplusplus

class WebServer {
  public:
    void init();
    void cyclicTask();
    static inline void ipToString(uint32_t ip, char* ipString);
    static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen);

    static inline void paramsToMap(int iNumParams, char *pcParam[], char *pcValue[], std::map<std::string, std::string>* params);
    static std::string urlDecode(std::string &input);

    static base64_encodestate b64Encode;
    static base64_decodestate b64Decode;
    static uint8_t tmpBuf[800];
    static uint8_t tmpBuf2[800];

  private:
};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

static const char *cgi_system_reset_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_statusLeds_brightness_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_dmxBuffer_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_ioBoards_config(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_load(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_save(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_enable(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_disable(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_wireless_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_partyMode_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);


static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen);


#ifdef __cplusplus
}
#endif

#endif // WEBSERVER_H
