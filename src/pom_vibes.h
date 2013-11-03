#pragma once
#include "pebble_os.h"

static const uint32_t VIBRATE_PAT_MINIMAL[] = {30,};
static const VibePattern VIBRATE_MINIMAL = {
    .durations = VIBRATE_PAT_MINIMAL,
    .num_segments = ARRAY_LENGTH(VIBRATE_PAT_MINIMAL),
};

static const uint32_t VIBRATE_PAT_DIT_DIT_DAH[] = {100, 200, 100, 200, 500, 150};
static const VibePattern VIBRATE_DIT_DIT_DAH = {
    .durations = VIBRATE_PAT_DIT_DIT_DAH,
    .num_segments = ARRAY_LENGTH(VIBRATE_PAT_DIT_DIT_DAH),
};
