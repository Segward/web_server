#ifndef TLS_SERVER_H
#define TLS_SERVER_H

int tls_server_start(int port, const char *cert_path, const char *key_path);

#endif
