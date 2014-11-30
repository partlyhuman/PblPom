#pragma once
#include <pebble.h>

typedef enum {
    PomStateReady,
    PomStateWorking,
    PomStateResting,
} PomState;

typedef struct {
    uint16_t language;
    uint16_t workTicks;
    uint16_t restTicks;
    uint16_t longRestTicks;
    uint16_t pomsPerLongRest;
    uint16_t vibrateTicks;
    bool takeLongRests;
    bool vibrateWhileWorking;
    bool showClock;
    bool autoAdvance;
    bool annoyAfterRestExceeded;
} PomSettings;


typedef struct {

    PomSettings settings;
    
    unsigned int completedPoms;
    int ticksRemaining;
    int totalTicks;
    PomState state;
    
    Window *mainWindow;
    Window *menuWindow;
    TextLayer *workingTextLayer;
    TextLayer *timeTextLayer;
    InverterLayer *inverterLayer;

} PomApplication;


void pomSetState(PomState newState);

//the shared, global, application structure
extern PomApplication app;
