#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "hello_world.h"
#include "text.h"

static const GSize FULL_SIZE = {144, 168};
static const uint32_t VIBRATE_WORKING_REMINDER_MS = 50;

static HelloWorldApplication app;

void vibrateFor(uint32_t ms) {
    static VibePattern pat;
    static uint32_t durations[1];
    durations[0] = ms;
    pat.num_segments = 1;
    pat.durations = durations;
    vibes_enqueue_custom_pattern(pat);
}

static char gTimeString[6];
void formatTime(char* str, int seconds) {
    if (seconds < 0) seconds = 0;
    snprintf(str, 6, "%02d:%02d", seconds / 60, seconds % 60);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *event) {
    app.ticks++;
    
    if (app.settings.vibrateWhileWorking && (event->units_changed & MINUTE_UNIT) > 0) {
        light_enable_interaction();
        vibrateFor(VIBRATE_WORKING_REMINDER_MS);
    }
    
    float pct = (app.ticks + 0.0) / app.settings.workTicks;
    if (pct > 1.0f) pct = 1.0f;

    inverter_layer_init(&app.inverterLayer, GRect(0, 0, FULL_SIZE.w, FULL_SIZE.h * pct));
    formatTime(gTimeString, app.settings.workTicks - app.ticks);
    text_layer_set_text(&app.timeTextLayer, gTimeString);
    layer_mark_dirty(&app.window.layer);
    
    //    static char str[12];
    //    snprintf(str, 12, "pct=%0.3f", pct);
    //    app_log(APP_LOG_LEVEL_INFO, "hello_world.c", 40, str);
}

void handle_init(AppContextRef ctx) {
    window_init(&app.window, "Window Name");
    window_set_fullscreen(&app.window, true);
    window_set_background_color(&app.window, GColorWhite);
    
    text_layer_init(&app.workingTextLayer, GRect(2, 2, FULL_SIZE.w, 45));
    text_layer_set_font(&app.workingTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text(&app.workingTextLayer, text_working[app.settings.language]);
    
    text_layer_init(&app.timeTextLayer, GRect(4, 45, FULL_SIZE.w, 30));
    text_layer_set_font(&app.timeTextLayer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK));
    
    layer_add_child(&app.window.layer, &app.workingTextLayer.layer);
    layer_add_child(&app.window.layer, &app.timeTextLayer.layer);
    layer_add_child(&app.window.layer, &app.inverterLayer.layer);
    window_stack_push(&app.window, true);
}

void pbl_main(void *params) {
    app.settings = (HelloWorldApplicationSettings){
        .language = LanguageEnglish,
        .workTicks = 60 * 2,
        .restTicks = 60 * 1,
        .vibrateWhileWorking = true,
    };
    
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .tick_info = {
            .tick_handler = &handle_tick,
            .tick_units = SECOND_UNIT|MINUTE_UNIT,
        }
    };
    app_event_loop(params, &handlers);
}
