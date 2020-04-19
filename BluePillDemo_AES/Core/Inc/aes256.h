#ifndef AES256_H
#define AES256_H

/*  
*   Byte-oriented AES-256 implementation.
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

typedef struct 
{
    unsigned char key[32]; 
    unsigned char enckey[32]; 
    unsigned char deckey[32];
} aes256_context; 

/**
 * Initialize AES-256 encryption 
 */
void aes256_init(aes256_context *ctx, unsigned char *k);

/** 
 * Encrypt plaintext in buf to ciphertext in buf
 */
void aes256_encrypt_ecb(aes256_context *ctx, unsigned char *buf);

/** 
 * Decrypt ciphertext in buf to plaintext in buf
 */
void aes256_decrypt_ecb(aes256_context *ctx, unsigned char *buf);

/**
 * Clean up after use to clear memory
 */
void aes256_done(aes256_context *ctx);

#endif

