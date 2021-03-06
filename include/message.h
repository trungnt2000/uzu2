// ngotrung Mon 06 Sep 2021 06:04:05 PM +07
#ifndef MESSAGE_H
#define MESSAGE_H
#include "ecs.h"
#include "toolbox.h"

enum MsgId
{
    MsgLevelLoaded,
    MsgLevelUnloaded,
    MsgDialogueClosed,
    MsgDialogueShown,
    MsgEntityDied,
    MsgCommandSelected,
    MsgPlayerHitEnemy,
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

struct MsgPlayerHitEnemy
{
    ecs_entity_t player;
    ecs_entity_t enemy;
};

struct MsgCommandSelected
{
    ecs_entity_t entity;
    const char*  command;
};

void ems_init(void);
void ems_shutdown(void);
void _ems_broadcast(enum MsgId msg_id, const void* data);
void ems_connect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx, u32 flags);
void ems_disconnect(enum MsgId msg_id, bool (*fn)(void*, const void*), void* ctx);

#define ems_broadcast(T, ...) _ems_broadcast(T, &(struct T)__VA_ARGS__)

#endif // MESSAGE_H
