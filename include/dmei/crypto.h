#ifndef DMEI_CRYPTO_H
#define DMEI_CRYPTO_H

#include <cstdint>

// In a real Linux Kernel environment, this would use get_random_bytes()
// In an embedded RTOS/Drone environment (like ESP32 or STM32), this maps 
// to the hardware True Random Number Generator (TRNG) peripheral.

namespace dmei {

uint64_t generate_secure_entropy() {
    uint64_t entropy = 0;
    
#if defined(__linux__)
    // Read from /dev/urandom or syscall
    FILE* urandom = fopen("/dev/urandom", "r");
    if (urandom) {
        fread(&entropy, sizeof(entropy), 1, urandom);
        fclose(urandom);
    }
#elif defined(ESP32)
    // ESP32 Hardware RNG
    entropy = esp_random();
    entropy = (entropy << 32) | esp_random();
#else
    // Fallback/Simulated
    entropy = 0xFEEDBEEFCAFEBABE; // Replace with hardware RNG reading
#endif

    return entropy;
}

// Fast avalanche hashing for table indices (Simplified xxHash/Murmur concept)
inline uint64_t seeded_fast_hash(uint64_t key, uint64_t seed, uint8_t func_id) {
    uint64_t h = key ^ (seed + func_id);
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

} // namespace dmei

#endif // DMEI_CRYPTO_H
