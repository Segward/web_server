#ifndef ROUTER_H
#define ROUTER_H

#include <packet.h>

#define MAX_ROUTES 16

typedef packet_t (*route_handler_t)(void);

void router_add(const char *path, route_handler_t handler);
packet_t router_handle(const char *path);

#endif
