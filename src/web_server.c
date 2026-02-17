#include <web_server.h>
#include <router.h>
#include <packet.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define REQ_BUF 4096
#define BACKLOG 16

static void send_packet(int client_fd, packet_t p)
{
  size_t body_len = strlen(p.body);

  char header[1024];
  int header_len = snprintf(header, sizeof(header),
                            "HTTP/1.1 %d %s\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %zu\r\n"
                            "Connection: close\r\n"
                            "\r\n",
                            p.status_code,
                            p.status_text,
                            p.content_type,
                            body_len);

  send(client_fd, header, header_len, 0);
  send(client_fd, p.body, body_len, 0);
}

static void extract_path(const char *request, char *path_out)
{
  char method[16];
  char version[16];

  sscanf(request, "%15s %255s %15s", method, path_out, version);
}

static void handle_client(int client_fd)
{
  char buffer[REQ_BUF];
  int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
  if (n <= 0) return;

  buffer[n] = '\0';

  char path[256];
  extract_path(buffer, path);

  printf("Request for: %s\n", path);

  packet_t response = router_handle(path);
  send_packet(client_fd, response);
}

int web_server_start(int port)
{
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(server_fd, BACKLOG) < 0) {
    perror("listen");
    return 1;
  }

  printf("Server running on http://127.0.0.1:%d\n", port);

  while (1)
  {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) continue;

    handle_client(client_fd);
    close(client_fd);
  }
}
