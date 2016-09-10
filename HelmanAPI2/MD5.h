#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define ROUND0(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, d ^ (b & (c ^ d)), k, s, t)
#define ROUND1(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, c ^ (d & (b ^ c)), k, s, t)
#define ROUND2(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, b ^ c ^ d        , k, s, t)
#define ROUND3(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, c ^ (b | ~d)     , k, s, t)

#define ROUND_TAIL(a, b, expr, k, s, t)    \
	a += (expr) + UINT32_C(t) + block[k];  \
	a = b + (a << s | a >> (32 - s));


#define HASH_SIZE 4             //Size of Chain Point (Bytes), must be less or equal to sizeof(hash_t)

typedef uint32_t hash_t;        //Chain Point Value Type
typedef uint16_t len_t; 	    //Length of Chain Type
typedef uint32_t count_t;       //Number of Chains Type

/*
Hashes n times MD5
Returns chains point after n hash computations.
@param input : Input chain point.
@param flavor : Falvor value.
@param n : Number of times to hash.
*/
hash_t n_hash(hash_t input, hash_t flavor, len_t n);

/*
Genrate hash input
Returns pseudo random chain point.
@param seed : Seed to randomize the generated value.
*/
hash_t gen_input(hash_t seed);

/*
MD5 compression function
*/
void md5_compress(uint32_t state[4], uint32_t block[16]);

/*
MD5 Hash message function
*/
void md5_hash(uint8_t *message, uint32_t len, uint32_t hash[4]);


#ifdef __cplusplus
}      /* extern "C" */
#endif  /* __cplusplus */

#endif //MD5_H
