#include <tls_server.h>
#include <router.h>
#include <packet.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define REQ_BUF 4096
#define BACKLOG 16

static void extract_path(const char *request, char *path_out)
{
  char method[16];
  char version[16];
  sscanf(request, "%15s %255s %15s", method, path_out, version);
}

static int send_packet_tls(SSL *ssl, packet_t p)
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

  if (header_len < 0) return 0;

  if (SSL_write(ssl, header, header_len) <= 0) return 0;
  if (SSL_write(ssl, p.body, (int)body_len) <= 0) return 0;

  return 1;
}

static void handle_client_tls(SSL_CTX *ctx, int client_fd)
{
  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    close(client_fd);
    return;
  }

  SSL_set_fd(ssl, client_fd);

  if (SSL_accept(ssl) <= 0) {
    SSL_free(ssl);
    close(client_fd);
    return;
  }

  char buffer[REQ_BUF];
  int n = SSL_read(ssl, buffer, sizeof(buffer) - 1);
  if (n <= 0) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_fd);
    return;
  }

  buffer[n] = '\0';

  char path[256];
  extract_path(buffer, path);

  printf("HTTPS request for: %s\n", path);

  packet_t response = router_handle(path);
  send_packet_tls(ssl, response);

  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(client_fd);
}

static SSL_CTX *create_server_ctx(const char *cert_path, const char *key_path)
{
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  const SSL_METHOD *method = TLS_server_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) return NULL;

  if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
    SSL_CTX_free(ctx);
    return NULL;
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
    SSL_CTX_free(ctx);
    return NULL;
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    SSL_CTX_free(ctx);
    return NULL;
  }

  return ctx;
}

int tls_server_start(int port, const char *cert_path, const char *key_path)
{
  SSL_CTX *ctx = create_server_ctx(cert_path, key_path);
  if (!ctx) {
    fprintf(stderr, "Failed to create SSL_CTX (bad cert/key?)\n");
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    SSL_CTX_free(ctx);
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
    close(server_fd);
    SSL_CTX_free(ctx);
    return 1;
  }

  if (listen(server_fd, BACKLOG) < 0) {
    perror("listen");
    close(server_fd);
    SSL_CTX_free(ctx);
    return 1;
  }

  printf("HTTPS server running on https://127.0.0.1:%d\n", port);

  while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) continue;

    handle_client_tls(ctx, client_fd);
  }

  close(server_fd);
  SSL_CTX_free(ctx);
  return 0;
}
