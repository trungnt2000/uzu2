// ngotrung Mon 06 Sep 2021 06:04:05 PM +07
#ifndef MESSAGE_H
#define MESSAGE_H
#include "toolbox.h"

enum MsgId
{
    MsgLevelLoaded,
    MsgLevelUnloaded,
    MsgDialogueClosed,
    MsgDialogueShown,
    MsgEntityDied,
    MSG_CNT
};

struct MsgLevelLoaded
{
    const char* level_name;
};

struct MsgLevelUnloaded
{
    const char* level_name;
};

void ems_init(void);
void ems_shutdown(void);
void ems_broadcastfn(enum MsgId msg_id, const void* data);
void ems_connect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx, u32 flags);
void ems_disconnect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx);

/* wrapper macro */
#define ems_broadcast(T, ...) ems_broadcastfn(T, &(struct T)__VA_ARGS__)

#endif // MESSAGE_H
