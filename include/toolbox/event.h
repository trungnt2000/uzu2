#ifndef SIGNAL_H
#define SIGNAL_H
#include "common.h"

typedef struct Event   Event;
typedef struct Emitter Emitter;

typedef void* conn_t;
void          conn_dispose(conn_t* connRef);

typedef struct EventReceiver
{
  void* ctx;
  void (*fn)(void* ctx, const void* data);
} EventReceiver;

#define EVENT_RECEIVER_1(_fn, _ctx)                                            \
  (Slot)                                                                       \
  {                                                                            \
    .fn = (_fn), .ctx = (_ctx)                                                 \
  }
#define EVENT_RECEIVER_2(_fn) EVENT_RECEIVER_1(_fn, NULL)

Event* event_create(void);
void   event_free(Event* evt);

conn_t event_subscribe(Event* sig, EventReceiver receiver);
void   event_publish(Event* evt, const void* arg);

Emitter* emitter_create(u32 numSingals);
void     emitter_free(Emitter* emitter);
conn_t   emitter_subscribe(Emitter* emitter, u32 sig, EventReceiver receiver);
void     emitter_publish(Emitter* emitter, u32 sig, const void* arg);

#endif // SIGNAL_H
