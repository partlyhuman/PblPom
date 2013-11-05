#include "pom_cookies.h"
#include "pom.h"
#include "http.h"
#include "debugging.h"

#define COOKIE_KEY_BYTEARRAY 292
//enum {
//    CookieKeyLanguage = 0,
//    CookieKeyWorkDuration,
//    CookieKeyRestDuration,
//    CookieKeyCount
//};


static int32_t requestId;
//static uint32_t cookieKeys[CookieKeyCount];
//static uint8_t cookieValues[CookieKeyCount];
//static uint16_t *cookieValueBindings[CookieKeyCount];


void traceHttpResult(HTTPResult result) {
    if (result == HTTP_OK) {
        CONSOLE("OK");

    
    } else {
        CONSOLE("Error %d", result);
    }
}

//static int cookieSaveStep;
//void pomSaveOneCookie() {
//    if (cookieSaveStep >= CookieKeyCount) return;
//    HTTPResult result = http_cookie_set_int16(++requestId, (uint32_t)cookieSaveStep, *cookieValueBindings[cookieSaveStep]);
//    if (result == HTTP_OK) {
//        CONSOLE("Save cookie OK.");
//    } else {
//        CONSOLE("Save error %d", result);
//    }
//}
//
void pomOnCookieSet(int32_t request_id, bool successful, void* context) {
    CONSOLE("Cookie set %s", (successful)? "OK" : "FAIL");
//    ++cookieSaveStep;
//    pomSaveOneCookie();
}
//
//void pomSaveCookies() {
//    CONSOLE("Saving cookies...");
//    cookieSaveStep = 0;
//    pomSaveOneCookie();
//}

void pomSaveCookies() {
    CONSOLE("Saving one big field, should be 14 bytes, is %d", sizeof(PomSettings));
    void *data = &(app.settings);
    traceHttpResult(http_cookie_set_data(++requestId, COOKIE_KEY_BYTEARRAY, data, sizeof(PomSettings)));
}

void pomOnCookieGet(int32_t request_id, Tuple* t, void* context) {
    CONSOLE("Cookie received");
    if (t->key == COOKIE_KEY_BYTEARRAY) {
        CONSOLE("It's our settings, yay");
        memcpy(&app.settings, t->value->data, sizeof(PomSettings));
    }
}

void pomLoadCookies() {
//    CONSOLE("SKIP LOADING for now");
    CONSOLE("Loading cookies...");
    traceHttpResult(http_cookie_get(++requestId, COOKIE_KEY_BYTEARRAY));

//    if (result != HTTP_OK) {
//        CONSOLE("Load error %d", result);
//    }
}

void pomOnCookieReconnect(void* context) {
    CONSOLE("Reconnected to phone");
}

void pomInitCookiesModule(void *ctx) {
//    cookieValueBindings[CookieKeyLanguage] = &app.settings.language;
//    cookieValueBindings[CookieKeyWorkDuration] = &app.settings.workTicks;
//    cookieValueBindings[CookieKeyRestDuration] = &app.settings.restTicks;
//    
//    for (size_t i = 0; i < CookieKeyCount; i++) {
//        cookieKeys[i] = (int32_t)i;
//    }
    
    http_set_app_id(0x1b2b3b4b);
    http_register_callbacks((HTTPCallbacks){
        .cookie_get = pomOnCookieGet,
        .cookie_set = pomOnCookieSet,
        .reconnect = pomOnCookieReconnect,
    }, ctx);
}
