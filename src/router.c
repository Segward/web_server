#include <router.h>
#include <string.h>

typedef struct route_entry
{
  const char *path;
  route_handler_t handler;
} route_entry_t;

static route_entry_t routes[MAX_ROUTES];
static int route_count = 0;

void router_add(const char *path, route_handler_t handler)
{
  routes[route_count].path = path;
  routes[route_count].handler = handler;
  route_count++;
}

packet_t router_handle(const char *path)
{
  for (int i = 0; i < route_count; i++)
    if (strcmp(path, routes[i].path) == 0)
      return routes[i].handler();

  return packet_new(404, "Not Found", "text/html", "<h1>404 Not Found</h1>");
}
