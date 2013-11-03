#pragma once
#include "pebble_os.h"

#define POM_UUID { 0x42, 0xB2, 0x76, 0xD7, 0x8F, 0x6F, 0x44, 0x9B, 0x89, 0x03, 0xD1, 0x56, 0xC0, 0xD1, 0xE9, 0xBD }
#define POM_NAME "Pom"

typedef enum {
    PomStateWorking,
    PomStateResting,
    PomStatePaused
} PomState;

typedef struct {
    unsigned int language;
    unsigned int workTicks;
    unsigned int restTicks;
    bool vibrateWhileWorking;
} PomSettings;


typedef struct {

    PomSettings settings;
    
    unsigned int completedPoms;
    int ticksRemaining;
    int ticks;
    PomState state;
    
    Window window;
    TextLayer workingTextLayer;
    TextLayer timeTextLayer;
    InverterLayer inverterLayer;

} PomApplication;
