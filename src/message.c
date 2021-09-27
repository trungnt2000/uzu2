#include "message.h"
#include "toolbox.h"

static Emitter* s_emitter;

void
ems_init()
{
  s_emitter = emitter_create(MSG_CNT);
}

void
ems_shutdown()
{
  emitter_free(s_emitter);
  s_emitter = NULL;
}

void
ems_broadcastfn(enum MsgId msg_id, const void* data)
{
  emitter_emit(s_emitter, (int)msg_id, data);
}

void
ems_connect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx, u32 flags)
{
  emitter_connect(s_emitter, (int)msg_id, fn, ctx, flags);
}

void
ems_disconnect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx)
{
  emitter_disconnect(s_emitter, (int)msg_id, fn, ctx);
}
