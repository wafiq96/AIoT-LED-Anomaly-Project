#include "tls_transport.h"
#include "wifi.h"          /* X-CUBE-WIFI1 es-wifi driver API */
#include <string.h>
#include <stdio.h>

/* ---- BIO callbacks: mbedTLS calls these, we forward to WIFI_SendData/WIFI_ReceiveData ---- */

static int es_wifi_send(void *ctx_v, const unsigned char *buf, size_t len)
{
    tls_ctx_t *ctx = (tls_ctx_t *)ctx_v;
    uint16_t sent = 0;
    WIFI_Status_t st = WIFI_SendData(ctx->wifi_socket, (uint8_t *)buf,
                                      (uint16_t)len, &sent, 5000);
    printf("TLS: send -> st=%d sent=%u/%u\r\n", st, sent, (unsigned)len);
    if (st != WIFI_STATUS_OK)
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    return (int)sent;
}

static int es_wifi_recv_timeout(void *ctx_v, unsigned char *buf, size_t len, uint32_t timeout_ms)
{
    tls_ctx_t *ctx = (tls_ctx_t *)ctx_v;
    uint16_t got = 0;

    /* the ES-WIFI/ISM43362 driver hard-fails any single read request over
       ES_WIFI_PAYLOAD_SIZE (1200 bytes) instead of chunking internally -
       cap so mbedTLS's larger reads (Certificate message, etc.) succeed
       across multiple calls instead of erroring out immediately */
    if (len > 1200) {
        len = 1200;
    }

    WIFI_Status_t st = WIFI_ReceiveData(ctx->wifi_socket, buf, (uint16_t)len,
                                         &got, timeout_ms ? timeout_ms : 1000);
    printf("TLS: recv -> st=%d got=%u (req %u)\r\n", st, got, (unsigned)len);
    if (st != WIFI_STATUS_OK)
        return MBEDTLS_ERR_SSL_WANT_READ;
    if (got == 0)
        return MBEDTLS_ERR_SSL_TIMEOUT;
    return (int)got;
}

int tls_transport_connect(tls_ctx_t *ctx,
                           uint8_t wifi_socket,
                           const char *host,
                           uint16_t port,
                           const unsigned char *ca_pem,
                           size_t ca_pem_len)
{
    int ret;
    ctx->wifi_socket = wifi_socket;

    mbedtls_ssl_init(&ctx->ssl);
    mbedtls_ssl_config_init(&ctx->conf);
    mbedtls_x509_crt_init(&ctx->ca_cert);
    mbedtls_ctr_drbg_init(&ctx->ctr_drbg);
    mbedtls_entropy_init(&ctx->entropy);

    /* 1. Resolve hostname to IP, then open the raw TCP socket on the ISM43362.
     *    WIFI_OpenClientConnection wants a resolved IP, not a hostname string -
     *    the module doesn't reliably do DNS itself from that call. */
    uint8_t resolved_ip[4] = {0};
    WIFI_Status_t wst = WIFI_GetHostAddress((char *)host, resolved_ip, sizeof(resolved_ip));
    if (wst != WIFI_STATUS_OK) {
        printf("TLS: DNS resolution failed for %s (%d)\r\n", host, wst);
        return -1;
    }
    printf("TLS: %s -> %d.%d.%d.%d\r\n", host,
           resolved_ip[0], resolved_ip[1], resolved_ip[2], resolved_ip[3]);

    wst = WIFI_OpenClientConnection(ctx->wifi_socket, WIFI_TCP_PROTOCOL,
                                     "hivemq", resolved_ip, port, 0);
    if (wst != WIFI_STATUS_OK) {
        printf("TLS: WIFI_OpenClientConnection failed (%d)\r\n", wst);
        return -1;
    }

    /* 2. Seed RNG */
    const char *pers = "hivemq_tls_client";
    ret = mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy,
                                 (const unsigned char *)pers, strlen(pers));
    if (ret != 0) goto fail;

    /* 3. Load CA cert used to verify HiveMQ's chain (Let's Encrypt ISRG Root X1 for HiveMQ Cloud) */
    ret = mbedtls_x509_crt_parse(&ctx->ca_cert, ca_pem, ca_pem_len);
    if (ret != 0) {
        printf("TLS: CA cert parse failed (-0x%04x)\r\n", -ret);
        goto fail;
    }

    /* 4. TLS config */
    ret = mbedtls_ssl_config_defaults(&ctx->conf, MBEDTLS_SSL_IS_CLIENT,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) goto fail;

    mbedtls_ssl_conf_authmode(&ctx->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&ctx->conf, &ctx->ca_cert, NULL);
    mbedtls_ssl_conf_rng(&ctx->conf, mbedtls_ctr_drbg_random, &ctx->ctr_drbg);

    ret = mbedtls_ssl_setup(&ctx->ssl, &ctx->conf);
    if (ret != 0) goto fail;

    /* hostname is required so mbedTLS checks it against the cert's SAN/CN */
    ret = mbedtls_ssl_set_hostname(&ctx->ssl, host);
    if (ret != 0) goto fail;

    mbedtls_ssl_set_bio(&ctx->ssl, ctx, es_wifi_send, NULL, es_wifi_recv_timeout);
    mbedtls_ssl_conf_read_timeout(&ctx->conf, 5000);

    /* 5. Handshake */
    int hs_attempts = 0;
    while ((ret = mbedtls_ssl_handshake(&ctx->ssl)) != 0) {
    	if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
    		printf("TLS: handshake failed (-0x%04x)\r\n", -ret);
            goto fail;
        }
        if (++hs_attempts > 20) {
        	printf("TLS: handshake giving up after %d retries\r\n", hs_attempts);
            goto fail;
        }
    }

    uint32_t flags = mbedtls_ssl_get_verify_result(&ctx->ssl);
    if (flags != 0) {
        char vrfy_buf[256];
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
        printf("TLS: cert verification failed:\r\n%s\r\n", vrfy_buf);
        ret = -1;
        goto fail;
    }

    printf("TLS: handshake OK, cipher = %s\r\n", mbedtls_ssl_get_ciphersuite(&ctx->ssl));
    return 0;

fail:
    tls_transport_close(ctx);
    return ret;
}

int tls_transport_send(tls_ctx_t *ctx, const unsigned char *buf, size_t len)
{
    int ret;
    size_t written = 0;
    while (written < len) {
        ret = mbedtls_ssl_write(&ctx->ssl, buf + written, len - written);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;
        if (ret < 0)
            return ret;
        written += (size_t)ret;
    }
    return (int)written;
}

int tls_transport_recv(tls_ctx_t *ctx, unsigned char *buf, size_t len, uint32_t timeout_ms)
{
    (void)timeout_ms; /* configured once via mbedtls_ssl_conf_read_timeout */
    int ret = mbedtls_ssl_read(&ctx->ssl, buf, len);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_TIMEOUT)
        return 0;
    return ret;
}

void tls_transport_close(tls_ctx_t *ctx)
{
    mbedtls_ssl_close_notify(&ctx->ssl);
    WIFI_CloseClientConnection(ctx->wifi_socket);
    mbedtls_ssl_free(&ctx->ssl);
    mbedtls_ssl_config_free(&ctx->conf);
    mbedtls_x509_crt_free(&ctx->ca_cert);
    mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
    mbedtls_entropy_free(&ctx->entropy);
}
