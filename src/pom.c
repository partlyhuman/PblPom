#include <pebble.h>
#include <pebble_fonts.h>
#include "debugging.h"
#include "pom.h"
#include "pom_vibes.h"
#include "pom_text.h"
#include "pom_menu.h"
#include "pom_cookies.h"

// the common, global app structure
PomApplication app;

GSize windowSize = {144, 168};

// Utilities --------------------------------------------------------------

static char gTimeString[6]; // shared time format buffer
/** Formats a time into MM:SS, e.g. 04:50 */
inline void formatTime(char* str, int seconds) {
    if (seconds < 0) seconds = 0;
    snprintf(str, 6, "%02d:%02d", seconds / 60, seconds % 60);
}

//  Application  ------------------------------------------------------------

/** Move layers to the appropriate positions. Helper for pomSetState(). */
void pomMoveTextLayers() {
    GRect frame;
    switch (app.state) {
        case PomStateWorking:
        case PomStateReady:
            frame = layer_get_frame(text_layer_get_layer(app.workingTextLayer));
            frame.origin.y = 2;
            layer_set_frame(text_layer_get_layer(app.workingTextLayer), frame);
            frame = layer_get_frame(text_layer_get_layer(app.timeTextLayer));
            frame.origin.y = 45;
            layer_set_frame(text_layer_get_layer(app.timeTextLayer), frame);
            break;
        case PomStateResting:
            frame = layer_get_frame(text_layer_get_layer(app.workingTextLayer));
            frame.origin.y = windowSize.h - 65 - 2;
            layer_set_frame(text_layer_get_layer(app.workingTextLayer), frame);
            frame = layer_get_frame(text_layer_get_layer(app.timeTextLayer));
            frame.origin.y = windowSize.h - 20 - 2;
            layer_set_frame(text_layer_get_layer(app.timeTextLayer), frame);
            break;
        default:
            break;
    }
    frame = layer_get_frame(text_layer_get_layer(app.timeTextLayer));
    frame.size.w = (app.state == PomStateReady)? 80 : 30;
    layer_set_frame(text_layer_get_layer(app.timeTextLayer), frame);
}

/** Change the state between ready, working, and resting. */
void pomSetState(PomState newState) {
    app.state = newState;
    
    // recalculate window size
    window_set_fullscreen(app.mainWindow, !app.settings.showClock);
    layer_mark_dirty(window_get_root_layer(app.mainWindow));
    windowSize = layer_get_frame(window_get_root_layer(app.mainWindow)).size;
    
    switch (newState) {
        case PomStateWorking:
            app.totalTicks = app.ticksRemaining = app.settings.workTicks;
            
            text_layer_set_text(app.workingTextLayer, POM_TEXT_WORK[app.settings.language]);
            formatTime(gTimeString, app.settings.workTicks);
            text_layer_set_text(app.timeTextLayer, gTimeString);
            break;

        case PomStateResting:
            app.totalTicks = app.ticksRemaining = app.settings.restTicks;
            if (app.settings.takeLongRests && (app.completedPoms % app.settings.pomsPerLongRest == 0)) {
                app.ticksRemaining = app.settings.longRestTicks;
            }

            text_layer_set_text(app.workingTextLayer, POM_TEXT_REST[app.settings.language]);
            formatTime(gTimeString, app.settings.restTicks);
            text_layer_set_text(app.timeTextLayer, gTimeString);
            break;
            
        case PomStateReady:
            layer_set_bounds(inverter_layer_get_layer(app.inverterLayer), GRectZero);
            text_layer_set_text(app.workingTextLayer, POM_TEXT_READY[app.settings.language]);
            static char pomCounterString[64];
            snprintf(pomCounterString, ARRAY_LENGTH(pomCounterString), POM_TEXT_POM_COUNTER[app.settings.language], app.completedPoms);
            text_layer_set_text(app.timeTextLayer, pomCounterString);
            break;

        default:
            WARN("Unhandled state change %d", newState);
            break;
    }
    
    pomMoveTextLayers();
    layer_mark_dirty(window_get_root_layer(app.mainWindow));
}

/** Tick handler. Called every second. Also called on the minute for "heartbeat" working reminders. */
void pomOnTick(struct tm *tick_time, TimeUnits units_changed) {
    if (app.state == PomStateReady) return;
    app.ticksRemaining--;
    
    // check for time up
    // Note: because this returns, you have an extra second to see the change over. feature, not a bug.
    if (app.ticksRemaining < 0) {
        if (app.state == PomStateWorking) {
            // you finished the pomodoro! congrats!
            app.completedPoms++;
            vibes_short_pulse();
            light_enable_interaction();
            pomSetState(PomStateResting);
            return;
        }
        else if (app.state == PomStateResting) {
            // time to start another pomodoro.
            vibes_enqueue_custom_pattern(VIBRATE_DIT_DIT_DAH);
            light_enable_interaction();
            pomSetState(PomStateReady);
            return;
        }
    }
    
    bool isWorking = (app.state == PomStateWorking);
    bool isResting = (app.state == PomStateResting);

    // heartbeat
    if (isWorking && app.settings.vibrateWhileWorking && (units_changed & MINUTE_UNIT) > 0) {
        vibes_enqueue_custom_pattern(VIBRATE_MINIMAL);
    }
    
    // resize inverter
    float pctRemaining = (app.ticksRemaining + 0.0) / app.totalTicks;
    GRect inverterFrame = GRect(0, 0, windowSize.w, 0);
    if (isWorking) {
        inverterFrame.size.h = (1.0 - pctRemaining) * windowSize.h;
    }
    else if (isResting) {
        inverterFrame.size.h = pctRemaining * windowSize.h;
    }
    layer_set_frame(inverter_layer_get_layer(app.inverterLayer), inverterFrame);
    
    // set timer text
    formatTime(gTimeString, app.ticksRemaining);
    text_layer_set_text(app.timeTextLayer, gTimeString);
    
    // redraw!
    layer_mark_dirty(window_get_root_layer(app.mainWindow));
}

/** Handles up or down button click while in main window. Use this click to start or restart a cycle. */
void pomOnMainWindowUpOrDownClick(ClickRecognizerRef recognizer, void *context) {
    if (app.state == PomStateReady) {
        pomSetState(PomStateWorking);
    } else {
        pomSetState(PomStateReady);
    }
}

/** Select (middle button) click handler. Launches into settings menu. */
void pomOnMainWindowSelectClick(ClickRecognizerRef recognizer, void *context) {
    if (window_stack_contains_window(app.menuWindow)) {
        WARN("Window already in window stack");
        return;
    }
    window_stack_push(app.menuWindow, true);
}

/** Set up click handlers on the main window. */
void pomMainWindowClickProvider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, pomOnMainWindowUpOrDownClick);
    window_single_click_subscribe(BUTTON_ID_DOWN, pomOnMainWindowUpOrDownClick);
    window_single_click_subscribe(BUTTON_ID_SELECT, pomOnMainWindowSelectClick);
}

/** App initialization. */
void pomStartup() {
    // setup default settings
    // after, load settings from persistent storage
    app.settings = (PomSettings){
        .language = PomEnglish,
        .workTicks = 60 * 25,
        .restTicks = 60 * 5,
        .longRestTicks = 60 * 15,
        .pomsPerLongRest = 4,
        .takeLongRests = true,
        .vibrateWhileWorking = true,
        .showClock = false,
    };
    
    app.completedPoms = 0;

    app.mainWindow = window_create();
    window_set_background_color(app.mainWindow, GColorWhite);
    window_set_click_config_provider(app.mainWindow, pomMainWindowClickProvider);
    
    app.workingTextLayer = text_layer_create(GRect(2, 2, windowSize.w, 50));
    text_layer_set_font(app.workingTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_background_color(app.workingTextLayer, GColorClear);
    
    app.timeTextLayer = text_layer_create(GRect(4, 45, 80, 30));
    text_layer_set_font(app.timeTextLayer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK));
    text_layer_set_background_color(app.timeTextLayer, GColorClear);
    
    app.inverterLayer = inverter_layer_create(GRectZero);
    
    layer_add_child(window_get_root_layer(app.mainWindow), text_layer_get_layer(app.workingTextLayer));
    layer_add_child(window_get_root_layer(app.mainWindow), text_layer_get_layer(app.timeTextLayer));
    layer_add_child(window_get_root_layer(app.mainWindow), inverter_layer_get_layer(app.inverterLayer));

#if USE_CONSOLE
    __console_layer = text_layer_create(GRect(0, 28, 144, 140));
    text_layer_set_overflow_mode(__console_layer, GTextOverflowModeWordWrap);
    text_layer_set_font(__console_layer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK));
    layer_add_child(window_get_root_layer(app.mainWindow), text_layer_get_layer(__console_layer.layer));
#endif

    pomInitMenuModule();
    pomInitCookiesModule();

    tick_timer_service_subscribe(SECOND_UNIT|MINUTE_UNIT, pomOnTick);
    pomLoadCookies();

    pomSetState(PomStateReady);
    window_stack_push(app.mainWindow, true);
}

void pomShutdown() {
    pomSaveCookies();
    window_destroy(app.mainWindow);
    window_destroy(app.menuWindow);
}

//  Pebble Core ------------------------------------------------------------

int main() {
    pomStartup();
    app_event_loop();
    pomShutdown();
}
