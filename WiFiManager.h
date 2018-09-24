/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>

extern "C" {
  #include "user_interface.h"
}

const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/> <title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>html, body{margin: 0; display: flex; min-width: 320px; min-height: 100vh; flex-direction: column; align-items: center; justify-content: center; background-color: #FAFAFF; font-family: 'Titillium Web', 'Calibri', 'Roboto', 'DejaVu Sans', 'Helvetica', 'sans-serif'; text-align: center;}a{font-weight: 700; color: #32281E;}.c{text-align: center;}.i{margin: 0 auto 20px; display: block;}.d{margin-bottom: 1.5rem; font-size: 1.2rem; text-align: center;}.n{margin-bottom: 1.5rem; font-size: 1.2rem; font-weight: 200;}dt{font-weight: 700;}dd{margin-bottom: 1rem; margin-inline-start: 144px;}div, input{padding: 5px; font-size: 1em;}input{margin-bottom: 10px; border: none; border-bottom: solid 1px #B3B3E6; width: 100%; height: 42px; background-color: transparent; font-weight: 200; color: #32281E; outline: none;}button{border: 0; width: 100%; padding: 0.6rem 1rem; background-color: #B3B3E6; font-size: 1rem; font-weight: 700; color: #fff; text-transform: uppercase;}.q{float: right; width: 64px; font-size: 1rem; font-weight: 200; text-align: right;}.l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center; background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText || l.textContent; document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body> <div style='text-align:left;display:inline-block;min-width:260px;'> <img class=\"i\" src=\"data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMjUxIiBoZWlnaHQ9IjUyIiB2aWV3Qm94PSIwIDAgMjUxIDUyIiBmaWxsPSJub25lIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciPgo8cGF0aCBkPSJNMjMuNDg4IDIzLjU4NEMxOS44NCAyMS44NTYgMTYuMjU2IDIxLjg1NiAxNS45MzYgMjEuODU2QzEyLjY3MiAyMS44NTYgOS41MzYgMTkuOTM2IDkuNTM2IDE2LjIyNEM5LjUzNiAxMi4xMjggMTIuNDE2IDEwLjAxNiAxNS45MzYgMTAuMDE2QzE5LjI2NCAxMC4wMTYgMjIuMDggMTEuNzQ0IDIyLjIwOCAxNS4zOTJDMjIuMjA4IDE1LjU4NCAyMi41MjggMTUuOTA0IDIyLjkxMiAxNS45MDRIMzAuOTEyQzMxLjM2IDE1LjkwNCAzMS42OCAxNS41ODQgMzEuNjE2IDE1LjM5MkMzMS40ODggNi42ODggMjQuNzA0IDAuNzk5OTk5IDE1LjkzNiAwLjc5OTk5OUM3Ljc0NCAwLjc5OTk5OSAwIDYuNjI0IDAgMTYuMjI0QzAgMjIuMTc2IDMuMDA4IDI2LjcyIDguMzIgMjkuMTUyQzEyLjAzMiAzMC44OCAxNS40ODggMzAuODggMTUuOTM2IDMwLjg4QzE4LjgxNiAzMS4wMDggMjIuMzM2IDMzLjMxMiAyMi4zMzYgMzcuMDg4QzIyLjMzNiA0MC40OCAxOS4zMjggNDIuNzg0IDE1LjkzNiA0Mi43ODRDMTUuOTM2IDQyLjc4NCAxNS45MzYgNDIuNzg0IDE1Ljg3MiA0Mi43ODRWNTJDMTUuOTM2IDUyIDE1LjkzNiA1MiAxNS45MzYgNTJDMjQuMTI4IDUyIDMxLjgwOCA0Ni42ODggMzEuODA4IDM3LjA4OEMzMS44MDggMzEuMDcyIDI4Ljg2NCAyNi4wOCAyMy40ODggMjMuNTg0Wk05Ni4yMjE1IDEwLjIwOEw4OS42OTM1IDI3LjgwOEw3OS41ODE1IDEuMTg0Qzc5LjQ1MzUgMC45Mjc5OTggNzkuMTk3NSAwLjc5OTk5OSA3OC45NDE1IDAuNzk5OTk5SDcwLjIzNzVDNjkuOTgxNSAwLjc5OTk5OSA2OS43MjU1IDAuOTI3OTk4IDY5LjY2MTUgMS4xODRMNTkuNDg1NSAyNy44MDhMNTMuMDIxNSAxMC4yMDhINDMuMDM3NUw1OC45MDk1IDUxLjYxNkM1OS4xMDE1IDUyLjEyOCA1OS45MzM1IDUyLjEyOCA2MC4xMjU1IDUxLjYxNkw3NC41ODk1IDEzLjY2NEw4OS4wNTM1IDUxLjYxNkM4OS4yNDU1IDUyLjEyOCA5MC4wNzc1IDUyLjEyOCA5MC4yNjk1IDUxLjYxNkwxMDYuMjA2IDEwLjIwOEg5Ni4yMjE1Wk0xNDkuNTQyIDUxLjIzMkwxMzAuMDg2IDEuMTg0QzEyOS44OTQgMC42NzE5OTcgMTI4Ljk5OCAwLjY3MTk5NyAxMjguODA2IDEuMTg0TDEwOS40MTQgNTEuMjMyQzEwOS4yMjIgNTEuNjE2IDEwOS40NzggNTIgMTEwLjA1NCA1MkgxMTguODIyQzExOS4wNzggNTIgMTE5LjM5OCA1MS44NzIgMTE5LjQ2MiA1MS42MTZMMTI5LjQ0NiAyMy4wNzJMMTM5LjU1OCA1MS42MTZDMTM5LjYyMiA1MS44NzIgMTM5Ljk0MiA1MiAxNDAuMTM0IDUySDE0OC45NjZDMTQ5LjQxNCA1MiAxNDkuNzM0IDUxLjYxNiAxNDkuNTQyIDUxLjIzMlpNMTk2LjE0NCA1MS4xNjhDMTkyLjk0NCA0NC4wNjQgMTg5LjQ4OCAzNy4wMjQgMTg2LjI4OCAyOS43OTJDMTkxLjQwOCAyNy45MzYgMTk1LjY5NiAyMy44NCAxOTUuNjk2IDE2LjAzMkMxOTUuNjk2IDAuOTI3OTk4IDE4MS4yOTYgMC43OTk5OTkgMTc4LjQxNiAwLjc5OTk5OUgxNjEuODRDMTYxLjUyIDAuNzk5OTk5IDE2MS4yIDEuMTIgMTYxLjIgMS4zNzZWMTAuMDhIMTcwLjkyOEgxNzguNDE2QzE3OS45NTIgMTAuMDggMTg1Ljk2OCAxMC40IDE4NS45NjggMTYuMDMyQzE4NS45NjggMjEuNjY0IDE4MC41MjggMjEuOTIgMTc4LjQxNiAyMS45MkgxNzAuOTI4SDE2MS4yVjUxLjQyNEMxNjEuMiA1MS42OCAxNjEuNTIgNTIgMTYxLjg0IDUySDE3MC4yMjRDMTcwLjYwOCA1MiAxNzAuOTI4IDUxLjY4IDE3MC45MjggNTEuNDI0VjMxLjMyOEgxNzYuNDMyTDE4NS43NzYgNTEuNjE2QzE4NS44NCA1MS44MDggMTg2LjIyNCA1MiAxODYuNDE2IDUySDE5NS41NjhDMTk2LjA4IDUyIDE5Ni4zMzYgNTEuNjE2IDE5Ni4xNDQgNTEuMTY4Wk0yNDkuOTggMC43OTk5OTlIMjQxLjcyNEMyNDEuMjc2IDAuNzk5OTk5IDI0MC45NTYgMS4xMiAyNDAuOTU2IDEuMzc2VjM0LjcyTDIxNy4zNCAwLjk5MTk5N0MyMTcuMjc2IDAuODYzOTk4IDIxNi44OTIgMC43OTk5OTkgMjE2Ljc2NCAwLjc5OTk5OUgyMDguMjUyQzIwNy45MzIgMC43OTk5OTkgMjA3LjYxMiAxLjEyIDIwNy42MTIgMS4zNzZWMzkuNzEySDIxNy40MDRWMTguMDhMMjQwLjk1NiA1MS44MDhDMjQxLjAyIDUxLjkzNiAyNDEuNDA0IDUyIDI0MS41MzIgNTJIMjQ5Ljk4QzI1MC40MjggNTIgMjUwLjY4NCA1MS42OCAyNTAuNjg0IDUxLjQyNFYxLjM3NkMyNTAuNjg0IDEuMTIgMjUwLjQyOCAwLjc5OTk5OSAyNDkuOTggMC43OTk5OTlaIiBmaWxsPSJ1cmwoI3BhaW50MF9saW5lYXIpIi8+CjxkZWZzPgo8bGluZWFyR3JhZGllbnQgaWQ9InBhaW50MF9saW5lYXIiIHgxPSI2LjAxODU1ZS0wNyIgeTE9IjIwIiB4Mj0iMjYzIiB5Mj0iMjAiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIj4KPHN0b3Agc3RvcC1jb2xvcj0iI0IzQjNFNiIvPgo8c3RvcCBvZmZzZXQ9IjEiIHN0b3AtY29sb3I9IiM3Njc2Q0MiLz4KPC9saW5lYXJHcmFkaWVudD4KPC9kZWZzPgo8L3N2Zz4K\" alt=\"SWARN\"> <div class=\"d\">SWARN - Sensor Dashboard</div>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"> <button>Configure WiFi</button> </form> <br/> <form action=\"/0wifi\" method=\"get\"> <button>Configure WiFi (No Scan)</button> </form> <br/> <form action=\"/i\" method=\"get\"> <button>Info</button> </form> <br/> <form action=\"/r\" method=\"post\"> <button>Reset</button> </form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

#ifndef WIFI_MANAGER_MAX_PARAMS
#define WIFI_MANAGER_MAX_PARAMS 10
#endif

class WiFiManagerParameter {
  public:
    /** 
        Create custom parameters that can be added to the WiFiManager setup web page
        @id is used for HTTP queries and must not contain spaces nor other special characters
    */
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    ~WiFiManagerParameter();

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();
    ~WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //useful for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, useful if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter, returns false on failure
    bool          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsuccessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    int                    _max_params;
    WiFiManagerParameter** _params;

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
