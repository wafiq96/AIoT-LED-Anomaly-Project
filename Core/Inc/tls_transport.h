#ifndef TLS_TRANSPORT_H
#define TLS_TRANSPORT_H

#include <stdint.h>
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"

typedef struct {
    mbedtls_entropy_context   entropy;
    mbedtls_ctr_drbg_context  ctr_drbg;
    mbedtls_ssl_context       ssl;
    mbedtls_ssl_config        conf;
    mbedtls_x509_crt          ca_cert;
    uint8_t                   wifi_socket;   /* es-wifi socket number, e.g. 0 */
} tls_ctx_t;

/*
 * Opens a raw TCP connection to host:port via the onboard ISM43362 (WIFI_OpenClientConnection),
 * then performs the mbedTLS handshake on top of it, verifying against ca_pem.
 *
 * ca_pem     : NUL-terminated PEM string (must include -----BEGIN/END CERTIFICATE-----)
 * ca_pem_len : strlen(ca_pem) + 1  (mbedtls_x509_crt_parse wants the NUL included for PEM)
 *
 * Returns 0 on success, negative mbedtls error code otherwise.
 */
int tls_transport_connect(tls_ctx_t *ctx,
                           uint8_t wifi_socket,
                           const char *host,
                           uint16_t port,
                           const unsigned char *ca_pem,
                           size_t ca_pem_len);

/* Wraps mbedtls_ssl_write, handles WANT_READ/WANT_WRITE retry loop. Returns bytes written or <0. */
int tls_transport_send(tls_ctx_t *ctx, const unsigned char *buf, size_t len);

/* Wraps mbedtls_ssl_read. Returns bytes read, 0 on orderly close, <0 on error. */
int tls_transport_recv(tls_ctx_t *ctx, unsigned char *buf, size_t len, uint32_t timeout_ms);

void tls_transport_close(tls_ctx_t *ctx);

#endif /* TLS_TRANSPORT_H */
