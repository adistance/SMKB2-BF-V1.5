/*
*   Byte-oriented AES-256 implementation.
*   All lookup tables replaced with 'on the fly' calculations.
*
*   Copyright (c) 2007-2009 Ilya O. Levin, http://www.literatecode.com
*   Other contributors: Hal Finney
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose with or without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies.
*
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#ifndef __ALGO_AES256_H__
#define __ALGO_AES256_H__ (1)

#ifndef uint8_t
#define uint8_t  unsigned char
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifdef __cplusplus
extern "C" {
#endif

    void aes256_init( uint8_t * /* AesKey */, uint8_t * /* AesNonce */ );
    void aes256_done(void);
    void aes256_encrypt_ecb(uint8_t * /* plaintext */);
    void aes256_decrypt_ecb(uint8_t * /* cipertext */);
    void aes256_endecrypt_blk_ctr(uint8_t * /* plaintext or cipertext */);
    void aes256_ctr(uint8_t * /* AesKey */, uint8_t * /* AesNonce */,
                    uint8_t * /* nStartAddr */, int /* nSize */);
#ifdef __cplusplus
}
#endif

#endif // __ALGO_AES256_H__
