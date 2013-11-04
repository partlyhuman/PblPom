#pragma once

typedef enum {
    PomEnglish = 0,
    PomSpanish,
    PomLanguageCount
} PomLanguage;

extern const char* POM_TEXT_WORK[];
extern const char* POM_TEXT_REST[];
extern const char* POM_TEXT_READY[];
extern const char* POM_TEXT_POM_COUNTER[];

extern const char** POM_TEXT_BOOLEAN[];
extern const char* POM_TEXT_YES[];
extern const char* POM_TEXT_NO[];
extern const char* POM_TEXT_X_MINUTES[];

extern const char* POM_TEXT_SETTINGS_LANGUAGE[];
extern const char* POM_TEXT_SETTINGS_LANGUAGES[];
extern const char* POM_TEXT_SETTINGS_WORK_DURATION[];
extern const char* POM_TEXT_SETTINGS_REST_DURATION[];
extern const char* POM_TEXT_SETTINGS_VIBRATE_WHILE_WORKING[];
