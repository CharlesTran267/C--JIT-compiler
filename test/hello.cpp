#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

void append_message(std::vector<uint8_t> &machine_code,
                    const std::string &name) {
  size_t message_size = name.size();
  machine_code[24] = (message_size & 0xff) >> 0;
  machine_code[25] = (message_size & 0xff00) >> 8;
  machine_code[26] = (message_size & 0xff0000) >> 16;
  machine_code[27] = (message_size & 0xff000000) >> 24;

  for (auto c : name) {
    machine_code.emplace_back(c);
  }
}

size_t estimate_memory_size(size_t machine_code_size) {
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t factor = 1, required_size;

  while (true) {
    required_size = page_size * factor;
    if (required_size >= machine_code_size) {
      break;
    }
    factor++;
  }

  return required_size;
}

int main() {
  std::cout << "What is your name?\n";
  std::string name;
  std::getline(std::cin, name);

  std::vector<uint8_t> machine_code{
      0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x48, 0xc7, 0xc7, 0x01,
      0x00, 0x00, 0x00, 0x48, 0x8d, 0x35, 0x0a, 0x00, 0x00, 0x00, 0x48,
      0xc7, 0xc2, 0x11, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3};

  // append message to machine code
  append_message(machine_code, name);

  // Get the required memory size for mmap
  size_t required_mem_size = estimate_memory_size(machine_code.size());

  uint8_t *mem = (uint8_t *)mmap(nullptr, required_mem_size,
                                 PROT_READ | PROT_WRITE | PROT_EXEC,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (mem == MAP_FAILED) {
    std::cerr << "Failed to allocate memory\n";
    return 1;
  }

  // Copy the machine code to the allocated memory
  std::copy(machine_code.begin(), machine_code.end(), mem);

  // Execute the machine code
  void (*func)() = (void (*)())mem;
  func();

  // Release the allocated memory
  munmap(mem, required_mem_size);

  return 0;
}
