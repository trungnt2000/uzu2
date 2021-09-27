#ifndef TOOLBOX_SIGNAL_H
#define TOOLBOX_SIGNAL_H
#include "toolbox/common.h"

typedef struct Signal  Signal;
typedef struct Emitter Emitter;

#define SLOT_FLAG_ONESHOT (1u)

Signal* signal_create(void);
void    signal_free(Signal* signal);

void signal_connect(Signal* signal, bool (*func)(void*, const void*), void* ctx, u32 flags);
bool signal_disconnect(Signal* signal, bool (*func)(void*, const void*), void* ctx);
void signal_emit(Signal* signal, const void* data);

Emitter* emitter_create(u32 numSingals);
void     emitter_free(Emitter* emitter);
void     emitter_connect(Emitter* emitter, int sig, bool (*func)(void*, const void*), void* ctx, u32 flags);
void     emitter_disconnect(Emitter* emitter, int sig, bool (*func)(void*, const void*), void* ctx);
void     emitter_emit(Emitter* emitter, int sig, const void* data);

#endif // TOOLBOX_SIGNAL_H
