#include "toolbox/event.h"
#include "SDL.h"
#define SIGNAL_DEFAULT_CAPACITY 16

typedef struct _Node
{
  EventReceiver receiver;
  struct _Node* next;
  struct _Node* prev;
  Event*        event;
  BOOL          isRemoved;
} _Node;

struct Event
{
  BOOL   isEmiting;
  BOOL   hasRemovedSlot;
  _Node* head;
};

struct Emitter
{
  u32   numEvents;
  Event events[];
};

static _Node*
node_create(EventReceiver receiver)
{
  _Node* node    = SDL_malloc(sizeof(_Node));
  node->next     = NULL;
  node->prev     = NULL;
  node->receiver = receiver;
  node->event    = NULL;
  return node;
}

static void
node_free(_Node* node)
{
  SDL_free(node);
}

static Event*
event_init(Event* event)
{
  event->hasRemovedSlot = UZU_FALSE;
  event->isEmiting      = UZU_FALSE;
  event->head           = NULL;
  return event;
}

static void
event_fini(Event* event)
{
  _Node *next, *receiver;
  receiver = event->head;
  while (receiver)
  {
    next = receiver->next;
    node_free(receiver);
    receiver = next;
  }
}

conn_t
event_subscribe(Event* event, EventReceiver receiver)
{
  _Node* node = node_create(receiver);
  node->event = event;
  if (event->head == NULL)
  {
    event->head = node;
  }
  else
  {
    node->next        = event->head;
    event->head->prev = node;
  }
  return node;
}

static void
event_remove_slot(Event* event, _Node* receiver)
{
  if (event->head == receiver)
  {
    event->head       = event->head->next;
    event->head->prev = NULL;
  }
  else
  {
    receiver->prev->next = receiver->next;
    receiver->next->prev = receiver->prev;
  }
  node_free(receiver);
}

void
conn_dispose(conn_t* connRef)
{
  _Node* receiver = *connRef;
  if (receiver != NULL)
  {
    if (receiver->event->isEmiting)
    {
      receiver->isRemoved             = UZU_TRUE;
      receiver->event->hasRemovedSlot = UZU_TRUE;
    }
    else
    {
      event_remove_slot(receiver->event, receiver);
    }
  }
  *connRef = NULL;
}

void
event_publish(Event* event, const void* data)
{
  event->isEmiting = UZU_TRUE;
  for (_Node* node = event->head; node != NULL; node = node->next)
  {
    node->receiver.fn(node->receiver.ctx, data);
  }
  event->isEmiting = UZU_FALSE;
  if (event->hasRemovedSlot)
  {
    _Node* node = event->head;
    _Node* next;
    while (node)
    {
      next = node->next;
      if (node->isRemoved)
        event_remove_slot(event, node);
      node = next;
    }
  }
  event->hasRemovedSlot = UZU_FALSE;
}

Event*
event_create(void)
{
  return event_init(SDL_malloc(sizeof(Event)));
}

void
event_free(Event* evt)
{
  event_fini(evt);
  SDL_free(evt);
}

Emitter*
emitter_create(u32 numEvents)
{
  Emitter* emitter   = SDL_malloc(sizeof(Emitter) + sizeof(Event) * numEvents);
  emitter->numEvents = numEvents;
  for (u32 i = 0; i < numEvents; ++i)
    event_init(&(emitter->events[i]));
  return emitter;
}

void
emitter_free(Emitter* emitter)
{
  for (u32 i = 0; i < emitter->numEvents; ++i)
    event_fini(&(emitter->events[i]));
  SDL_free(emitter);
}

conn_t
emitter_subscribe(Emitter* emitter, u32 event, EventReceiver receiver)
{
  ASSERT_MSG(event < emitter->numEvents, "invalid event id");
  return event_subscribe(&(emitter->events[event]), receiver);
}

void
emitter_publish(Emitter* emitter, u32 event, const void* arg)
{
  ASSERT_MSG(event < emitter->numEvents, "invalid event id");
  event_publish(&(emitter->events[event]), arg);
}
