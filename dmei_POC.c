#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <wmmintrin.h>
#include <immintrin.h>

#define TABLE_SIZE 512

// ----------------------------
// DMEI VAULT (secrets per epoch)
// ----------------------------
typedef struct {
    __m128i master_key_K;      // K: master key
    __m128i slot_key_E;        // E: slot encryption key
    uint64_t global_anchor_T;  // T: base address anchor
    uint64_t n_section_size;   // size of N-section [0, n_section_size)
    uint64_t epoch_id;         // reshuffle counter
} DMEI_Vault;

DMEI_Vault dmei_vault;

// ----------------------------
// DISPATCH TABLE (N + M sections)
// ----------------------------
uint64_t dmei_table[TABLE_SIZE];

// ----------------------------
// AES helpers (for Enc_E / Dec_E)
// ----------------------------
static inline __m128i aes_block(__m128i p, __m128i k) {
    return _mm_aesenc_si128(p, k);
}

static inline uint64_t aes64(__m128i p, __m128i k) {
    return (uint64_t)_mm_extract_epi64(aes_block(p, k), 0);
}

static inline uint64_t enc_E(uint64_t x) {
    __m128i p = _mm_set_epi64x(0, (long long)x);
    return aes64(p, dmei_vault.slot_key_E);
}

static inline uint64_t dec_E(uint64_t x) {
    __m128i p = _mm_set_epi64x(0, (long long)x);
    return aes64(p, dmei_vault.slot_key_E);
}

// ----------------------------
// FUNCTIONS TO DISPATCH
// ----------------------------
void task_alpha() { printf("      [EXEC] ALPHA\n"); }
void task_beta()  { printf("      [EXEC] BETA\n"); }
void task_gamma() { printf("      [EXEC] GAMMA\n"); }

typedef struct {
    uint64_t S;          // dispatch identifier
    const char *name;    // human-readable name
    void (*fn)();        // function pointer
    uint64_t caller_IP;  // valid caller IP (simulated)

    // fixed, collision-free indexes in N-section
    uint64_t an1_index;  // slot for Enc_E(arm)
    uint64_t an2_index;  // slot for Enc_E(T2)
    uint64_t an3_index;  // slot for Enc_E(FMAC)
    uint64_t jump_index; // slot for Enc_E(offset)

    // fixed index in M-section for caller IP
    uint64_t ip_slot_index;

    // per-epoch values (for printing)
    uint64_t arm_value;
    uint64_t T2_value;
    uint64_t FMAC_value;
    uint64_t offset_value;
    uint64_t reconstructed_Ij;
} DMEI_Function;

DMEI_Function dmei_functions[] = {
    // S, name, fn, IP, An1, An2, An3, j, IP-slot
    {0x01, "task_alpha", task_alpha, 0x7fff00001000ULL,  10, 11, 12, 13, 300},
    {0x02, "task_beta",  task_beta,  0x7fff00002000ULL,  20, 21, 22, 23, 301},
    {0x03, "task_gamma", task_gamma, 0x7fff00003000ULL,  30, 31, 32, 33, 302}
};

#define NUM_DMEI_FUNCS 3

// ----------------------------
// PRINT FUNCTION LIST (epoch view)
// ----------------------------
void print_function_epoch_view(void) {
    printf("\n=== Epoch %" PRIu64 " function view ===\n", dmei_vault.epoch_id);
    printf("Global anchor T = 0x%016" PRIx64 "\n", dmei_vault.global_anchor_T);
    printf("N-section size  = %" PRIu64 " (indexes [0..%" PRIu64 "])\n",
           dmei_vault.n_section_size, dmei_vault.n_section_size - 1);
    printf("M-section start = %" PRIu64 " (indexes [%" PRIu64 "..%d])\n",
           dmei_vault.n_section_size, dmei_vault.n_section_size, TABLE_SIZE - 1);

    printf("\nFunction entries:\n");
    printf("  S    Name         An1  An2  An3   j   IP_slot   arm            T2             FMAC           offset         I_j\n");
    printf("  ---- ------------ ---- ---- ---- ---- -------- ---------------- ---------------- ---------------- ---------------- ----------------\n");
    for (int i = 0; i < NUM_DMEI_FUNCS; i++) {
        DMEI_Function *f = &dmei_functions[i];
        printf("  0x%02lx %-12s %4lu %4lu %4lu %4lu %8lu 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx\n",
               (unsigned long)f->S,
               f->name,
               (unsigned long)f->an1_index,
               (unsigned long)f->an2_index,
               (unsigned long)f->an3_index,
               (unsigned long)f->jump_index,
               (unsigned long)f->ip_slot_index,
               (unsigned long)f->arm_value,
               (unsigned long)f->T2_value,
               (unsigned long)f->FMAC_value,
               (unsigned long)f->offset_value,
               (unsigned long)f->reconstructed_Ij);
    }
    printf("\n");
}

// ----------------------------
// BUILD / RESHUFFLE PHASE
// ----------------------------
void dmei_build_table(void) {
    dmei_vault.epoch_id++;

    // 1. Initialise vault secrets (demo: change per epoch)
    uint64_t base1 = 0x1111111111111111ULL ^ dmei_vault.epoch_id;
    uint64_t base2 = 0x2222222222222222ULL ^ (dmei_vault.epoch_id << 8);

    dmei_vault.master_key_K = _mm_set_epi64x((long long)base1, (long long)base2);

    uint64_t e1 = 0x3333333333333333ULL ^ (dmei_vault.epoch_id << 16);
    uint64_t e2 = 0x4444444444444444ULL ^ (dmei_vault.epoch_id << 24);
    dmei_vault.slot_key_E = _mm_set_epi64x((long long)e1, (long long)e2);

    dmei_vault.global_anchor_T = 0x1000000000000000ULL ^ (dmei_vault.epoch_id << 32);
    dmei_vault.n_section_size  = TABLE_SIZE / 2; // N-section [0..255], M-section [256..511]

    printf("\n[*] Reshuffle -> Epoch %" PRIu64 "\n", dmei_vault.epoch_id);
    printf("    K   = (0x%016" PRIx64 ", 0x%016" PRIx64 ")\n",
           (uint64_t)base1, (uint64_t)base2);
    printf("    E   = (0x%016" PRIx64 ", 0x%016" PRIx64 ")\n",
           (uint64_t)e1, (uint64_t)e2);
    printf("    T   = 0x%016" PRIx64 "\n", dmei_vault.global_anchor_T);

    // 2. Fill table with canaries
    for (int i = 0; i < TABLE_SIZE; i++) {
        dmei_table[i] = 0xDEAD0000 ^ (uint64_t)i;
    }

    // 3. Register caller IPs in M-section (fixed slots)
    for (int i = 0; i < NUM_DMEI_FUNCS; i++) {
        DMEI_Function *f = &dmei_functions[i];
        dmei_table[f->ip_slot_index] = enc_E(f->caller_IP);

        printf("    [CALLER] S=0x%lx (%s) IP=0x%lx -> M[%lu]\n",
               (unsigned long)f->S, f->name,
               (unsigned long)f->caller_IP,
               (unsigned long)f->ip_slot_index);
    }

    // 4. Register entries in N-section (An1, An2, An3, j)
    for (int i = 0; i < NUM_DMEI_FUNCS; i++) {
        DMEI_Function *f = &dmei_functions[i];

        uint64_t S = f->S;

        // arm and T2 change per epoch to show moving target
        f->arm_value = (0xA000000000000000ULL ^ S ^ dmei_vault.epoch_id);
        f->T2_value  = (0xB000000000000000ULL ^ S ^ (dmei_vault.epoch_id << 4));

        // FMAC: bind function address, S, epoch, and caller_IP
        f->FMAC_value = aes64(
            _mm_set_epi64x(
                (long long)(uintptr_t)f->fn,
                (long long)(S ^ dmei_vault.epoch_id ^ f->caller_IP)
            ),
            dmei_vault.master_key_K
        );

        // offset = I_j - T - T2, where I_j is fn address
        uint64_t I_j = (uint64_t)(uintptr_t)f->fn;
        f->offset_value = I_j - dmei_vault.global_anchor_T - f->T2_value;
        f->reconstructed_Ij = dmei_vault.global_anchor_T + f->T2_value + f->offset_value;

        dmei_table[f->an1_index] = enc_E(f->arm_value);
        dmei_table[f->an2_index] = enc_E(f->T2_value);
        dmei_table[f->an3_index] = enc_E(f->FMAC_value);
        dmei_table[f->jump_index] = enc_E(f->offset_value);

        printf("    [ENTRY] S=0x%lx (%s)\n", (unsigned long)S, f->name);
        printf("            An1=%lu  An2=%lu  An3=%lu  j=%lu\n",
               (unsigned long)f->an1_index,
               (unsigned long)f->an2_index,
               (unsigned long)f->an3_index,
               (unsigned long)f->jump_index);
    }

    print_function_epoch_view();
}

// ----------------------------
// LOOKUP DEMO (FMAC verified, no jump)
// ----------------------------
void dmei_lookup_demo(uint64_t S, uint64_t caller_IP) {
    // find function by S
    DMEI_Function *f = NULL;
    for (int i = 0; i < NUM_DMEI_FUNCS; i++) {
        if (dmei_functions[i].S == S) {
            f = &dmei_functions[i];
            break;
        }
    }
    if (!f) {
        printf("[LOOKUP] Unknown S=0x%lx\n", (unsigned long)S);
        return;
    }

    printf("\n[LOOKUP] Epoch %" PRIu64 ", S=0x%lx (%s), caller_IP=0x%lx\n",
           dmei_vault.epoch_id,
           (unsigned long)S,
           f->name,
           (unsigned long)caller_IP);

    // 1. Caller IP validation (fixed slot in M-section)
    uint64_t stored_enc_ip = dmei_table[f->ip_slot_index];
    if (stored_enc_ip != enc_E(caller_IP)) {
        printf("    -> invalid caller (M[%lu])\n",
               (unsigned long)f->ip_slot_index);
        printf("       expected Enc_E(0x%lx), got 0x%lx\n",
               (unsigned long)caller_IP,
               (unsigned long)stored_enc_ip);
        return;
    }
    printf("    -> caller IP valid at M[%lu]\n",
           (unsigned long)f->ip_slot_index);

    // 2. Resolve An slots from N-section
    uint64_t arm  = dec_E(dmei_table[f->an1_index]);
    uint64_t T2   = dec_E(dmei_table[f->an2_index]);
    uint64_t FMAC_stored = dec_E(dmei_table[f->an3_index]);

    // 3. Resolve jump slot
    uint64_t offset = dec_E(dmei_table[f->jump_index]);
    uint64_t I_j    = dmei_vault.global_anchor_T + T2 + offset;

    // 4. Recompute FMAC_now and verify integrity
    uint64_t FMAC_now = aes64(
        _mm_set_epi64x(
            (long long)(uintptr_t)f->fn,
            (long long)(S ^ dmei_vault.epoch_id ^ caller_IP)
        ),
        dmei_vault.master_key_K
    );

    printf("    N-section slots:\n");
    printf("      An1[%lu] -> arm   = 0x%lx\n",
           (unsigned long)f->an1_index, (unsigned long)arm);
    printf("      An2[%lu] -> T2    = 0x%lx\n",
           (unsigned long)f->an2_index, (unsigned long)T2);
    printf("      An3[%lu] -> FMAC_stored = 0x%lx\n",
           (unsigned long)f->an3_index, (unsigned long)FMAC_stored);
    printf("      j[%lu]   -> offset= 0x%lx\n",
           (unsigned long)f->jump_index, (unsigned long)offset);
    printf("      reconstructed I_j = 0x%lx\n",
           (unsigned long)I_j);
    printf("      FMAC_now (recomputed) = 0x%lx\n",
           (unsigned long)FMAC_now);

    if (FMAC_now != FMAC_stored) {
        printf("    -> FMAC mismatch! Code or address or caller tampered.\n");
        printf("       (no dispatch)\n");
    } else {
        printf("    -> FMAC verified. Integrity OK.\n");
        printf("       (not jumping to I_j, demo only)\n");
        // If you want to make it live:
        // ((void (*)())I_j)();
    }

    // 5. After each lookup, trigger a reshuffle
    dmei_build_table();
}

// ----------------------------
// MAIN
// ----------------------------
int main(void) {
    printf("=== DMEI PoC: FMAC verified, per-lookup reshuffle ===\n");

    // initial build (epoch 1)
    dmei_build_table();

    // valid caller for S=0x01
    dmei_lookup_demo(0x01, 0x7fff00001000ULL);

    // another lookup (same S, same IP) -> new epoch, new secrets, new table
    dmei_lookup_demo(0x01, 0x7fff00001000ULL);

    // invalid caller IP (will fail IP check)
    dmei_lookup_demo(0x01, 0x7fff00009999ULL);

    return 0;
}

