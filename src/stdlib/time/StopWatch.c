
#include "./StopWatch.h"

#include "../core/panic.h"

// #include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

typedef struct StopWatch {

  struct timespec start;
  struct timespec stop;

} StopWatch;

#define BILLION  1000000000L;


StopWatch* StopWatch__create()
{
  StopWatch* self = calloc(1, sizeof(StopWatch));
  return self;
}

void StopWatch__free(StopWatch** self)
{
  if (!self || !*self) {
    return;
  }
  free(*self);
  *self = NULL;
}

void StopWatch__start(StopWatch* self)
{
  if (clock_gettime(CLOCK_REALTIME, &self->start) == -1) {
    // perror( "clock gettime" );
    panic("clock gettime");
    // return EXIT_FAILURE;
  }
}

void StopWatch__stop(StopWatch* self)
{
  if (clock_gettime(CLOCK_REALTIME, &self->stop) == -1) {
    // perror( "clock gettime" );
    panic("clock gettime");
    // return EXIT_FAILURE;
  }
}

double StopWatch__getTime(const StopWatch* self)
{
  return ( self->stop.tv_sec - self->start.tv_sec ) + (double)( self->stop.tv_nsec - self->start.tv_nsec ) / (double)BILLION;
}

