#include "message.h"
#include "toolbox.h"

static MsgBus* s_emitter;

void
ems_init()
{
    s_emitter = mbus_create(MSG_CNT);
}

void
ems_shutdown()
{
    mbus_free(s_emitter);
    s_emitter = NULL;
}

void
_ems_broadcast(enum MsgId msg_id, const void* data)
{
    mbus_emit(s_emitter, (int)msg_id, data);
}

void
ems_connect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx, u32 flags)
{
    mbus_connect(s_emitter, (int)msg_id, fn, ctx, flags);
}

void
ems_disconnect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx)
{
    mbus_disconnect(s_emitter, (int)msg_id, fn, ctx);
}
