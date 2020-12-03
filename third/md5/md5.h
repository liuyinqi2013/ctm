
#ifndef _MD5_H_INCLUDED_
#define _MD5_H_INCLUDED_

#include <stddef.h>

typedef unsigned char u_char;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MD5_API
#define MD5_API 
#endif

typedef struct {
    uint64_t  bytes;
    uint32_t  a, b, c, d;
    u_char    buffer[64];
} md5_t;

MD5_API void md5_init(md5_t *ctx);
MD5_API void md5_update(md5_t *ctx, const void *data, size_t size);
MD5_API void md5_final(u_char result[16], md5_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* _MD5_H_INCLUDED_ */
