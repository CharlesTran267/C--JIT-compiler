#Enfore Intel Syntax
.intel_syntax noprefix

# Call the "write" system function (man 2 write)
# ssize_t write(int fd, const void *buf, size_t count);

mov rax, 1 # Store the "write" system call number 0x1 in rax
mov rdi, 1 # Store the file descriptor 1 (stdout) in rdi
lea rsi, [rip + 0xa] # Store the location of the string (0xa bytes after the current instruction) in rsi

mov rdx, 17 # Store the length of the string in rdx
syscall
ret
.string "Hello Your Name here\n"
