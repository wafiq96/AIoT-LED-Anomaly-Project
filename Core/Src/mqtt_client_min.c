#include "mqtt_client_min.h"
#include <string.h>

/* ---- helpers ---- */
static uint16_t next_packet_id = 1;

static uint8_t *put_u16(uint8_t *p, uint16_t v) { *p++ = (v >> 8) & 0xFF; *p++ = v & 0xFF; return p; }

static uint8_t *put_str(uint8_t *p, const char *s)
{
    uint16_t len = (uint16_t)strlen(s);
    p = put_u16(p, len);
    memcpy(p, s, len);
    return p + len;
}

/* MQTT remaining-length uses a variable-length encoding (1-4 bytes) */
static uint8_t *encode_remaining_len(uint8_t *p, uint32_t len)
{
    do {
        uint8_t byte = len % 128;
        len /= 128;
        if (len > 0) byte |= 0x80;
        *p++ = byte;
    } while (len > 0);
    return p;
}

/* ---- CONNECT ---- */

int mqtt_connect(tls_ctx_t *tls,
                  const char *client_id,
                  const char *username,
                  const char *password,
                  uint16_t keepalive_sec)
{
    uint8_t buf[256];
    uint8_t *p = buf;

    /* Fixed header written after we know remaining length, so build variable+payload first */
    uint8_t body[240];
    uint8_t *b = body;

    b = put_str(b, "MQTT");             /* protocol name */
    *b++ = 0x04;                        /* protocol level = MQTT 3.1.1 */

    uint8_t connect_flags = 0x02;       /* clean session */
    if (username) connect_flags |= 0x80;
    if (password) connect_flags |= 0x40;
    *b++ = connect_flags;

    b = put_u16(b, keepalive_sec);

    b = put_str(b, client_id);
    if (username) b = put_str(b, username);
    if (password) b = put_str(b, password);

    uint32_t remaining_len = (uint32_t)(b - body);

    p = buf;
    *p++ = 0x10;                         /* CONNECT packet type */
    p = encode_remaining_len(p, remaining_len);
    memcpy(p, body, remaining_len);
    p += remaining_len;

    int ret = tls_transport_send(tls, buf, (size_t)(p - buf));
    if (ret < 0) return ret;

    /* Read CONNACK: fixed header (2 bytes) + flags(1) + return code(1) = 4 bytes */
    uint8_t resp[4] = {0};
    int got = 0;
    for (int tries = 0; tries < 50 && got < 4; tries++) {
        int r = tls_transport_recv(tls, resp + got, sizeof(resp) - got, 200);
        if (r < 0) return r;
        got += r;
    }
    if (got < 4 || (resp[0] & 0xF0) != 0x20)
        return -100;   /* didn't get a CONNACK */

    return resp[3];    /* CONNACK return code, 0 = accepted */
}

/* ---- PUBLISH (QoS 0, no packet identifier needed) ---- */

int mqtt_publish_qos0(tls_ctx_t *tls, const char *topic,
                       const uint8_t *payload, uint16_t payload_len)
{
    uint8_t buf[512];
    uint8_t *p = buf;

    uint8_t body[480];
    uint8_t *b = body;
    b = put_str(b, topic);
    if ((size_t)(b - body) + payload_len > sizeof(body)) return -1; /* payload too big for this buffer */
    memcpy(b, payload, payload_len);
    b += payload_len;

    uint32_t remaining_len = (uint32_t)(b - body);

    p = buf;
    *p++ = 0x30;   /* PUBLISH, QoS0, no DUP/RETAIN - set 0x31 if you want RETAIN */
    p = encode_remaining_len(p, remaining_len);
    memcpy(p, body, remaining_len);
    p += remaining_len;

    return tls_transport_send(tls, buf, (size_t)(p - buf));
}

int mqtt_ping(tls_ctx_t *tls)
{
    uint8_t pkt[2] = { 0xC0, 0x00 };   /* PINGREQ */
    return tls_transport_send(tls, pkt, sizeof(pkt));
    /* Broker replies with PINGRESP (0xD0 0x00); safe to ignore if you're only publishing */
}

void mqtt_disconnect(tls_ctx_t *tls)
{
    uint8_t pkt[2] = { 0xE0, 0x00 };   /* DISCONNECT */
    tls_transport_send(tls, pkt, sizeof(pkt));
}

static int recv_exact(tls_ctx_t *tls, uint8_t *buf, size_t len, uint32_t per_call_timeout_ms, int max_tries)
{
    size_t got = 0;
    for (int tries = 0; tries < max_tries && got < len; tries++) {
        int r = tls_transport_recv(tls, buf + got, len - got, per_call_timeout_ms);
        if (r < 0) return r;
        got += (size_t)r;
    }
    return (int)got;
}

int mqtt_subscribe(tls_ctx_t *tls, const char *topic, uint8_t qos)
{
    uint8_t buf[256];
    uint8_t *p = buf;
    uint8_t body[240];
    uint8_t *b = body;

    uint16_t pid = next_packet_id++;
    if (next_packet_id == 0) next_packet_id = 1;   /* packet id must never be 0 */

    b = put_u16(b, pid);
    b = put_str(b, topic);
    *b++ = qos;

    uint32_t remaining_len = (uint32_t)(b - body);

    p = buf;
    *p++ = 0x82;   /* SUBSCRIBE - low nibble must be 0b0010, fixed by spec */
    p = encode_remaining_len(p, remaining_len);
    memcpy(p, body, remaining_len);
    p += remaining_len;

    return tls_transport_send(tls, buf, (size_t)(p - buf));
    /* Broker replies with SUBACK - mqtt_poll() below reads and discards it safely */
}

int mqtt_poll(tls_ctx_t *tls, mqtt_message_handler_t handler)
{
    uint8_t header_byte;

    /* Short timeout: if nothing has arrived, don't stall the caller's loop */
    int r = tls_transport_recv(tls, &header_byte, 1, 10);
    if (r == 0) return 0;
    if (r < 0) return r;

    /* Remaining-length field: 1-4 bytes, matches encode_remaining_len's format */
    uint32_t multiplier = 1, remaining_len = 0;
    uint8_t enc_byte;
    int len_bytes = 0;
    do {
        if (recv_exact(tls, &enc_byte, 1, 50, 20) != 1) return -101;
        remaining_len += (uint32_t)(enc_byte & 0x7F) * multiplier;
        multiplier *= 128;
        if (++len_bytes > 4) return -102;   /* malformed length */
    } while (enc_byte & 0x80);

    uint8_t packet_type = header_byte & 0xF0;

    if (packet_type == 0x30)   /* PUBLISH */
    {
        uint8_t body[512];
        if (remaining_len > sizeof(body)) return -103;
        if (recv_exact(tls, body, remaining_len, 50, 40) != (int)remaining_len) return -104;

        uint16_t topic_len = ((uint16_t)body[0] << 8) | body[1];
        char topic[128];
        if (topic_len >= sizeof(topic)) return -105;
        memcpy(topic, body + 2, topic_len);
        topic[topic_len] = '\0';

        size_t offset = 2u + topic_len;
        uint8_t qos = (header_byte & 0x06) >> 1;
        if (qos > 0) offset += 2;

        uint16_t payload_len = (uint16_t)(remaining_len - offset);
        if (handler) handler(topic, body + offset, payload_len);
        return 1;
    }

    uint8_t scratch[64];
    uint32_t left = remaining_len;
    while (left > 0) {
        uint32_t chunk = left > sizeof(scratch) ? sizeof(scratch) : left;
        if (recv_exact(tls, scratch, chunk, 50, 20) != (int)chunk) break;
        left -= chunk;
    }
    return 0;
}
