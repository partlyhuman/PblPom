#pragma once
#include "pebble_os.h"

#define MY_UUID { 0x42, 0xB2, 0x76, 0xD7, 0x8F, 0x6F, 0x44, 0x9B, 0x89, 0x03, 0xD1, 0x56, 0xC0, 0xD1, 0xE9, 0xBD }
#ifndef XCODE
    PBL_APP_INFO_SIMPLE(MY_UUID, "HelloWorld", "Partlyhuman", 1);
#endif

typedef struct {

    unsigned int language;
    unsigned int workTicks;
    unsigned int restTicks;
    bool vibrateWhileWorking;

} HelloWorldApplicationSettings;

typedef struct {

    HelloWorldApplicationSettings settings;
    
    Window window;
    TextLayer workingTextLayer;
    TextLayer timeTextLayer;
    InverterLayer inverterLayer;
    
    unsigned int ticks;

} HelloWorldApplication;
