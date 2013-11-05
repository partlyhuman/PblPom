#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "debugging.h"
#include "pom.h"
#include "pom_vibes.h"
#include "pom_text.h"
#include "pom_menu.h"
#include "pom_cookies.h"

// constants
const GSize FULL_SIZE = {144, 168};

// the common, global app structure
PomApplication app;


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
            frame = app.workingTextLayer.layer.frame;
            frame.origin.y = 2;
            layer_set_frame(&app.workingTextLayer.layer, frame);
            frame = app.timeTextLayer.layer.frame;
            frame.origin.y = 45;
            layer_set_frame(&app.timeTextLayer.layer, frame);
            break;
        case PomStateResting:
            frame = app.workingTextLayer.layer.frame;
            frame.origin.y = FULL_SIZE.h - 65 - 2;
            layer_set_frame(&app.workingTextLayer.layer, frame);
            frame = app.timeTextLayer.layer.frame;
            frame.origin.y = FULL_SIZE.h - 20 - 2;
            layer_set_frame(&app.timeTextLayer.layer, frame);
            break;
        default:
            break;
    }
    frame = app.timeTextLayer.layer.frame;
    frame.size.w = (app.state == PomStateReady)? 80 : 30;
    layer_set_frame(&app.timeTextLayer.layer, frame);
}

/** Change the state between ready, working, and resting. */
void pomSetState(PomState newState) {
    app.state = newState;

    switch (newState) {
        case PomStateWorking:
            app.totalTicks = app.ticksRemaining = app.settings.workTicks;
            
            text_layer_set_text(&app.workingTextLayer, POM_TEXT_WORK[app.settings.language]);
            formatTime(gTimeString, app.settings.workTicks);
            text_layer_set_text(&app.timeTextLayer, gTimeString);
            break;

        case PomStateResting:
            app.totalTicks = app.ticksRemaining = app.settings.restTicks;
            if (app.settings.takeLongRests && (app.completedPoms % app.settings.pomsPerLongRest == 0)) {
                app.ticksRemaining = app.settings.longRestTicks;
            }

            text_layer_set_text(&app.workingTextLayer, POM_TEXT_REST[app.settings.language]);
            formatTime(gTimeString, app.settings.restTicks);
            text_layer_set_text(&app.timeTextLayer, gTimeString);
            break;
            
        case PomStateReady:
            layer_set_bounds(&app.inverterLayer.layer, GRectZero);
            text_layer_set_text(&app.workingTextLayer, POM_TEXT_READY[app.settings.language]);
            static char pomCounterString[64];
            snprintf(pomCounterString, ARRAY_LENGTH(pomCounterString), POM_TEXT_POM_COUNTER[app.settings.language], app.completedPoms);
            text_layer_set_text(&app.timeTextLayer, pomCounterString);
            break;

        default:
            WARN("Unhandled state change %d", newState);
            break;
    }

    pomMoveTextLayers();
    layer_mark_dirty(&app.mainWindow.layer);
//    layer_set_frame(&app.inverterLayer.layer, GRectZero);
}

/** Tick handler. Called every second. Also called on the minute for "heartbeat" working reminders. */
void pomOnTick(AppContextRef ctx, PebbleTickEvent *event) {
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
    if (isWorking && app.settings.vibrateWhileWorking && (event->units_changed & MINUTE_UNIT) > 0) {
        vibes_enqueue_custom_pattern(VIBRATE_MINIMAL);
    }
    
    // resize inverter
    float pctRemaining = (app.ticksRemaining + 0.0) / app.totalTicks;
    GRect inverterFrame = GRect(0, 0, FULL_SIZE.w, 0);
    if (isWorking) {
        inverterFrame.size.h = (1.0 - pctRemaining) * FULL_SIZE.h;
    }
    else if (isResting) {
        inverterFrame.size.h = pctRemaining * FULL_SIZE.h;
    }
    layer_set_frame(&app.inverterLayer.layer, inverterFrame);
    
    // set timer text
    formatTime(gTimeString, app.ticksRemaining);
    text_layer_set_text(&app.timeTextLayer, gTimeString);
    
    // redraw!
    layer_mark_dirty(&app.mainWindow.layer);
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
    if (window_stack_contains_window(&app.menuWindow)) {
        WARN("Window already in window stack");
        return;
    }
    window_stack_push(&app.menuWindow, true);
}

/** Set up click handlers on the main window. */
void pomMainWindowClickProvider(ClickConfig **buttonConfigs, void *context) {
    buttonConfigs[BUTTON_ID_UP]->click.handler =
        buttonConfigs[BUTTON_ID_DOWN]->click.handler =
        pomOnMainWindowUpOrDownClick;
    buttonConfigs[BUTTON_ID_SELECT]->click.handler = pomOnMainWindowSelectClick;
}

void pomOnDeinit(AppContextRef ctx) {
}

void pomOnTimer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t timerKey) {
    //Delayed calling of httpebble stuff, it doesn't like to go on init.
    CONSOLE("App timer up.");
    pomLoadCookies();
}

/** App initialization. */
void pomOnInit(AppContextRef ctx) {
    window_init(&app.mainWindow, "Pom");
    window_set_fullscreen(&app.mainWindow, true);
    window_set_background_color(&app.mainWindow, GColorWhite);
    window_set_click_config_provider(&app.mainWindow, pomMainWindowClickProvider);
    
    text_layer_init(&app.workingTextLayer, GRect(2, 2, FULL_SIZE.w, 50));
    text_layer_set_font(&app.workingTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    
    text_layer_init(&app.timeTextLayer, GRect(4, 45, 80, 30));
    text_layer_set_font(&app.timeTextLayer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK));
    
    inverter_layer_init(&app.inverterLayer, GRectZero);
    
    layer_add_child(&app.mainWindow.layer, &app.workingTextLayer.layer);
    layer_add_child(&app.mainWindow.layer, &app.timeTextLayer.layer);
    layer_add_child(&app.mainWindow.layer, &app.inverterLayer.layer);
    window_stack_push(&app.mainWindow, true);
    
#ifdef DEBUG
    __CONSOLE_INIT
    layer_add_child(&app.mainWindow.layer, &__console_layer.layer);
#endif
    
    pomInitMenuModule(ctx);
    pomInitCookiesModule(ctx);
    pomSetState(PomStateReady);
    
    app_timer_send_event(ctx, 1000, 0);
}

//  Pebble Core ------------------------------------------------------------

void pbl_main(void *params) {
    // setup default settings
    // TODO load settings from persistent storage
    app.settings = (PomSettings){
        .language = PomEnglish,
        .workTicks = 60 * 25,
        .restTicks = 60 * 5,
        .longRestTicks = 60 * 15,
        .pomsPerLongRest = 4,
        .takeLongRests = true,
        .vibrateWhileWorking = true,
    };
    
    app.completedPoms = 0;
    
    PebbleAppHandlers handlers = {
        .init_handler = &pomOnInit,
        .deinit_handler = &pomOnDeinit,
        .tick_info = {
            .tick_handler = &pomOnTick,
            .tick_units = SECOND_UNIT|MINUTE_UNIT,
        },
        .timer_handler = &pomOnTimer,
    };
    app_event_loop(params, &handlers);
}

#ifndef XCODE
PBL_APP_INFO(POM_UUID_HTTPEBBLE, POM_NAME, "Partlyhuman", 1, 0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_STANDARD_APP);
#endif
