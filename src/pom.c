#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "pom.h"
#include "pom_vibes.h"
#include "pom_text.h"

// constants
static const GSize FULL_SIZE = {144, 168};
static const uint32_t VIBRATE_WORKING_REMINDER_MS = 50;

// Global app handle
static PomApplication app;

// Utilities --------------------------------------------------------------

/** Vibrates a custom duration, just one pulse */
void vibrateFor(uint32_t ms) {
    static VibePattern pat;
    static uint32_t durations[1];
    durations[0] = ms;
    pat.num_segments = 1;
    pat.durations = durations;
    vibes_enqueue_custom_pattern(pat);
}

static char gTimeString[6]; // shared time format buffer
/** Formats a time into MM:SS, e.g. 04:50 */
inline void formatTime(char* str, int seconds) {
    if (seconds < 0) seconds = 0;
    snprintf(str, 6, "%02d:%02d", seconds / 60, seconds % 60);
}

static char gLog[256]; // shared logging buffer
#define LOG(...) snprintf(gLog, ARRAY_LENGTH(gLog), __VA_ARGS__); app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, gLog)
#define WARN(...) snprintf(gLog, ARRAY_LENGTH(gLog), __VA_ARGS__); app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, gLog)
#define ERROR(...) snprintf(gLog, ARRAY_LENGTH(gLog), __VA_ARGS__); app_log(APP_LOG_LEVEL_ERROR, __FILE__, __LINE__, gLog)

//  Application  ------------------------------------------------------------

void pomSetState(PomState newState) {
    app.state = newState;
    GRect frame;

    switch (newState) {
        case PomStateWorking:
            app.ticks = 0;
            app.ticksRemaining = app.settings.workTicks;
            
            text_layer_set_text(&app.workingTextLayer, POM_TEXT_WORK[app.settings.language]);
            formatTime(gTimeString, app.settings.workTicks);
            text_layer_set_text(&app.timeTextLayer, gTimeString);

            frame = app.workingTextLayer.layer.frame;
            frame.origin.y = 2;
            layer_set_frame(&app.workingTextLayer.layer, frame);
            frame = app.timeTextLayer.layer.frame;
            frame.origin.y = 45;
            layer_set_frame(&app.timeTextLayer.layer, frame);

            break;

        case PomStateResting:
            app.ticks = 0;
            app.ticksRemaining = app.settings.restTicks;

            text_layer_set_text(&app.workingTextLayer, POM_TEXT_REST[app.settings.language]);
            formatTime(gTimeString, app.settings.restTicks);
            text_layer_set_text(&app.timeTextLayer, gTimeString);
            
            frame = app.workingTextLayer.layer.frame;
            frame.origin.y = FULL_SIZE.h - 65 - 2;
            layer_set_frame(&app.workingTextLayer.layer, frame);
            frame = app.timeTextLayer.layer.frame;
            frame.origin.y = FULL_SIZE.h - 20 - 2;
            layer_set_frame(&app.timeTextLayer.layer, frame);
            
            break;

        default:
            WARN("Unhandled state change %d", newState);
            break;
    }
//    layer_mark_dirty(&app.window.layer);
//    layer_set_frame(&app.inverterLayer.layer, GRectZero);
}

void pomOnTick(AppContextRef ctx, PebbleTickEvent *event) {
    if (app.state == PomStatePaused) return;

    app.ticks++;
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
            pomSetState(PomStateWorking);
            return;
        }
    }
    
    bool isWorking = (app.state == PomStateWorking);
    bool isResting = (app.state == PomStateResting);

    // heartbeat
    if (isWorking && app.settings.vibrateWhileWorking && (event->units_changed & MINUTE_UNIT) > 0) {
        light_enable_interaction();
        vibes_enqueue_custom_pattern(VIBRATE_MINIMAL);
    }
    
    int totalTicks = (isWorking)? app.settings.workTicks : app.settings.restTicks;
    float pct = (app.ticks + 0.0) / totalTicks;

    // resize inverter
    GRect inverterFrame = GRect(0, 0, FULL_SIZE.w, 0);
    if (isWorking) {
        inverterFrame.size.h = pct * FULL_SIZE.h;
    }
    else if (isResting) {
        inverterFrame.size.h = (1.0 - pct) * FULL_SIZE.h;
    }
    if (inverterFrame.size.h >= FULL_SIZE.h) inverterFrame.size.h = FULL_SIZE.h;
    if (inverterFrame.size.h <= 0) inverterFrame.size.h = 0;
    layer_set_frame(&app.inverterLayer.layer, inverterFrame);
    
    // set timer text
    formatTime(gTimeString, app.ticksRemaining);
    text_layer_set_text(&app.timeTextLayer, gTimeString);
    
    // redraw!
    layer_mark_dirty(&app.window.layer);
}

void pomOnInit(AppContextRef ctx) {
    vibes_enqueue_custom_pattern(VIBRATE_MINIMAL);
    window_init(&app.window, "Pom");
    window_set_fullscreen(&app.window, true);
    window_set_background_color(&app.window, GColorWhite);
    
    text_layer_init(&app.workingTextLayer, GRect(2, 2, FULL_SIZE.w, 45));
    text_layer_set_font(&app.workingTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    
    text_layer_init(&app.timeTextLayer, GRect(4, 45, FULL_SIZE.w, 30));
    text_layer_set_font(&app.timeTextLayer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK));
    
    inverter_layer_init(&app.inverterLayer, GRectZero);
    
    layer_add_child(&app.window.layer, &app.workingTextLayer.layer);
    layer_add_child(&app.window.layer, &app.timeTextLayer.layer);
    layer_add_child(&app.window.layer, &app.inverterLayer.layer);
    window_stack_push(&app.window, true);
    
    pomSetState(PomStateWorking);
}

//  Pebble Core ------------------------------------------------------------

void pbl_main(void *params) {
    // setup default settings
    // TODO load settings from persistent storage
    app.settings = (PomSettings){
        .language = PomEnglish,
        .workTicks = 60 * 0.2,
        .restTicks = 60 * 0.2,
        .vibrateWhileWorking = true,
    };
    
    PebbleAppHandlers handlers = {
        .init_handler = &pomOnInit,
        .tick_info = {
            .tick_handler = &pomOnTick,
            .tick_units = SECOND_UNIT|MINUTE_UNIT,
        }
    };
    app_event_loop(params, &handlers);
}

#ifndef XCODE
PBL_APP_INFO_SIMPLE(POM_UUID, POM_NAME, "Partlyhuman", 1);
#endif
