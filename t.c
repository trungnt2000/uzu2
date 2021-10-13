
struct entity
{
    int x;
    int y;
}

struct action_opts
{
    void (*start)(void* self, struct entity* entity);
    void (*ended)(void* self, struct entity* entity);
    void (*tick)(void* self, struct entity* entity);
};

struct action
{
    const struct action_opts* opts;
};

struct move_action
{
    struct action action;
    int           dst_x;
    int           dst_y;
};

struct
