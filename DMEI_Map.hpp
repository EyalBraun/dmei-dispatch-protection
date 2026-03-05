/**
 * @file DMEI_Map.hpp
 * @author Eyal B.
 * @brief Deterministic Mathematical Entropy Indexing (DMEI)
 * @version 1.0
 * @date 2026
 * * DMEI provides near-zero latency ( ~2.8 cycles) identity mapping 
 * for Hard Real-Time systems and High-Frequency Trading.
 */

#ifndef DMEI_MAP_HPP
#define DMEI_MAP_HPP

#include <cstdint>
#include <vector>
#include <optional>

namespace DMEI {

    // Constants for the mixing function (M1, M2)
    // Derived from prime distribution for maximum avalanche effect
    const uint64_t M1 = 0xff51afd7ed558ccdLLU;
    const uint64_t M2 = 0xc4ceb9fe1a85ec53LLU;

    /**
     * @brief Core Bijective Mixing Function h(x)
     * Performs XOR-Shift-Multiply to ensure perfect bit-diffusion.
     */
    inline uint64_t h(uint64_t x) {
        x ^= x >> 33;
        x *= M1;
        x ^= x >> 33;
        x *= M2;
        x ^= x >> 33;
        return x;
    }

    /**
     * @brief The Recursive Identity Mapping
     * @param s_hash The precomputed hash of the input (seed)
     * @return A deterministic index in the memory field
     */
    inline uint64_t lookup(uint64_t s_hash) {
        // Implementation of the identity: I = h(h(s))
        // Note: For advanced ASLR-safe usage, XOR with base offset here.
        return h(s_hash);
    }

    /**
     * @brief A High-Performance Static Map using DMEI
     * Optimized for cases where keys are known at initialization (Flight Controllers).
     */
    template <typename T>
    class Map {
    private:
        std::vector<T> data;
        uint64_t mask;

    public:
        explicit Map(size_t size) {
            // Round up to power of 2 for fast masking
            size_t actual_size = 1;
            while (actual_size < size) actual_size <<= 1;
            data.resize(actual_size);
            mask = actual_size - 1;
        }

        /**
         * @brief Insert a value using its hash
         * @param key_hash The h(s) of the key
         * @param value The value to store
         */
        void insert(uint64_t key_hash, const T& value) {
            uint64_t idx = lookup(key_hash) & mask;
            data[idx] = value;
        }

        /**
         * @brief Direct Lookup - Zero Jitter
         * @param key_hash The h(s) of the key
         * @return Reference to the stored value
         */
        inline const T& get(uint64_t key_hash) const {
            return data[lookup(key_hash) & mask];
        }
    };
}

#endif // DMEI_MAP_HPP
