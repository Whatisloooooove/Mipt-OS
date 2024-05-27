#include "falloc.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void falloc_init(file_allocator_t* allocator, const char* filepath,
                 uint64_t allowed_page_count) {
  int fd = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return;
  }

  off_t size = PAGE_SIZE * (allowed_page_count + 8);
  allocator->fd = fd;
  if (ftruncate(fd, size) == -1) {
    close(fd);
    return;
  }
  allocator->base_addr = (mmap(NULL, PAGE_SIZE * (allowed_page_count + 8),
                               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) +
                          8 * PAGE_SIZE);

  allocator->page_mask = (uint64_t*)(allocator->base_addr - 8 * PAGE_SIZE);

  allocator->allowed_page_count = allowed_page_count;
  uint64_t last_pages = allocator->curr_page_count;
  allocator->curr_page_count = 0;

  uint64_t count = 0;
  for (uint64_t ind = 0; ind < PAGE_MASK_SIZE; ++ind) {
    uint64_t mask = allocator->page_mask[ind];
    count = 0;
    while (mask) {
      mask &= (mask - 1);
      count++;
    }
    allocator->curr_page_count += count;
  }
}

void falloc_destroy(file_allocator_t* allocator) {
  munmap(allocator->page_mask, PAGE_SIZE * (allocator->allowed_page_count + 8));
  allocator->base_addr = NULL;
  allocator->page_mask = NULL;
  close(allocator->fd);
}

void* falloc_acquire_page(file_allocator_t* allocator) {
  if (allocator->allowed_page_count != allocator->curr_page_count) {
    for (uint64_t ind_page = 0; ind_page < PAGE_MASK_SIZE; ++ind_page) {
      for (uint64_t bit = 0; bit < 64; ++bit) {
        uint64_t bit_mask = 1ULL << bit;
        if (!(allocator->page_mask[ind_page] & bit_mask)) {
          allocator->page_mask[ind_page] |= bit_mask;
          ++allocator->curr_page_count;
          return (allocator->base_addr + (PAGE_SIZE * (bit + ind_page * 64)));
        }
      }
    }
  }
  return NULL;
}

void falloc_release_page(file_allocator_t* allocator, void** addr) {
  uintptr_t base = (uintptr_t)allocator->base_addr;
  uintptr_t address = (uintptr_t)(*addr);
  if (address >= base) {
    size_t offset = address - base;
    uint64_t page_index = offset / PAGE_SIZE;
    allocator->page_mask[page_index / 64] &= ~(1ULL << (page_index % 64));
    allocator->curr_page_count--;
    *addr = NULL;
  }
}