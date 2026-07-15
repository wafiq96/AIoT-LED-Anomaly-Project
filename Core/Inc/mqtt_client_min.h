#ifndef MQTT_CLIENT_MIN_H
#define MQTT_CLIENT_MIN_H

#include <stdint.h>
#include <stddef.h>
#include "tls_transport.h"

int mqtt_connect(tls_ctx_t *tls,
                  const char *client_id,
                  const char *username,
                  const char *password,
                  uint16_t keepalive_sec);

int mqtt_publish_qos0(tls_ctx_t *tls, const char *topic, const uint8_t *payload, uint16_t payload_len);
int mqtt_ping(tls_ctx_t *tls);

void mqtt_disconnect(tls_ctx_t *tls);

int mqtt_subscribe(tls_ctx_t *tls, const char *topic, uint8_t qos);

typedef void (*mqtt_message_handler_t)(const char *topic, const uint8_t *payload, uint16_t payload_len);

int mqtt_poll(tls_ctx_t *tls, mqtt_message_handler_t handler);

#endif /* MQTT_CLIENT_MIN_H */
