#include "pom.h"
#include "pom_menu.h"
#include "pom_text.h"

typedef enum {
    PomMenuLanguage = 0,
    PomMenuWorkDuration,
    PomMenuRestDuration,
    PomMenuVibrateWhileWorking,
    PomMenuItemCount
} PomMenuId;

static SimpleMenuLayer menuLayer;
static SimpleMenuItem menuItems[PomMenuItemCount];
static SimpleMenuSection menuSectionRoot;
static SimpleMenuSection menuSectionsAll[1];

void pomOnMenuSelect(int index, void *context) {
    LOG("CLICK on %d", index);
    PomSettings *s = &app.settings;
    PomMenuId id = index;
    switch (id) {
        case PomMenuLanguage:
            LOG("Changing language! Old language=%d", s->language);
            if (++s->language >= PomLanguageCount) {
                s->language = 0;
            }
            LOG("New language=%d", s->language);
            break;
        
        case PomMenuVibrateWhileWorking:
            s->vibrateWhileWorking = !s->vibrateWhileWorking;
            break;
            
        default:
            return;
    }
    pomUpdateMenus();
}

void pomUpdateMenus() {
    PomLanguage lang = app.settings.language;
    static char workDurationString[32];
    static char restDurationString[32];
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
            default: break;
        }
    }
    menu_layer_reload_data((MenuLayer*)&menuLayer);
}

void pomBuildMenus() {
    for (PomMenuId id = 0; id < PomMenuItemCount; id++) {
        menuItems[id].callback = pomOnMenuSelect;
    }
    
    menuSectionRoot.items = menuItems;
    menuSectionRoot.num_items = PomMenuItemCount;
    menuSectionRoot.title = "SETTINGS";
    menuSectionsAll[0] = menuSectionRoot;
}

void pomOnMenuWindowLoad(struct Window *menuWindowRef) {
    LOG("LOading window");
    pomUpdateMenus();
    LOG("Loading window complete");
}

void pomOnMenuWindowUnload(struct Window *menuWindowRef) {
    LOG("Unloading menu window");
}


void pomInitMenus() {
    LOG("Initializing menu");
    window_init(&app.menuWindow, "MENU");
    window_set_fullscreen(&app.menuWindow, true);
    window_set_background_color(&app.menuWindow, GColorWhite);
    window_set_window_handlers(&app.menuWindow, (WindowHandlers){
        .load = pomOnMenuWindowLoad,
        .unload = pomOnMenuWindowUnload,
    });
    
    pomBuildMenus();
    
    simple_menu_layer_init(&menuLayer,
                           app.menuWindow.layer.frame,
                           &app.menuWindow,
                           menuSectionsAll,
                           1,
                           NULL);
    layer_add_child(&app.menuWindow.layer, simple_menu_layer_get_layer(&menuLayer));

    LOG("DONE INITIALIZING");
}
