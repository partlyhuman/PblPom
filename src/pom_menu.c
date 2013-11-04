#include "pom.h"
#include "pom_menu.h"
#include "pom_text.h"

static SimpleMenuLayer menuLayer;
static const SimpleMenuItem menuSectionRootItems[2] = {
    {
        .title = "Test1",
        .subtitle = "Subtitle1",
        .callback = pomOnMenuSelect,
    },
    {
        .title = "Test2",
        .subtitle = "Subtitle2",
        .callback = pomOnMenuSelect,
    }
};
static const SimpleMenuSection menuSectionRoot = {
    .title = "Settings",
    .items = menuSectionRootItems,
    .num_items = ARRAY_LENGTH(menuSectionRootItems),
};
static SimpleMenuSection menuSectionsAll[1];

void pomOnMenuSelect(int index, void *context) {
    LOG("CLICK!");
}

void pomOnMenuWindowLoad(struct Window *menuWindowRef) {
    LOG("LOading window");
    LOG("Loading window complete");
}

void pomOnMenuWindowUnload(struct Window *menuWindowRef) {
    LOG("Unloading menu window");
}

void pomInitMenus() {
    LOG("Initializing menu");
    window_init(&app.menuWindow, "MENU");
    window_set_fullscreen(&app.menuWindow, false);
    window_set_background_color(&app.menuWindow, GColorWhite);
    window_set_window_handlers(&app.menuWindow, (WindowHandlers){
        .load = pomOnMenuWindowLoad,
        .unload = pomOnMenuWindowUnload,
    });

    menuSectionsAll[0] = menuSectionRoot;
    
    simple_menu_layer_init(&menuLayer,
                           app.menuWindow.layer.frame,
                           &app.menuWindow,
                           menuSectionsAll,
                           1,
                           NULL);
    layer_add_child(&app.menuWindow.layer, simple_menu_layer_get_layer(&menuLayer));

    LOG("DONE INITIALIZING");
}
