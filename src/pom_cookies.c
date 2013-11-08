// #include "pom_cookies.h"
// #include "pom.h"
// #include "http.h"
// #include "debugging.h"

// #define COOKIE_KEY_BYTEARRAY 293

// static int32_t requestId;

// void traceHttpResult(HTTPResult result) {
//     if (result != HTTP_OK) {
//         CONSOLE("Error %d", result);
//     }
// }

// void pomOnCookieSet(int32_t request_id, bool successful, void* context) {
//     CONSOLE("Cookie set %s", (successful)? "OK" : "FAIL");
// }

// void pomSaveCookies() {
//     CONSOLE("Saving settings struct: %d bytes", sizeof(PomSettings));
//     void *data = &(app.settings);
//     traceHttpResult(http_cookie_set_data(++requestId, COOKIE_KEY_BYTEARRAY, data, sizeof(PomSettings)));
// }

// void pomOnCookieGet(int32_t request_id, Tuple* t, void* context) {
//     CONSOLE("Cookie received");
//     if (t->key == COOKIE_KEY_BYTEARRAY) {
//         CONSOLE("It's our settings, yay");
//         memcpy(&app.settings, t->value->data, sizeof(PomSettings));
//         pomSetState(app.state);
//     }
// }

// void pomLoadCookies() {
//     CONSOLE("Loading cookies...");
//     traceHttpResult(http_cookie_get(++requestId, COOKIE_KEY_BYTEARRAY));
// }

// void pomOnCookieReconnect(void* context) {
//     CONSOLE("Reconnected to phone");
//     pomSaveCookies();
// }

// void pomInitCookiesModule(void *ctx) {
//     http_set_app_id(0x1b2b3b4b);
//     http_register_callbacks((HTTPCallbacks){
//         .cookie_get = pomOnCookieGet,
//         .cookie_set = pomOnCookieSet,
//         .reconnect = pomOnCookieReconnect,
//     }, ctx);
// }
