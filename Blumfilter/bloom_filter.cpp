#include "bloom_filter.h"

#include <stdlib.h>
#include <string.h>

uint64_t calc_hash(const char* str, uint64_t modulus, uint64_t seed) {
  uint64_t calc_hash = 0;
  uint32_t str_length = strlen(str);
  for (uint32_t ind = 0; ind != str_length; ++ind) {
    if (modulus != 0) {
      calc_hash = (calc_hash * seed + str[ind]) % modulus;
    }
  }
  return calc_hash;
}

void bloom_init(struct BloomFilter* bloom_filter, uint64_t set_size,
                hash_fn_t hash_fn, uint64_t hash_fn_count) {
  bloom_filter->set_size = set_size;
  bloom_filter->hash_fn = hash_fn;
  bloom_filter->hash_fn_count = hash_fn_count;

  uint32_t words_num =
      (set_size + 63) /
      64;  //количество 64-битных слов необходимых для хранения ввсех битов
  bloom_filter->set = (uint64_t*)calloc(words_num, sizeof(uint64_t));
}

void bloom_destroy(struct BloomFilter* bloom_filter) {
  free(bloom_filter->set);
  bloom_filter->set = NULL;
}

void bloom_insert(struct BloomFilter* bloom_filter, Key key) {
  for (uint32_t ind = 0; ind != bloom_filter->hash_fn_count; ++ind) {
    uint64_t hash_value =
        bloom_filter->hash_fn(key, bloom_filter->set_size, ind + 1);
    bloom_filter->set[hash_value / 64] |= (1ull << (hash_value % 64));
  }
}

bool bloom_check(struct BloomFilter* bloom_filter, Key key) {
  for (uint32_t ind = 0; ind < bloom_filter->hash_fn_count; ++ind) {
    uint64_t hash = bloom_filter->hash_fn(key, bloom_filter->set_size, ind + 1);
    if ((bloom_filter->set[hash / 64] & (1ull << (hash % 64))) == 0) {
      return false;
    }
  }
  return true;
}