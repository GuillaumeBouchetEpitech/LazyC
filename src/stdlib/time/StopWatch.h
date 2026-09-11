
#pragma once

typedef struct StopWatch StopWatch;

StopWatch* StopWatch__create();
void StopWatch__free(StopWatch** self);

void StopWatch__start(StopWatch* self);
void StopWatch__stop(StopWatch* self);
double StopWatch__getTime(const StopWatch* self);

