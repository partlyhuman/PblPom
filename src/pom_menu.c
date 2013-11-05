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
    PomMenuTakeLongRests,
    PomMenuLongRestDuration,
    PomMenuItemCount
} PomMenuId; // Aliases for each menu item by index

static SimpleMenuLayer menuLayer;
static SimpleMenuItem menuItems[PomMenuItemCount];
static SimpleMenuSection menuSectionRoot;
static SimpleMenuSection menuSectionsAll[1];

static NumberWindow durationChooserWindow;

/**
 Handler for number window, when you select a value.
 Commits the selected value to the selected setting, and pops the window.
*/
void pomOnNumberSelect(struct NumberWindow *window, void *context) {
    int value = number_window_get_value(window);
    int ticks = value * 60;
    PomMenuId id = simple_menu_layer_get_selected_index(&menuLayer);
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
    switch (id) {
        case PomMenuLanguage:
            if (++s->language >= PomLanguageCount) {
                s->language = 0;
            }
            break;
        
        case PomMenuVibrateWhileWorking:
            s->vibrateWhileWorking = !s->vibrateWhileWorking;
            break;

        case PomMenuTakeLongRests:
            s->takeLongRests = !s->takeLongRests;
            break;
            
        case PomMenuRestDuration:
            number_window_set_value(&durationChooserWindow, s->restTicks/60);
            number_window_set_step_size(&durationChooserWindow, 1);
            number_window_set_label(&durationChooserWindow, POM_TEXT_SETTINGS_REST_DURATION[s->language]);
            window_stack_push((Window*)&durationChooserWindow, true);
            break;
            
        case PomMenuLongRestDuration:
            number_window_set_value(&durationChooserWindow, s->longRestTicks/60);
            number_window_set_step_size(&durationChooserWindow, 1);
            number_window_set_label(&durationChooserWindow, POM_TEXT_SETTINGS_LONG_REST_DURATION[s->language]);
            window_stack_push((Window*)&durationChooserWindow, true);
            break;
            
        case PomMenuWorkDuration:
            number_window_set_value(&durationChooserWindow, s->workTicks/60);
            number_window_set_step_size(&durationChooserWindow, 5);
            number_window_set_label(&durationChooserWindow, POM_TEXT_SETTINGS_WORK_DURATION[s->language]);
            window_stack_push((Window*)&durationChooserWindow, true);
            break;
            
            
        default:
            return;
    }
    pomUpdateMenus();
}

/** Refreshes values expressed in the settings menu. */
void pomUpdateMenus() {
    PomLanguage lang = app.settings.language;
    static char workDurationString[32];
    static char restDurationString[32];
    static char longRestDurationString[32];
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
                LOG(workDurationString);
                snprintf(workDurationString, ARRAY_LENGTH(workDurationString), POM_TEXT_X_MINUTES[lang], app.settings.workTicks / 60);
                LOG(workDurationString);
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
                
            case PomMenuTakeLongRests:
                m->title = POM_TEXT_SETTINGS_TAKE_LONG_RESTS[lang];
                m->subtitle = POM_TEXT_BOOLEAN[app.settings.takeLongRests][lang];
                break;
            
            case PomMenuLongRestDuration:
                m->title = POM_TEXT_SETTINGS_LONG_REST_DURATION[lang];
                snprintf(longRestDurationString, ARRAY_LENGTH(longRestDurationString), POM_TEXT_X_MINUTES[lang], app.settings.longRestTicks / 60);
                m->subtitle = longRestDurationString;
                
            default:
                break;
        }
    }
    menu_layer_reload_data((MenuLayer*)&menuLayer);
}

/** Window load handler for settings window. Called before opening the window. */
void pomOnMenuWindowLoad(struct Window *menuWindowRef) {
    pomUpdateMenus();
}

/** Window unload handler for settings window. */
void pomOnMenuWindowUnload(struct Window *menuWindowRef) {
    CONSOLE("Window poppin'");
    pomSaveCookies();
}

/** Initialize everything needed for settings menus. Called by pomOnInit(). */
void pomInitMenuModule(void *ctx) {
    // setup window with all the settings in it
    window_init(&app.menuWindow, "MENU");
    window_set_fullscreen(&app.menuWindow, true);
    window_set_background_color(&app.menuWindow, GColorWhite);
    window_set_window_handlers(&app.menuWindow, (WindowHandlers){
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
    simple_menu_layer_init(&menuLayer,
                           app.menuWindow.layer.frame,
                           &app.menuWindow,
                           menuSectionsAll,
                           1,
                           NULL);
    layer_add_child(&app.menuWindow.layer, simple_menu_layer_get_layer(&menuLayer));

    // setup number picker window, used when you edit a duration value
    number_window_init(&durationChooserWindow, "Duration", (NumberWindowCallbacks){.selected = pomOnNumberSelect}, NULL);
    number_window_set_min(&durationChooserWindow, 1);
    number_window_set_max(&durationChooserWindow, 60);
    number_window_set_step_size(&durationChooserWindow, 1);
}
