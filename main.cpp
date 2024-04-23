#include <bits/stdc++.h>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <sys/mman.h>
#include <unistd.h>
#include <vector>

struct MemoryPage {
  uint8_t *mem;     // pointer to the start of the memory
  size_t page_size; // size of the page (taken from sysconf(_SC_PAGE_SIZE))
  size_t page_nums; // number of pages requested
  size_t position;  // current position to the non-used memory

  MemoryPage(size_t pages_requested = 1) {
    page_size = sysconf(_SC_PAGE_SIZE);
    position = 0;
    mem = (uint8_t *)mmap(nullptr, page_size * pages_requested,
                          PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
      throw std::runtime_error("Failed to allocate memory");
    }

    page_nums = pages_requested;
  }

  ~MemoryPage() { munmap(mem, page_size * page_nums); }

  void check_available_space(size_t size) {
    if (position + size > page_size * page_nums) {
      throw std::runtime_error("Out of memory");
    }
  }

  void push(uint8_t data) {
    check_available_space(sizeof data);

    mem[position++] = data;
  }

  void push(void (*fn)()) {
    size_t fn_add = reinterpret_cast<size_t>(fn);
    check_available_space(sizeof fn_add);

    std::memcpy(mem + position, &fn_add, sizeof fn_add);
    position += sizeof fn_add;
  }

  void push(std::vector<uint8_t> &data) {
    check_available_space(data.size());

    std::memcpy(mem + position, data.data(), data.size());
    position += data.size();
  }

  void showMemory() {
    std::cout << "Memory content: " << position << "/" << page_size * page_nums
              << " bytes used\n";

    std::cout << std::hex;
    for (size_t i = 0; i < position; i++) {
      std::cout << "0x" << (int)mem[i] << " ";
      if (i % 16 == 0 && i > 0) {
        std::cout << '\n';
      }
    }

    std::cout << std::dec;
    std::cout << '\n';
  }
};

std::vector<int> a{1, 2, 3};

void test() {
  std::cout << "Test\n";
  for (auto &n : a) {
    n -= 5;
  }
}

namespace AsemblyChunks {
std::vector<uint8_t> function_prologue = {
    0x55,            // push rbp
    0x48, 0x89, 0xe5 // mov rbp, rsp
};

std::vector<uint8_t> function_epilogue = {
    0x5d, // pop rbp
    0xc3  // ret
};
} // namespace AsemblyChunks

int main() {
  MemoryPage mp;

  mp.push(AsemblyChunks::function_prologue);

  mp.push(0x48);
  mp.push(0xb8);
  mp.push(test); // mov rax, <function address>

  mp.push(0xff);
  mp.push(0xd0); // call rax

  mp.push(AsemblyChunks::function_epilogue);
  mp.showMemory();

  std::cout << "Initial value: ";
  for (auto &n : a) {
    std::cout << n << " ";
  }
  std::cout << '\n';

  void (*fn)() = (void (*)())mp.mem;
  fn();

  std::cout << "After function call: ";
  for (auto &n : a) {
    std::cout << n << " ";
  }
  std::cout << '\n';

  return 0;
}
