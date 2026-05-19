
# SOFT & RAIN: Multi-Architecture Transpiler & JIT Compiler Engine 🌧️⚡

SOFT & RAIN is a high-performance, zero-dependency, header-only JIT compiler core and low-level runtime infrastructure written in pure C99. It bridges a high-level pseudo-assembly virtual interface (**SOFT IR**) with ultra-fast, native hardware machine code emission (**RAIN Encoder Matrix**) targeting **x86_64**, **ARM64 (AArch64)**, and **RISC-V (RV64GC)** hardware architectures simultaneously.

The ecosystem is split into two co-dependent layers:
1. **SOFT Platform (`soft.h`):** A domain-aware, safe stack-virtual machine executing abstract structural register and memory commands.
2. **RAIN Core Engine (`rain.h`):** A zero-abstraction bitwise macro-overloaded JIT byte encoder compiling operations directly to hardware-executable arrays.

---

## 🚀 What's New in the Latest Version

* **Advanced Native FFI Ecosystem:** Complete functional integration of `rain_register_c_func` and `rain_get_c_func` across all code emitters, providing dynamic host runtime callback registration.
* **Architecture-Specific Call Bridges:** Introduced direct bare-metal wrappers to escape JIT threads straight into standard C hooks (`printf`, `malloc`, etc.):
  * `rain_x86_call_c(bin, "function_name")`
  * `rain_arm64_call_c(bin, "function_name")`
  * `rain_riscv_call_c(bin, "function_name")`
* **Textual File Parsing Pipeline (`soft-asm.c`):** An asynchronous, string-tokenizing scripting parser to swallow raw code scripts on-disk, parse scope labels, and emit complete multi-architecture binary images (`.bin`).
* **Multi-Syscall Integration:** Added `SOFT_MULTISYSCALL` operation maps inside the virtual machine core to aggregate transactional host operating system traps cleanly.

---

## Pipeline & Architecture Flow

The engine completely bypasses heavy internal abstraction graphs (like LLVM or GCC IR), performing single-pass translation from virtual variables down to raw machine bits.


```

[ Raw Script File / SOFT Instruction Stream ]
│
▼
[ SOFT Tokenizing & Parsing Layer ]
│
▼
[ SOFT Virtual Register & Domain Memory Tracker ]
│
▼
[ RAIN Multi-Arch Macro Machine ]
│
┌─────────────────┼─────────────────┐
▼                 ▼                 ▼
[ x86_64 ]       [ ARM64 ]        [ RISC-V ]
(Intel/AMD)    (Apple Silicon)  (Custom 2.4GHz SoC)
(REX/ModRM)    (32-bit Scaled)   (Standard RV64GC)

```

---

## 🏗️ SOFT Interface: Virtual Architecture Specification

### 1. The Virtual Register State (Bit-Width Partitioned)
SOFT abstracts physical registers into hardware-independent structures grouped by bit widths:
* **8-bit Lanes:** `sA8`, `sB8`, `sC8`, `sD8`
* **16-bit Lanes:** `sA16`, `sB16`, `sC16`, `sD16`
* **32-bit Lanes:** `sA32`, `sB32`, `sC32`, `sD32`
* **64-bit Lanes:** `sA64`, `sB64`, `sC64`, `sD64`

### 2. Domain-Driven Stack Scoping (`SOFT_CMD`)
Memory allocation is tracked dynamically via explicit linear memory frames known as **Domains**. Variables belong inside explicit offsets resolved automatically relative to the Stack Pointer (`sp`).

#### Available Commands:
* `SOFT_NEW_DOMAIN`: Allocates a localized layout tracking grid in memory with explicit size limits.
* `SOFT_DEL_DOMAIN`: Destroys the domain tracking footprint and adjusts the execution stack pointer depth.
* `SOFT_NEW_VARIABLE`: Appends an explicit variables block bound inside a defined parent Domain.
* `SOFT_LOAD_VARIABLE` / `SOFT_SET_VARIABLE`: Memory-to-register and register-to-memory state pipelines.
* `SOFT_GET_VARIABLE` / `SOFT_GET_VARIABLE_PTR`: Extracts data payloads or direct hardware physical frame addresses.
* `SOFT_ADD` / `SOFT_SUB` / `SOFT_MUL` / `SOFT_DIV`: Hardware math wrappers with transparent immediate fallback management.
* `SOFT_IF` / `SOFT_WHILE` / `SOFT_ELSE` / `SOFT_ELIF`: High-level structural jump mapping.
* `SOFT_PUSH` / `SOFT_POP`: Directly interact with the architectural system pipeline stack layer.

---

## 🌧️ RAIN Interface: Comprehensive Instruction Reference

### A. x86_64 Target Compiler (`rain_x86_64_compiler`)
* **Opcodes:** `mov`, `add`, `sub`, `mul`, `div`, `push`, `pop`, `ret`, `xor`, `cmp`, `jmp`, `jz`, `je`, `jl`, `jg`, `call`.
* **Hardware Mappings:** Directly translates parameters into physical Intel/AMD hardware structures: `rax`, `rcx`, `rdx`, `rbx`, `rsp`, `rbp`, `rsi`, `rdi`, `r8`-`r15` (and lower structural registers like `eax`, `ax`, `al`).

### B. ARM64 Target Compiler (`rain_arm64_compiler`)
* **Opcodes:** `mov`, `add`, `sub`, `mul` (utilizes native 32-bit `MADD` masking pipelines), `div` (`SDIV`/`UDIV` selection), `ldr`, `str`, `ret`, `blr` (Branch with Link to Register).
* **Hardware Mappings:** `x0` through `x30` core general-purpose lanes, `sp` (Stack Pointer), and `xzr` (Zero Register).

### C. RISC-V 64 Target Compiler (`rain_riscv_compiler`)
* **Opcodes:** `add`, `addi`, `sub`, `mul`, `div`, `lw`, `ld`, `sw`, `sd`, `jal`, `jalr`, `ret`, `ecall`, `nop`, `li` (Load Immediate pseudo-instruction handler block).
* **Hardware Mappings:** Fully compliant with RISC-V ABI conventions: `x0` (`zero`), `x1` (`ra`), `x2` (`sp`), parameters `x10`-`x17` (`a0`-`a7`), spanning cleanly up to physical `x31`.

---

## 💻 Full System Integration Examples

### Example 1: Programming via C Structures (`soft-asm.c`)
You can programmatically compose structural compilation blocks to multi-target your build array natively:

```c
#include "soft.h"
#include <stdio.h>

int main() {
    int64_t size_32 = 32;
    int64_t size_8  = 8;
    int64_t initial_val = 100;

    // Build a programmatic array of operations
    SOFT_ORDER orders[] = {
        // 1. Initialize a domain stack space called "global_scope"
        { SOFT_NEW_DOMAIN, (void*[]){"global_scope", &size_32}, (SOFT_TYPE[]){SOFT_TYPE_NAME, SOFT_TYPE_INT}, 2 },
        // 2. Map a variable "health" inside it
        { SOFT_NEW_VARIABLE, (void*[]){"health", "global_scope", &size_8}, (SOFT_TYPE[]){SOFT_TYPE_NAME, SOFT_TYPE_NAME, SOFT_TYPE_INT}, 3 },
        // 3. Mathematical adjustments using virtual registers
        { SOFT_ADD, (void*[]){"sA64", &initial_val}, (SOFT_TYPE[]){SOFT_TYPE_REG64, SOFT_TYPE_INT}, 2 },
        // 4. Teardown safely
        { SOFT_DEL_DOMAIN, (void*[]){"global_scope"}, (SOFT_TYPE[]){SOFT_TYPE_NAME}, 1 }
    };

    // Emit fully formed machine code binaries for x86_64, ARM64, and RISC-V instantly
    rain_bin_t x86_bin   = softCompileX86_64(orders, 4);
    rain_bin_t riscv_bin = softCompileRISC_V(orders, 4);
    rain_bin_t arm64_bin = softCompileARM64(orders, 4);

    printf("Generated x86_64 Payload size: %zu bytes\\n", x86_bin.size);
    printf("Generated RISC-V Payload size: %zu bytes\\n", riscv_bin.size);

    rain_bin_free(&x86_bin);
    rain_bin_free(&riscv_bin);
    rain_bin_free(&arm64_bin);
    return 0;
}

```

### Example 2: Using the Text-Based Script Assembly Engine

The framework provides an independent tokenizer that reads plain-text structural scripting lines from your file-system.

Create a source file named `script.soft`:

```text
SOFT_NEW_DOMAIN "main_runtime" 64
SOFT_ADD sA64 50
SOFT_SUB sA64 sB64
SOFT_DEL_DOMAIN "main_runtime"

```

Compile it to raw target bytes directly from your terminal using the built-in parser interface:

```c
// Compilation interface code inside your host engine setup
int main() {
    // Reads text file, parses tokens, resolves variable/stack scopes, and dumps a valid file down to disk
    // Targets: "x86_64", "arm64", "riscv"
    soft_compile_file_to_binary("script.soft", "output_payload.bin", "riscv");
    return 0;
}

```

---

## ⚙️ JIT Execution Configuration Requirements

When executing the compiled outputs in memory, you must mark your allocation pools executable on your host operating system platform:

* **Linux / POSIX Environments:** Use `mmap` with flags `PROT_READ | PROT_WRITE | PROT_EXEC`. If using split-mapping structures, write code to your write-buffer, then execute `mprotect` to switch modes safely.
* **Instruction Cache Flushing:** On **ARM64** and **RISC-V** platforms, always invoke standard hardware cache sync primitives (`__builtin___clear_cache`) across your execution buffer borders before passing the program counter (`PC`) straight into the generated arrays.

---

## 📄 License

This compilation framework is distributed as open-source infrastructure under the **GNU General Public License v3.0 (GPLv3)**. Please consult explicit licensing texts for deployment compliance rights.

```

Bu dosya, projenizi hem **basit bir JIT makro motoru** hem de **bütünsel bir dosya/metin tabanlı derleyici mimarisi** olarak ele alır. GitHub profilinizde veya projenizin vitrininde üstün bir mühendislik dökümantasyonu olarak sergilenmeye hazırdır. 😉

```
