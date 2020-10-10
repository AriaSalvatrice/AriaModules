/*             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/
#pragma once

// Deterministic seedable PRNG
// Using xoroshiro128+ like VCV does - gives me a better distribution than mersenne twister.
// http://prng.di.unimi.it/
// https://community.vcvrack.com/t/controlling-the-random-seed/8005
// 
// The original algorithm is in the public domain, this is just a tiny wrapper around it.
//
// Running an automated test on Darius, I'm obtaining a distribution of results that look sane, 
// not skewed to either side, which was a problem with std's mersenne twister.
namespace prng {

struct prng {

    inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    uint64_t s[2];

    uint64_t next(void) {
        const uint64_t s0 = s[0];
        uint64_t s1 = s[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
        s[1] = rotl(s1, 37); // c

        return result;
    }
    
    void init(float seed1, float seed2){
        s[0] = seed1 * 572376460694501; // Keyboard smash - salting seems to improve results
        s[1] = seed2 * 645624357248923;
        for (size_t i = 0; i < 50; i++) next(); // Warm up for better results
    }

    float uniform() {
        for (size_t i = 0; i < 50; i++) next(); // More dry runs.
        return (next() >> (64 - 24)) / std::pow(2.f, 24);
    }

};

}
