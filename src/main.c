#include <web_server.h>
#include <packet.h>
#include <router.h>

packet_t page_root(void)
{
  return packet_new(200, "OK", "text/html", 
                    "<h1>Home</h1>" 
                    "<a href='/page1'>Page1</a><br>"
                    "<a href='/page2'>Page2</a>");
}

packet_t page1(void)
{
    return packet_new(200, "OK", "text/html",
                      "<h1>This is page1</h1><a href='/'>Home</a>");
}

packet_t page2(void)
{
    return packet_new(200, "OK", "text/html",
                      "<h1>This is page2</h1><a href='/'>Home</a>");
}

int main()
{
  router_add("/", page_root);
  router_add("/page1", page1);
  router_add("/page2", page2);

  return web_server_start(3000);
}
