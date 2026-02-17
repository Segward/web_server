#ifndef PACKET_H
#define PACKET_H

typedef struct packet 
{
  int         status_code;
  const char *status_text;
  const char *content_type;
  const char *body;
} packet_t;

packet_t packet_new(int status_code, const char *status_text, 
                    const char *content_type, const char *body);

#endif
