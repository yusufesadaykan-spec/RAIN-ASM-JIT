
# RAIN: Multi-Architecture JIT Byte Encoder Engine 🌧️⚡

RAIN is a zero-dependency, ultra-fast, header-only JIT (Just-In-Time) compiler core and machine code emitter written in pure C99. It allows you to programmatically compile assembly-structured instructions into executable, raw native machine code arrays for **x86_64**, **ARM64 (AArch64)**, and **RISC-V (RV64GC)** architectures simultaneously.

RAIN completely bypasses heavy intermediate representation graphs (like LLVM IR), performing high-speed bitwise transformation from structural functional calls straight to hardware-executable bytes. It is specifically tailored for lightweight virtual machine backends, custom scripting runtimes, emulator pipelines, or custom silicon design emulation (such as 2.4GHz FPGA SoC architectures).

---

## 🚀 What's New in the Latest Version

* **Advanced Host C-Function FFI Bridge:** Native capability to branch out from JIT-compiled byte arrays straight into host C functions (e.g., `printf`, `malloc`) using safe hardware execution boundaries.
* **Symbolic Reference Lookups:** Integrated symbol lookup engine via `rain_register_c_func` and `rain_get_c_func` to dynamically match runtime text strings to active memory function pointers.
* **Architecture-Specific C Call Macros:** Zero-overhead invocation bridges optimized for ABI register structures:
  * `rain_x86_call_c(bin, "function_name")` -> Emits dynamic `mov` to intermediate registers followed by a near `call`.
  * `rain_arm64_call_c(bin, "function_name")` -> Bridges address pointers via register `x16` utilizing branch-with-link (`blr`).
  * `rain_riscv_call_c(bin, "function_name")` -> Emits custom load-immediate (`li`) across temporary register `x5` tracking indirect jump-and-link (`jalr`).
* **Expanded Hardware Instructions:** Full native bit-mask tracking for RISC-V `rain_riscv_li` pseudo-ops and strict ARM64 register state validation layouts.

---

## Technical Architecture Pipeline




```
         [ User-Defined Dynamic Assembly Instructions ]
                               │
                               ▼
               [ RAIN Macro Compilation Pipeline ]
                               │
     ┌─────────────────────────┼─────────────────────────┐
     ▼                         ▼                         ▼
[ x86_64 ]                 [ ARM64 ]                 [ RISC-V ]

```

(Intel / AMD)             (Apple Silicon)          (Custom 64-bit SoC)
(REX/ModRM/SIB)            (32-bit Encoded)          (Standard RV64GC)

---

## Core Structures & Memory Management

### 1. The Execution Buffer (`rain_bin_t`)
A dynamically resizing memory arena tracking accumulated machine byte arrays.
* `void rain_bin_init(rain_bin_t *bin)`: Resets sizes and safely zeroes memory data pointer parameters.
* `void rain_bin_free(rain_bin_t *bin)`: Reclaims allocated heap segments without risking system memory leaks.
* `void rain_bin_append(rain_bin_t *bin, const uint8_t *bytes, size_t len)`: Automatically monitors capacity boundaries, performing standard data vector copying to grow the allocation boundary as instructions assemble.

### 2. Universal Operands System (`rain_operand_t`)
Strict algebraic types defining command input targets:
* `RAIN_OP_REG`: Points directly to an architecture-specific physical hardware register core.
* `RAIN_OP_IMM`: Packs immediate signed or unsigned constants into hardware-compliant offsets.
* `RAIN_OP_MEM`: Sets base register memory maps combined with user-defined bit offset displacements.

---

## Architecture Instruction Specifications

### A. Intel / AMD x86_64 Compiler Platform
* **Supported Commands:** `mov`, `add`, `sub`, `mul`, `div`, `push`, `pop`, `ret` (`0xC3`), `xor`, `cmp`, `jmp`, `jz`, `je`, `jl`, `jg`, `call`.
* **Register Matrix Support:** Fully mapped from 8-bit registers up to complete 64-bit hardware footprints:
  * *64-bit Registers:* `rain_x86_64_rax`, `rcx`, `rdx`, `rbx`, `rsp`, `rbp`, `rsi`, `rdi`, `r8` through `r15`.
  * *32-bit Downscales:* `rain_x86_64_eax`, `ecx`, `edx`, `ebx`, etc.
  * *16-bit Downscales:* `rain_x86_64_ax`, `cx`, `dx`, `bx`, etc.
  * *8-bit Downscales:* `rain_x86_64_al`, `cl`, `dl`, `bl`.

### B. ARM64 / AArch64 Compiler Platform
* **Supported Commands:** `mov` (supporting internal bit shifts), `add`, `sub`, `mul` (mapped to native hardware `MADD` bit fields), `div` (`SDIV`/`UDIV`), `ldr` (Load Register), `str` (Store Register), `ret` (`0xD65F03C0`), `blr` (Branch with Link to Register).
* **Register Matrix Support:** * *General Purpose:* `rain_arm64_x0` directly through to `rain_arm64_x30`.
  * *System Registers:* `rain_arm64_sp` (Stack Pointer) and `rain_arm64_xzr` (Hardwired Zero Register).

### C. RISC-V 64-bit Platform (RV64GC)
* **Supported Commands:** `add`, `addi`, `sub`, `mul`, `div`, `lw` (Load Word), `ld` (Load Doubleword), `sw` (Store Word), `sd` (Store Doubleword), `jal` (Jump and Link), `jalr` (Jump and Link Register), `ret` (`0x00008067`), `ecall` (System Trap Environment Call), `nop` (`0x00000013`), `li` (Load Immediate wrapper).
* **Register Matrix Support:** Compliant with RISC-V hardware ABI definitions:
  * `rain_riscv_x0` / `zero` (Hardwired Constant Zero).
  * `rain_riscv_x1` / `ra` (Function Call Return Link Address).
  * `rain_riscv_x2` / `sp` (Active System Stack Pointer).
  * Parameter Passing Lanes: `rain_riscv_x10` through `rain_riscv_x17` (`a0` to `a7`), extending cleanly out through register token `rain_riscv_x31`.

---

## Standard Integration Example (JIT Compilation + Native FFI Call)

This production-grade, compilable verification example (`rain_demo.c`) details how to initiate execution buffers, write architecture-independent structural logic, and trigger a host callback via the **new native FFI interface**.

```c
#include <stdio.h>
#include "rain.h"

// The Native Host Function called directly from inside JIT machine bytes
void native_callback() {
    printf("[FFI SUCCESS] Securely branched from inside RAIN JIT stream to Host C Engine!\\n");
}

void render_hex_output(const char *target_arch, const rain_bin_t *bin) {
    printf("=== %s Target Array Stack (%zu bytes) ===\\n", target_arch, bin->size);
    for (size_t i = 0; i < bin->size; i++) {
        printf("%02X ", bin->data[i]);
    }
    printf("\\n\\n");
}

int main() {
    printf("=== RAIN Multi-Target Emitter Activation Engine ===\\n\\n");

    // CRITICAL: Bind the text identifier to the memory layout address map
    rain_register_c_func("native_callback", (void*)&native_callback);

    // ------------------------------------------------------------------------
    // 1. INTEL/AMD x86_64 Pipeline Compilation Flow
    // ------------------------------------------------------------------------
    rain_bin_t x86_program;
    rain_bin_init(&x86_program);

    // mov rax, 42
    rain_x86_64_compiler(&x86_program, rain_x86_64_mov, rain_x86_64_rax, rain_x86_64_imd(42));
    // add rax, rcx
    rain_x86_64_compiler(&x86_program, rain_x86_64_add, rain_x86_64_rax, rain_x86_64_rcx);
    
    // CALL FFI BRIDGE: Safely escapes JIT to run host C functions
    rain_x86_call_c(&x86_program, "native_callback");

    // ret
    rain_x86_64_compiler(&x86_program, rain_x86_64_ret);
    render_hex_output("x86_64 Architecture", &x86_program);
    rain_bin_free(&x86_program);

    // ------------------------------------------------------------------------
    // 2. ARM64 (AArch64 / Apple Silicon / Raspberry Pi) Flow
    // ------------------------------------------------------------------------
    rain_bin_t arm64_program;
    rain_bin_init(&arm64_program);

    // mov x0, 100
    rain_arm64_compiler(&arm64_program, rain_arm64_mov, rain_arm64_x0, rain_arm64_imd(100));
    // add x0, x0, x1
    rain_arm64_compiler(&arm64_program, rain_arm64_add, rain_arm64_x0, rain_arm64_x0, rain_arm64_x1);
    
    // CALL FFI BRIDGE
    rain_arm64_call_c(&arm64_program, "native_callback");

    // ret
    rain_arm64_compiler(&arm64_program, rain_arm64_ret);
    render_hex_output("ARM64 Architecture", &arm64_program);
    rain_bin_free(&arm64_program);

    // ------------------------------------------------------------------------
    // 3. RISC-V (RV64GC Custom Silicon / Hardware Emulators) Flow
    // ------------------------------------------------------------------------
    rain_bin_t riscv_program;
    rain_bin_init(&riscv_program);

    // addi x10, x10, 50 -> (a0 = a0 + 50)
    rain_riscv_compiler(&riscv_program, rain_riscv_addi, rain_riscv_x10, rain_riscv_x10, rain_riscv_imd(50));
    
    // CALL FFI BRIDGE
    rain_riscv_call_c(&riscv_program, "native_callback");

    // ret
    rain_riscv_compiler(&riscv_program, rain_riscv_ret);
    render_hex_output("RISC-V 64 Target", &riscv_program);
    rain_bin_free(&riscv_program);

    return 0;
}

```

---

## Operating System JIT Execution Policies

To safely execute generated machine byte arrays on host operating platforms, you must request pages with absolute executable verification rights:

* **Linux / FreeBSD System Calls:** Utilize `mmap` mapping parameters containing flags `PROT_READ | PROT_WRITE | PROT_EXEC`. If kernel security systems enforce explicit `W^X` policies, allocate distinct write/execute aliased addresses or leverage `mprotect` toggles between composition and thread invocation boundaries.
* **Hardware Instruction Cache Synchronization:** On **ARM64** and **RISC-V** processor rings, the data cache and instruction cache are not hardware-coherent for dynamic code. You must explicitly flush data cache buffers using standard compiler hooks like `__builtin___clear_cache(start, end)` before moving the hardware Program Counter (`PC`) pointer into your `rain_bin_t` structure space.

---

## License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)** - feel free to review included repository headers for open-source development compliance rules.

```

```
