/*
 * Add/uncomment these in your project's mbedtls_config.h
 * (CubeMX: Middleware > MBEDTLS, or the copy under Middlewares/Third_Party/mbedTLS/include/mbedtls/mbedtls_config.h)
 *
 * Goal: minimal TLS1.2 client stack that can do ECDHE-RSA/ECDSA + AES-GCM,
 * sized to fit STM32L4S5 SRAM (640 KB total, but you're sharing it with
 * WiFi driver buffers + your sensor/MQTT state, so keep mbedTLS lean).
 */

/* --- Core TLS --- */
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_MAX_CONTENT_LEN        8192   /* default 16384 - too big, HiveMQ handshake fits in 4K */

/* --- X.509 (needed to parse HiveMQ's / Let's Encrypt CA cert) --- */
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_OID_C

/* --- Key exchange / signature (covers both RSA and ECDSA leaf certs) --- */
#define MBEDTLS_RSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

/* --- Symmetric / hash for AES-GCM-SHA256 ciphersuite --- */
#define MBEDTLS_AES_C
#define MBEDTLS_GCM_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_MD_C

/* --- RNG (mandatory for TLS) --- */
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_HARDWARE_ALT   /* wire to STM32 RNG peripheral - see hw_entropy.c below */

/* --- Bignum backing everything above --- */
#define MBEDTLS_BIGNUM_C

/* --- Memory: use a static pool instead of malloc/free fragmentation --- */
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_PLATFORM_MEMORY

/* In your init code, before any mbedTLS call:
 *   static unsigned char mbedtls_pool[48 * 1024];   // start at 48K, tune with mbedtls_memory_buffer_alloc_max_get()
 *   mbedtls_memory_buffer_alloc_init(mbedtls_pool, sizeof(mbedtls_pool));
 *
 * If you see MBEDTLS_ERR_SSL_ALLOC_FAILED during handshake, raise the pool size,
 * NOT MBEDTLS_SSL_MAX_CONTENT_LEN (that's the bigger lever, don't touch it first).
 */

/* --- Turn OFF things you don't need (keeps flash/RAM down) --- */
#undef MBEDTLS_SSL_PROTO_TLS1_3          /* not needed, adds a lot of code */
#undef MBEDTLS_SSL_PROTO_DTLS
#undef MBEDTLS_SSL_SRV_C                 /* you're only ever a client */
#undef MBEDTLS_SSL_RENEGOTIATION
