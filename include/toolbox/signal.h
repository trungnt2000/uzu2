#ifndef TOOLBOX_SIGNAL_H
#define TOOLBOX_SIGNAL_H
#include "toolbox/common.h"

typedef struct Signal Signal;
typedef struct MsgBus MsgBus;

#define SLOT_FLAG_ONESHOT (1u)

Signal* signal_create(void);
void    signal_free(Signal* signal);

void signal_connect(Signal* signal, bool (*func)(void*, const void*), void* ctx, u32 flags);
bool signal_disconnect(Signal* signal, bool (*func)(void*, const void*), void* ctx);
void signal_emit(Signal* signal, const void* data);

MsgBus* mbus_create(u32 numSingals);
void    mbus_free(MsgBus* emitter);
void    mbus_connect(MsgBus* emitter, int sig, bool (*func)(void*, const void*), void* ctx, u32 flags);
void    mbus_disconnect(MsgBus* emitter, int sig, bool (*func)(void*, const void*), void* ctx);
void    mbus_emit(MsgBus* emitter, int sig, const void* data);

#endif // TOOLBOX_SIGNAL_H
