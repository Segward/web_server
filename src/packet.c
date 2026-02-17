#include <packet.h>

packet_t packet_new(int status_code, const char *status_text, 
                    const char *content_type, const char *body)
{
  packet_t packet = {
    .status_code = status_code,
    .status_text = status_text,
    .content_type = content_type,
    .body = body
  };

  return packet;
}
