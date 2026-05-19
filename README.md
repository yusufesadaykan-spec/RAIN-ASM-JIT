# RAIN (Multi-Architecture JIT Byte Encoder) 🌧️

RAIN is a zero-dependency, header-only, lightning-fast JIT (Just-In-Time) compiler core and machine code emitter written in pure C99. It programmatically compiles assembly-like instruction structures into executable, raw native machine code arrays for **x86_64**, **ARM64 (AArch64)**, and **RISC-V (RV64GC)** architectures simultaneously.

Whether you are building a custom programming language compiler backend, an optimized virtual machine, an emulator, or a runtime execution engine running directly on custom silicon/FPGA architectures, RAIN provides a minimalist, bare-metal interface to emit valid CPU instructions on the fly.

---

## 🚀 What's New in the Latest Version

* **Advanced Native FFI (Foreign Function Interface):** RAIN now natively supports calling standard C host functions (e.g., `printf`, `malloc`, `exit`) directly from within your injected JIT compiled byte streams. 
* **Symbolic Function Registration:** Introducing `rain_register_c_func` and `rain_get_c_func` to dynamically map text labels to explicit runtime memory function pointers across all backends.
* **Architecture-Specific C Call Bridges:** New zero-overhead functional wrappers implemented:
  * `rain_x86_call_c(bin, "function_name")`
  * `rain_arm64_call_c(bin, "function_name")`
  * `rain_riscv_call_c(bin, "function_name")`
* **Expanded RISC-V Support:** Added full encoder pipeline capabilities for `rain_riscv_li` (Load Immediate pseudo-instruction handling) and `rain_riscv_jalr` linking for dynamic function jumps.

---

## Pipeline Architecture

RAIN bypasses heavy, slow intermediate formats like LLVM IR, translating logical commands straight into absolute machine bytes.

## Technical Feature Overview

* **Header-Only Integration:** Zero boilerplate. Dropping a single `#include "rain.h"` file into your codebase is all it takes.
* **No Dependencies:** Relies strictly on standard freestanding library footprints (`stdint.h`, `stdlib.h`, `string.h`).
* **Macro Overloading Engine:** Uses internal `RAIN_GET_MACRO` branching signatures to evaluate parameter lengths automatically at compile time, reducing interface clutter.
* **Safe Dynamic Buffering:** Managed via the `rain_bin_t` structure, dynamically resizing memory allocations safely as instructions accumulate.

---

## Core Structures & Memory Control API

### 1. Execution Buffer (`rain_bin_t`)
Maintains the raw memory cache array for emitted byte opcodes.
* `void rain_bin_init(rain_bin_t *bin)`: Allocates or sets null state for a safe compilation context.
* `void rain_bin_free(rain_bin_t *bin)`: Frees internal data cache from standard heap allocations safely.
* `void rain_bin_append(rain_bin_t *bin, const uint8_t *bytes, size_t len)`: Direct interface to append arbitrary custom byte sequences into the pipeline.

### 2. Universal Operands (`rain_operand_t`)
Encapsulates different structural arguments needed by hardware instructions.
* `RAIN_OP_REG`: Marks target parameter as a hardware register asset.
* `RAIN_OP_IMM`: Contains raw signed/unsigned immediate integer constants.
* `RAIN_OP_MEM`: Sets complex base-register addressing memory modes with standard offset displacements.

---

## Comprehensive Architecture Instruction Map

### A. x86_64 Core Engine (`rain_x86_64_compiler`)
* **Supported Instructions:** `mov`, `add`, `sub`, `mul` (via custom ModRM layout extensions), `div`, `push`, `pop`, `ret` (`0xC3`), `xor`, `cmp`, `jmp`, `jz`, `je`, `jl`, `jg`, and `call`.
* **Register Map:** Supports full bit-width scaling configurations:
  * *64-bit Registers:* `rain_x86_64_rax`, `rcx`, `rdx`, `rbx`, `rsp`, `rbp`, `rsi`, `rdi`, `r8` through `r15`.
  * *32-bit Registers:* `rain_x86_64_eax`, `ecx`, `edx`, etc.
  * *16-bit Registers:* `rain_x86_64_ax`, `cx`, `dx`, etc.
  * *8-bit Registers:* `rain_x86_64_al`, `cl`, `dl`, `bl`.

### B. ARM64 / AArch64 Engine (`rain_arm64_compiler`)
* **Supported Instructions:** `mov` (shift variations optimized), `add`, `sub`, `mul` (mapped through hardware `MADD` masks), `div` (`SDIV`/`UDIV` choices), `ldr` (load register), `str` (store register), `ret` (`0xD65F03C0`), and `blr` (Branch with Link to Register).
* **Register Map:** * *Core general-purpose:* `rain_arm64_x0` through `rain_arm64_x30`.
  * *Specialized:* `rain_arm64_sp` (Stack Pointer) and `rain_arm64_xzr` (Zero Constant Register).

### C. RISC-V 64 Engine (`rain_riscv_compiler`)
* **Supported Instructions:** `add`, `addi`, `sub`, `mul`, `div`, `lw` (Load Word), `ld` (Load Doubleword), `sw` (Store Word), `sd` (Store Doubleword), `jal` (Jump and Link), `jalr` (Jump and Link Register), `ret` (`0x00008067`), `ecall` (System Environment Trap), `nop` (`0x00000013`), and `li` (Load Immediate pseudo-op scaling).
* **Register Map:** Fully compliant with standard RISC-V ABI conventions:
  * `rain_riscv_x0` / `zero` (Hardwired Zero).
  * `rain_riscv_x1` / `ra` (Return Link Address).
  * `rain_riscv_x2` / `sp` (Stack Frame Pointer).
  * `rain_riscv_x10` through `rain_riscv_x17` (`a0` - `a7` parameter lanes), spanning out completely through `rain_riscv_x31`.

---

## Production Verification Example (JIT + Native FFI)

This complete, standalone C program showcases how to emit code for all platforms and demonstrates the **new FFI capability** to branch straight into a native C host function from your JIT engine context.

---

```c
#include <stdio.h>
#include "rain.h"

// The native C function we want to invoke from within the JIT execution stream
void host_print_callback() {
    printf("[FFI Match] Hello from RAIN's JIT Environment Core!\\n");
}

void print_hex_output(const char *platform_name, const rain_bin_t *bin) {
    printf("=== %s Machine Byte Array (%zu bytes) ===\\n", platform_name, bin->size);
    for (size_t i = 0; i < bin->size; i++) {
        printf("%02X ", bin->data[i]);
    }
    printf("\\n\\n");
}

int main() {
    // ------------------------------------------------------------------------
    // SETUP: Register the Host C function into RAIN's Internal Symbol Map
    // ------------------------------------------------------------------------
    rain_register_c_func("host_print_callback", (void*)&host_print_callback);

    // ------------------------------------------------------------------------
    // 1. INTEL/AMD x86_64 JIT Pipeline with Native FFI Call
    // ------------------------------------------------------------------------
    rain_bin_t x86_ctx;
    rain_bin_init(&x86_ctx);

    // mov rax, 500
    rain_x86_64_compiler(&x86_ctx, rain_x86_64_mov, rain_x86_64_rax, rain_x86_64_imd(500));
    // add rax, rcx
    rain_x86_64_compiler(&x86_ctx, rain_x86_64_add, rain_x86_64_rax, rain_x86_64_rcx);
    
    // NEW: Call native C function safely bridging System V ABI parameters
    rain_x86_call_c(&x86_ctx, "host_print_callback");
    
    // ret
    rain_x86_64_compiler(&x86_ctx, rain_x86_64_ret);
    print_hex_output("x86_64 Target", &x86_ctx);
    rain_bin_free(&x86_ctx);

    // ------------------------------------------------------------------------
    // 2. ARM64 (Apple Silicon / Raspberry Pi) Target Stream
    // ------------------------------------------------------------------------
    rain_bin_t arm64_ctx;
    rain_bin_init(&arm64_ctx);

    // mov x0, 75
    rain_arm64_compiler(&arm64_ctx, rain_arm64_mov, rain_arm64_x0, rain_arm64_imd(75));
    // add x0, x0, x1
    rain_arm64_compiler(&arm64_ctx, rain_arm64_add, rain_arm64_x0, rain_arm64_x0, rain_arm64_x1);
    
    // NEW: Branch out directly to native C host function
    rain_arm64_call_c(&arm64_ctx, "host_print_callback");
    
    // ret
    rain_arm64_compiler(&arm64_ctx, rain_arm64_ret);
    print_hex_output("ARM64 Target", &arm64_ctx);
    rain_bin_free(&arm64_ctx);

    // ------------------------------------------------------------------------
    // 3. RISC-V 64 Custom Processor / Emulator Core Target
    // ------------------------------------------------------------------------
    rain_bin_t riscv_ctx;
    rain_bin_init(&riscv_ctx);

    // addi a0, a0, 25
    rain_riscv_compiler(&riscv_ctx, rain_riscv_addi, rain_riscv_x10, rain_riscv_x10, rain_riscv_imd(25));
    
    // NEW: Jump and load via RISC-V runtime FFI execution map
    rain_riscv_call_c(&riscv_ctx, "host_print_callback");
    
    // ret
    rain_riscv_compiler(&riscv_ctx, rain_riscv_ret);
    print_hex_output("RISC-V Custom SoC Target", &riscv_ctx);
    rain_bin_free(&riscv_ctx);

    return 0;
}
