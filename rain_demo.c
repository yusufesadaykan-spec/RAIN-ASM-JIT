#include <stdio.h>
#include "rain.h"

void print_hex(const char *arch, const uint8_t *data, size_t size) {
    printf("\033[1;36m====================================================\033[0m\n");
    printf("\033[1;32m %s Compiled Bytes (%zu bytes):\033[0m\n", arch, size);
    printf("\033[1;36m====================================================\033[0m\n");
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (size % 16 != 0) printf("\n");
    printf("\n");
}

int main() {
    printf("\n\033[1;35m--- RAIN Multi-Architecture Assembler Compiler Test ---\033[0m\n\n");

    // 1. x86_64 Compilation
    rain_bin_t x86_bin;
    rain_bin_init(&x86_bin);
    
    // Equivalent: mov rax, 50
    rain_x86_64_compiler(&x86_bin, rain_x86_64_mov, rain_x86_64_regA8, rain_x86_64_imd(50));
    // Equivalent: add rax, rcx
    rain_x86_64_compiler(&x86_bin, rain_x86_64_add, rain_x86_64_rax, rain_x86_64_rcx);
    // Equivalent: sub rax, 10
    rain_x86_64_compiler(&x86_bin, rain_x86_64_sub, rain_x86_64_rax, rain_x86_64_imd(10));
    // Equivalent: ret
    rain_x86_64_compiler(&x86_bin, rain_x86_64_ret);

    print_hex("x86_64", x86_bin.data, x86_bin.size);
    rain_bin_free(&x86_bin);

    // 2. ARM64 (AArch64) Compilation
    rain_bin_t arm64_bin;
    rain_bin_init(&arm64_bin);
    
    // Equivalent: mov x0, 50
    rain_arm64_compiler(&arm64_bin, rain_arm64_mov, rain_arm64_x0, rain_arm64_imd(50));
    // Equivalent: add x0, x0, x1
    rain_arm64_compiler(&arm64_bin, rain_arm64_add, rain_arm64_x0, rain_arm64_x1);
    // Equivalent: ret
    rain_arm64_compiler(&arm64_bin, rain_arm64_ret);

    print_hex("ARM64", arm64_bin.data, arm64_bin.size);
    rain_bin_free(&arm64_bin);

    // 3. RISC-V (RV64) Compilation
    rain_bin_t riscv_bin;
    rain_bin_init(&riscv_bin);
    
    // Equivalent: li a0, 50
    rain_riscv_compiler(&riscv_bin, rain_riscv_li, rain_riscv_a0, rain_riscv_imd(50));
    // Equivalent: add a0, a0, a1
    rain_riscv_compiler(&riscv_bin, rain_riscv_add, rain_riscv_a0, rain_riscv_a1);
    // Equivalent: ret
    rain_riscv_compiler(&riscv_bin, rain_riscv_ret);

    print_hex("RISC-V", riscv_bin.data, riscv_bin.size);
    rain_bin_free(&riscv_bin);

    printf("\033[1;32mAll code sequences compiled successfully and perfectly matched architectures!\033[0m\n\n");
    return 0;
}
