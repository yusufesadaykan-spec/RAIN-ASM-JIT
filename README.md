
# rain
Rain is a lightweight, dependency-free JIT compiler written in pure C code. It's a library capable of generating code specific to RISCV, x86_64, and ARM processors (more to come). You can use it to create your own compilers and dynamic libraries (there are many use cases).

# keywords
jit-compiler assembler x86-64 arm64 riscv systems-programming compiler-design hardware-architecture

# how to use

---

## Core Structures & Initialization

### Memory Buffer (`rain_bin_t`)
Dinamically growing byte buffer used to accumulate emitted machine instructions.
* `void rain_bin_init(rain_bin_t *bin)`: Allocates or sets null states for a new binary container.
* `void rain_bin_free(rain_bin_t *bin)`: Safely releases the internal data cache from standard heap.
* `void rain_bin_append(rain_bin_t *bin, const uint8_t *bytes, size_t len)`: Appends raw bytes directly to the execution pipeline.

### Operand System (`rain_operand_t`)
Encapsulates register codes, immediate integers, and explicit memory offsets.
* `RAIN_OP_REG`: Physical register tracking.
* `RAIN_OP_IMM`: Raw tamsayı limits (8-bit to 64-bit bounds).
* `RAIN_OP_MEM`: Base register with custom scale displacement.

---

## Comprehensive Instruction Set & API Commands

RAIN utilizes a macro-overloading mechanism (`RAIN_GET_MACRO`). You call the same function identifier, and C automatically evaluates the argument count to call the appropriate internal layout encoder signature (`_2`, `_3`, `_4`).

### 1. x86_64 Architecture Reference
Call Wrapper: `rain_x86_64_compiler(bin, command, ...)`

#### Available Instruction Opcodes (`rain_x86_64_cmd_t`):
* `rain_x86_64_mov`: Move source to destination register or imm byte.
* `rain_x86_64_add`: Binary integer addition.
* `rain_x86_64_sub`: Binary integer subtraction.
* `rain_x86_64_mul`: Unsigned/Signed multiplication via ModRM extension.
* `rain_x86_64_div`: Unsigned/Signed division.
* `rain_x86_64_push`: Pushes register operand into Stack frame.
* `rain_x86_64_pop`: Pops top stack value out to designated destination register.
* `rain_x86_64_ret`: Return from near procedure (Emits `0xC3`).
* `rain_x86_64_xor`: Bitwise logical exclusive OR.
* `rain_x86_64_cmp`: Compares operands by setting internal EFLAGS status registers.
* `rain_x86_64_jmp`: Unconditional near code jumping via raw displacement values.
* `rain_x86_64_jz`: Jump near if zero status condition flag is toggled.
* `rain_x86_64_je`: Jump near if equal condition matches.
* `rain_x86_64_jl`: Jump near if arithmetic comparison is less than target.
* `rain_x86_64_jg`: Jump near if arithmetic comparison is greater than target.
* `rain_x86_64_call`: Executes system relative or absolute location calls.

#### Physical Register Map Reference (`rain_x86_64_reg_t`):
* **64-Bit General Registers:** `rain_x86_64_rax`, `rain_x86_64_rcx`, `rain_x86_64_rdx`, `rain_x86_64_rbx`, `rain_x86_64_rsp`, `rain_x86_64_rbp`, `rain_x86_64_rsi`, `rain_x86_64_rdi`, `rain_x86_64_r8` through `rain_x86_64_r15`.
* **32-Bit Registers:** `rain_x86_64_eax`, `rain_x86_64_ecx`, `rain_x86_64_edx`, etc.
* **16-Bit Registers:** `rain_x86_64_ax`, `rain_x86_64_cx`, `rain_x86_64_dx`, etc.
* **8-Bit Low Registers:** `rain_x86_64_al`, `rain_x86_64_cl`, `rain_x86_64_dl`, `rain_x86_64_bl`.

---

### 2. ARM64 / AArch64 Architecture Reference
Call Wrapper: `rain_arm64_compiler(bin, command, ...)`

#### Available Instruction Opcodes (`rain_arm64_cmd_t`):
* `rain_arm64_mov`: Moves imm16 shifts or basic register states.
* `rain_arm64_add`: Register + Register or Register + Imm12 arithmetic addition.
* `rain_arm64_sub`: Register - Register or Register - Imm12 arithmetic subtraction.
* `rain_arm64_mul`: Emits 32-bit hardware multiply sequences (`MADD` mapped mask).
* `rain_arm64_div`: Hardware integer signed/unsigned division commands (`SDIV`/`UDIV`).
* `rain_arm64_ldr`: Load register values from base index memory scopes.
* `rain_arm64_str`: Store register value directly into target hardware memory locations.
* `rain_arm64_ret`: Subroutine return execution (Emits unconditional link branch pointer `0xD65F03C0`).

#### Physical Register Map Reference (`rain_arm64_reg_t`):
* **64-Bit Core Registers:** `rain_arm64_x0` through `rain_arm64_x30`.
* **Special Registers:** `rain_arm64_sp` (Stack Pointer), `rain_arm64_xzr` (Zero Constant Register).

---

### 3. RISC-V Architecture Reference
Call Wrapper: `rain_riscv_compiler(bin, command, ...)`

#### Available Instruction Opcodes (`rain_riscv_cmd_t`):
* `rain_riscv_add` / `rain_riscv_addi`: Register-Register / Register-Immediate sign-extended additions.
* `rain_riscv_sub`: Traditional two's complement subtract routine blocks.
* `rain_riscv_mul`: R-Type standard base extension signed multiply module mapping.
* `rain_riscv_div`: Hardware-level signed division core instructions.
* `rain_riscv_lw`: Load Word (32-bit offset mapping extraction).
* `rain_riscv_ld`: Load Doubleword (64-bit standard base loading).
* `rain_riscv_sw`: Store Word payload down into aligned target sectors.
* `rain_riscv_sd`: Store Doubleword state pipeline tracking.
* `rain_riscv_jal`: Jump And Link (Relative distance code branch mechanism).
* `rain_riscv_jalr`: Jump And Link Register (Indirect function jumps).
* `rain_riscv_ret`: Pseudo-instruction expansion to return immediately (Emits `0x00008067`).
* `rain_riscv_ecall`: Operating system environment trap interface triggers (Emits `0x00000073`).
* `rain_riscv_nop`: No-operation instruction (Emits base code `0x00000013`).

#### Physical Register Map Reference (`rain_riscv_reg_t`):
* `rain_riscv_x0` / `rain_riscv_zero`: Hardwired zero value constant.
* `rain_riscv_x1` / `rain_riscv_ra`: Function execution link return address.
* `rain_riscv_x2` / `rain_riscv_sp`: Active system stack alignment frame pointer.
* `rain_riscv_x10` - `rain_riscv_x17` (`rain_riscv_a0` - `rain_riscv_a7`): Function entry parameters and return results.
* Full sequence tracking available from `rain_riscv_x0` directly through to `rain_riscv_x31`.

---

## Step-by-Step Programming Example

Below is a complete, compilable C verification file (`main.c`) demonstrating how to use `rain.h` to programmatically emit machine instructions for all three hardware architectures inside a single application.

```c
#include <stdio.h>
#include "rain.h"

void dump_binary_hex(const char *title, const rain_bin_t *bin) {
    printf("--- %s Hex Dump (%zu bytes) ---\\n", title, bin->size);
    for (size_t i = 0; i < bin->size; i++) {
        printf("%02X ", bin->data[i]);
        if ((i + 1) % 16 == 0) printf("\\n");
    }
    if (bin->size % 16 != 0) printf("\\n");
    printf("\\n");
}

int main() {
    // ------------------------------------------------------------------------
    // Scenario 1: Target Generation for Intel/AMD x86_64
    // ------------------------------------------------------------------------
    rain_bin_t x86_program;
    rain_bin_init(&x86_program);

    // mov rax, 100         -> Load immediate 100 into RAX register
    rain_x86_64_compiler(&x86_program, rain_x86_64_mov, rain_x86_64_rax, rain_x86_64_imd(100));
    
    // add rax, rcx         -> Add values inside RCX to RAX
    rain_x86_64_compiler(&x86_program, rain_x86_64_add, rain_x86_64_rax, rain_x86_64_rcx);
    
    // ret                 -> Near procedure exit sequence
    rain_x86_64_compiler(&x86_program, rain_x86_64_ret);

    dump_binary_hex("x86_64 JIT Pipeline Output", &x86_program);
    rain_bin_free(&x86_program);

    // ------------------------------------------------------------------------
    // Scenario 2: Target Generation for ARM64 (AArch64 / Apple Silicon / Pi)
    // ------------------------------------------------------------------------
    rain_bin_t arm64_program;
    rain_bin_init(&arm64_program);

    // mov x0, #42          -> Load intermediate 42 directly to X0
    rain_arm64_compiler(&arm64_program, rain_arm64_mov, rain_arm64_x0, rain_arm64_imd(42));
    
    // add x0, x0, x1       -> Standard core registration sum mapping
    rain_arm64_compiler(&arm64_program, rain_arm64_add, rain_arm64_x0, rain_arm64_x0, rain_arm64_x1);
    
    // ret                 -> Branch register link code completion
    rain_arm64_compiler(&arm64_program, rain_arm64_ret);

    dump_binary_hex("ARM64 JIT Pipeline Output", &arm64_program);
    rain_bin_free(&arm64_program);

    // ------------------------------------------------------------------------
    // Scenario 3: Target Generation for Custom RISC-V Processors / Emulators
    // ------------------------------------------------------------------------
    rain_bin_t riscv_program;
    rain_bin_init(&riscv_program);

    // addi x10, x10, 5     -> Add immediate 5 directly inside argument register A0
    rain_riscv_compiler(&riscv_program, rain_riscv_addi, rain_riscv_x10, rain_riscv_x10, rain_riscv_imd(5));
    
    // ret                 -> Unconditional target jump pipeline clearance
    rain_riscv_compiler(&riscv_program, rain_riscv_ret);

    dump_binary_hex("RISC-V Custom SoC JIT Pipeline Output", &riscv_program);
    rain_bin_free(&riscv_program);

    return 0;
}
