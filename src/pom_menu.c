#include "pom.h"
#include "pom_menu.h"
#include "pom_text.h"
#include "pom_cookies.h"
#include "debugging.h"

typedef enum {
    PomMenuLanguage = 0,
    PomMenuWorkDuration,
    PomMenuRestDuration,
    PomMenuVibrateWhileWorking,
    PomMenuVibrateFrequency,
    PomMenuTakeLongRests,
    PomMenuLongRestDuration,
    PomMenuLongRestRatio,
    PomMenuShowClock,
    PomMenuAutoContinue,
    PomMenuItemCount
} PomMenuId; // Aliases for each menu item by index

static SimpleMenuLayer *menuLayer;
static SimpleMenuItem menuItems[PomMenuItemCount];
static SimpleMenuSection menuSectionRoot;
static SimpleMenuSection menuSectionsAll[1];

static NumberWindow *numberChooserWindow;

static const int VIBRATE_TICK_OPTIONS[] = {1, 2, 5, 10, 15, 30, 60, 120};

static int indexOf(int toFind, const int array[], uint16_t length) {
    for(uint16_t i = 0; i < length; i++) {
        if (toFind == array[i]) {
            return i;
        }
    }
    return -1;
}

/**
 Handler for number window, when you select a value.
 Commits the selected value to the selected setting, and pops the window.
*/
void pomOnNumberSelect(struct NumberWindow *window, void *context) {
    int value = number_window_get_value(window);
    int ticks = value * 60;
    PomMenuId id = simple_menu_layer_get_selected_index(menuLayer);
        switch (id) {
        case PomMenuRestDuration:
            app.settings.restTicks = ticks;
            break;
        case PomMenuWorkDuration:
            app.settings.workTicks = ticks;
            break;
        case PomMenuLongRestDuration:
            app.settings.longRestTicks = ticks;
            break;
        case PomMenuLongRestRatio:
            app.settings.pomsPerLongRest = value;
            break;
        default:
            return;
    }
    window_stack_pop(true);
    pomUpdateMenus();
}

/**
 Common handler attached to every menu item. Called when you click the select (middle) button on the menu item.
 Either switches the value inline (e.g. for booleans), or opens a numeric stepper window (e.g. for durations).
*/
void pomOnMenuSelect(int index, void *context) {
    PomSettings *s = &app.settings;
    PomMenuId id = index;
    int temp;
    switch (id) {
        case PomMenuLanguage:
            if (++s->language >= PomLanguageCount) {
                s->language = 0;
            }
            break;
        
        case PomMenuVibrateFrequency:
            temp = indexOf(s->vibrateTicks, VIBRATE_TICK_OPTIONS, ARRAY_LENGTH(VIBRATE_TICK_OPTIONS));
            temp = (temp + 1) % ARRAY_LENGTH(VIBRATE_TICK_OPTIONS);
            s->vibrateTicks = VIBRATE_TICK_OPTIONS[temp];
            break;
            
        //boolean toggles

        case PomMenuVibrateWhileWorking:
            s->vibrateWhileWorking = !s->vibrateWhileWorking;
            break;
    
        case PomMenuTakeLongRests:
            s->takeLongRests = !s->takeLongRests;
            break;
            
        case PomMenuShowClock:
            s->showClock = !s->showClock;
            break;

        case PomMenuAutoContinue:
            s->autoContinue = !s->autoContinue;
            break;
            
        //number pickers

        case PomMenuRestDuration:
            number_window_set_value(numberChooserWindow, s->restTicks/60);
            number_window_set_label(numberChooserWindow, POM_TEXT_SETTINGS_REST_DURATION[s->language]);
            number_window_set_min(numberChooserWindow, 1);
            number_window_set_max(numberChooserWindow, 60);
            window_stack_push((Window *)numberChooserWindow, true);
            break;
            
        case PomMenuLongRestDuration:
            number_window_set_value(numberChooserWindow, s->longRestTicks/60);
            number_window_set_label(numberChooserWindow, POM_TEXT_SETTINGS_LONG_REST_DURATION[s->language]);
            number_window_set_min(numberChooserWindow, 1);
            number_window_set_max(numberChooserWindow, 120);
            window_stack_push((Window *)numberChooserWindow, true);
            break;
            
        case PomMenuWorkDuration:
            number_window_set_value(numberChooserWindow, s->workTicks/60);
            number_window_set_label(numberChooserWindow, POM_TEXT_SETTINGS_WORK_DURATION[s->language]);
            number_window_set_min(numberChooserWindow, 1);
            number_window_set_max(numberChooserWindow, 120);
            window_stack_push((Window *)numberChooserWindow, true);
            break;
        
        case PomMenuLongRestRatio:
            number_window_set_value(numberChooserWindow, s->pomsPerLongRest);
            number_window_set_label(numberChooserWindow, POM_TEXT_SETTINGS_LONG_REST_RATIO[s->language]);
            number_window_set_min(numberChooserWindow, 2);
            number_window_set_max(numberChooserWindow, 10);
            window_stack_push((Window *)numberChooserWindow, true);
            break;
            
        default:
            return;
    }
    pomUpdateMenus();
}

/** Refreshes values expressed in the settings menu. */
void pomUpdateMenus() {
    PomLanguage lang = app.settings.language;
    static char workDurationString[16];
    static char restDurationString[16];
    static char longRestDurationString[16];
    static char longRestRatioString[16];
    static char tickFrequencyString[16];
    SimpleMenuItem *m;
    for (PomMenuId id = 0; id < PomMenuItemCount; id++) {
        m = &menuItems[id];
        switch (id) {
            case PomMenuLanguage:
                m->title = POM_TEXT_SETTINGS_LANGUAGE[lang];
                m->subtitle = POM_TEXT_SETTINGS_LANGUAGES[lang];
                break;
                
            case PomMenuWorkDuration:
                m->title = POM_TEXT_SETTINGS_WORK_DURATION[lang];
                snprintf(workDurationString, ARRAY_LENGTH(workDurationString), POM_TEXT_X_MINUTES[lang], app.settings.workTicks / 60);
                m->subtitle = workDurationString;
                break;
                
            case PomMenuRestDuration:
                m->title = POM_TEXT_SETTINGS_REST_DURATION[lang];
                snprintf(restDurationString, ARRAY_LENGTH(restDurationString), POM_TEXT_X_MINUTES[lang], app.settings.restTicks / 60);
                m->subtitle = restDurationString;
                break;
                
            case PomMenuVibrateWhileWorking:
                m->title = POM_TEXT_SETTINGS_VIBRATE_WHILE_WORKING[lang];
                m->subtitle = POM_TEXT_BOOLEAN[app.settings.vibrateWhileWorking][lang];
                break;
                
            case PomMenuVibrateFrequency:
                m->title = POM_TEXT_SETTINGS_VIBRATE_FREQUENCY[lang];
                snprintf(tickFrequencyString, ARRAY_LENGTH(tickFrequencyString), POM_TEXT_X_SECONDS[lang], app.settings.vibrateTicks);
                m->subtitle = tickFrequencyString;
                break;
                
            case PomMenuTakeLongRests:
                m->title = POM_TEXT_SETTINGS_TAKE_LONG_RESTS[lang];
                m->subtitle = POM_TEXT_BOOLEAN[app.settings.takeLongRests][lang];
                break;
            
            case PomMenuLongRestDuration:
                m->title = POM_TEXT_SETTINGS_LONG_REST_DURATION[lang];
                snprintf(longRestDurationString, ARRAY_LENGTH(longRestDurationString), POM_TEXT_X_MINUTES[lang], app.settings.longRestTicks / 60);
                m->subtitle = longRestDurationString;
                break;

            case PomMenuLongRestRatio:
                m->title = POM_TEXT_SETTINGS_LONG_REST_RATIO[lang];
                snprintf(longRestRatioString, ARRAY_LENGTH(longRestRatioString), "%d", app.settings.pomsPerLongRest);
                m->subtitle = longRestRatioString;
                break;
                
            case PomMenuShowClock:
                m->title = POM_TEXT_SETTINGS_SHOW_CLOCK[lang];
                m->subtitle = POM_TEXT_BOOLEAN[app.settings.showClock][lang];
                break;
                
            case PomMenuAutoContinue:
                m->title = POM_TEXT_SETTINGS_AUTO_CONTINUE[lang];
                m->subtitle = POM_TEXT_BOOLEAN[app.settings.autoContinue][lang];
                break;
                
            default:
                break;
        }
    }
    menu_layer_reload_data((MenuLayer *)menuLayer);
}

/** Window load handler for settings window. Called before opening the window. */
void pomOnMenuWindowLoad(struct Window *menuWindowRef) {
    pomUpdateMenus();
}

/** Window unload handler for settings window. */
void pomOnMenuWindowUnload(struct Window *menuWindowRef) {
    pomSetState(app.state); //redraw in case language changed
}

/** Initialize everything needed for settings menus. Called by pomOnInit(). */
void pomInitMenuModule() {
    // setup window with all the settings in it
    app.menuWindow = window_create();
    window_set_fullscreen(app.menuWindow, true);
    window_set_background_color(app.menuWindow, GColorWhite);
    window_set_window_handlers(app.menuWindow, (WindowHandlers){
        .load = pomOnMenuWindowLoad,
        .unload = pomOnMenuWindowUnload,
    });
    
    // setup menu items, sections, and menu
    for (PomMenuId id = 0; id < PomMenuItemCount; id++) {
        menuItems[id].callback = pomOnMenuSelect;
        // all other properties will be set in pomUpdateMenus()
    }
    menuSectionRoot.items = menuItems;
    menuSectionRoot.num_items = PomMenuItemCount;
    menuSectionRoot.title = "SETTINGS";
    menuSectionsAll[0] = menuSectionRoot;
    
    // setup menu layer
    menuLayer = simple_menu_layer_create(
        layer_get_frame(window_get_root_layer(app.menuWindow)),
        app.menuWindow,
        menuSectionsAll,
        1,
        NULL);
    layer_add_child(window_get_root_layer(app.menuWindow), simple_menu_layer_get_layer(menuLayer));

    // setup number picker window, used when you edit a duration value
    numberChooserWindow = number_window_create("Duration", (NumberWindowCallbacks){.selected = pomOnNumberSelect}, NULL);
    number_window_set_step_size(numberChooserWindow, 1);
}
