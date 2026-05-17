#ifndef RAIN_H
#define RAIN_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Binary buffer structure
typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} rain_bin_t;

static inline void rain_bin_init(rain_bin_t *bin) {
    bin->data = NULL;
    bin->size = 0;
    bin->capacity = 0;
}

static inline void rain_bin_free(rain_bin_t *bin) {
    if (bin->data) {
        free(bin->data);
    }
    bin->data = NULL;
    bin->size = 0;
    bin->capacity = 0;
}

static inline void rain_bin_append(rain_bin_t *bin, const uint8_t *bytes, size_t len) {
    if (bin->size + len > bin->capacity) {
        bin->capacity = bin->capacity == 0 ? 256 : bin->capacity * 2;
        while (bin->size + len > bin->capacity) {
            bin->capacity *= 2;
        }
        bin->data = (uint8_t *)realloc(bin->data, bin->capacity);
    }
    memcpy(bin->data + bin->size, bytes, len);
    bin->size += len;
}

// Operand system
typedef enum {
    RAIN_OP_NONE,
    RAIN_OP_REG,
    RAIN_OP_IMM,
    RAIN_OP_MEM
} rain_op_type_t;

typedef struct {
    rain_op_type_t type;
    int reg;         // Register index
    int64_t imm;     // Immediate value
    int size;        // Operand size (bytes)
    struct {
        int base;
        int index;
        int scale;
        int32_t disp;
    } mem;           // Memory operand
} rain_operand_t;

static inline rain_operand_t rain_op_none(void) {
    rain_operand_t op;
    op.type = RAIN_OP_NONE;
    op.reg = 0;
    op.imm = 0;
    op.size = 0;
    memset(&op.mem, 0, sizeof(op.mem));
    return op;
}

// Helper to determine the macro overload count
#define RAIN_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME

// -------------------------------------------------------------
// X86_64 ARCHITECTURE DEFINITIONS & ENCODING
// -------------------------------------------------------------

typedef enum {
    rain_x86_64_mov,
    rain_x86_64_add,
    rain_x86_64_sub,
    rain_x86_64_cmp,
    rain_x86_64_and,
    rain_x86_64_or,
    rain_x86_64_xor,
    rain_x86_64_push,
    rain_x86_64_pop,
    rain_x86_64_jmp,
    rain_x86_64_je,
    rain_x86_64_jne,
    rain_x86_64_jg,
    rain_x86_64_jl,
    rain_x86_64_jge,
    rain_x86_64_jle,
    rain_x86_64_call,
    rain_x86_64_ret,
    rain_x86_64_nop,
    rain_x86_64_syscall,
} rain_x86_64_ins_t;

// Registers (size = 8 for 64-bit RAX, RBX, etc.)
#define rain_x86_64_regA8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 0, .size = 8 }) // RAX
#define rain_x86_64_regC8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 1, .size = 8 }) // RCX
#define rain_x86_64_regD8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 2, .size = 8 }) // RDX
#define rain_x86_64_regB8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 3, .size = 8 }) // RBX
#define rain_x86_64_regSP  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 4, .size = 8 }) // RSP
#define rain_x86_64_regBP  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 5, .size = 8 }) // RBP
#define rain_x86_64_regSI  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 6, .size = 8 }) // RSI
#define rain_x86_64_regDI  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 7, .size = 8 }) // RDI
#define rain_x86_64_regR8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 8, .size = 8 })
#define rain_x86_64_regR9  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 9, .size = 8 })
#define rain_x86_64_regR10 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 10, .size = 8 })
#define rain_x86_64_regR11 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 11, .size = 8 })
#define rain_x86_64_regR12 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 12, .size = 8 })
#define rain_x86_64_regR13 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 13, .size = 8 })
#define rain_x86_64_regR14 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 14, .size = 8 })
#define rain_x86_64_regR15 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 15, .size = 8 })

// Convenient aliases
#define rain_x86_64_rax rain_x86_64_regA8
#define rain_x86_64_rcx rain_x86_64_regC8
#define rain_x86_64_rdx rain_x86_64_regD8
#define rain_x86_64_rbx rain_x86_64_regB8
#define rain_x86_64_rsp rain_x86_64_regSP
#define rain_x86_64_rbp rain_x86_64_regBP
#define rain_x86_64_rsi rain_x86_64_regSI
#define rain_x86_64_rdi rain_x86_64_regDI
#define rain_x86_64_r8  rain_x86_64_regR8
#define rain_x86_64_r9  rain_x86_64_regR9
#define rain_x86_64_r10 rain_x86_64_regR10
#define rain_x86_64_r11 rain_x86_64_regR11
#define rain_x86_64_r12 rain_x86_64_regR12
#define rain_x86_64_r13 rain_x86_64_regR13
#define rain_x86_64_r14 rain_x86_64_regR14
#define rain_x86_64_r15 rain_x86_64_regR15

static inline rain_operand_t rain_x86_64_imd(int64_t val) {
    rain_operand_t op;
    op.type = RAIN_OP_IMM;
    op.reg = 0;
    op.imm = val;
    op.size = 8;
    memset(&op.mem, 0, sizeof(op.mem));
    return op;
}

static inline rain_operand_t rain_x86_64_mem(rain_operand_t base, int32_t displacement) {
    rain_operand_t op;
    op.type = RAIN_OP_MEM;
    op.reg = 0;
    op.imm = 0;
    op.size = 8;
    op.mem.base = base.reg;
    op.mem.index = -1;
    op.mem.scale = 1;
    op.mem.disp = displacement;
    return op;
}

// X86_64 memory ModRM and SIB byte builder
static inline void rain_x86_64_encode_mem(uint8_t *rex, uint8_t *modrm, uint8_t *sib, int *sib_present, uint8_t *disp_bytes, int *disp_len, int reg_or_op, int base, int32_t disp) {
    *sib_present = 0;
    *disp_len = 0;
    
    int mod = 0;
    if (disp == 0 && base != 5 && base != 13 && base != 4 && base != 12) {
        mod = 0;
    } else if (disp >= -128 && disp <= 127) {
        mod = 1;
        *disp_len = 1;
        disp_bytes[0] = (uint8_t)disp;
    } else {
        mod = 2;
        *disp_len = 4;
        memcpy(disp_bytes, &disp, 4);
    }
    
    int rm = base & 7;
    int reg_field = reg_or_op & 7;
    
    if ((base & 7) == 4) { // RSP / R12 requires SIB
        *sib_present = 1;
        *sib = 0x24; // base RSP, no index
        rm = 4;
    }
    
    *modrm = (mod << 6) | (reg_field << 3) | rm;
    
    if (base >= 8) {
        *rex |= 1; // REX.B
    }
}

// X86_64 arithmetic operations helper
static inline void rain_x86_64_encode_alu(rain_bin_t *bin, uint8_t op_reg_reg, uint8_t reg_field_imm, rain_operand_t op1, rain_operand_t op2) {
    if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
        uint8_t rex = 0x48 | (op2.reg >= 8 ? 4 : 0) | (op1.reg >= 8 ? 1 : 0);
        uint8_t opcode = op_reg_reg;
        uint8_t modrm = 0xC0 | ((op2.reg & 7) << 3) | (op1.reg & 7);
        uint8_t bytes[3] = { rex, opcode, modrm };
        rain_bin_append(bin, bytes, 3);
    } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
        uint8_t rex = 0x48 | (op1.reg >= 8 ? 1 : 0);
        if (op2.imm >= -128 && op2.imm <= 127) {
            uint8_t opcode = 0x83;
            uint8_t modrm = 0xC0 | (reg_field_imm << 3) | (op1.reg & 7);
            uint8_t imm8 = (uint8_t)op2.imm;
            uint8_t bytes[4] = { rex, opcode, modrm, imm8 };
            rain_bin_append(bin, bytes, 4);
        } else {
            uint8_t opcode = 0x81;
            uint8_t modrm = 0xC0 | (reg_field_imm << 3) | (op1.reg & 7);
            int32_t imm32 = (int32_t)op2.imm;
            uint8_t bytes[7] = { rex, opcode, modrm };
            memcpy(bytes + 3, &imm32, 4);
            rain_bin_append(bin, bytes, 7);
        }
    } else if (op1.type == RAIN_OP_MEM && op2.type == RAIN_OP_REG) {
        uint8_t rex = 0x48 | (op2.reg >= 8 ? 4 : 0);
        uint8_t opcode = op_reg_reg;
        uint8_t modrm, sib;
        int sib_present, disp_len;
        uint8_t disp_bytes[4];
        rain_x86_64_encode_mem(&rex, &modrm, &sib, &sib_present, disp_bytes, &disp_len, op2.reg, op1.mem.base, op1.mem.disp);
        
        uint8_t buf[16];
        int ptr = 0;
        buf[ptr++] = rex;
        buf[ptr++] = opcode;
        buf[ptr++] = modrm;
        if (sib_present) buf[ptr++] = sib;
        for (int i = 0; i < disp_len; i++) buf[ptr++] = disp_bytes[i];
        rain_bin_append(bin, buf, ptr);
    } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_MEM) {
        uint8_t rex = 0x48 | (op1.reg >= 8 ? 4 : 0);
        uint8_t opcode = op_reg_reg + 2; // load/store opcode distinction
        uint8_t modrm, sib;
        int sib_present, disp_len;
        uint8_t disp_bytes[4];
        rain_x86_64_encode_mem(&rex, &modrm, &sib, &sib_present, disp_bytes, &disp_len, op1.reg, op2.mem.base, op2.mem.disp);
        
        uint8_t buf[16];
        int ptr = 0;
        buf[ptr++] = rex;
        buf[ptr++] = opcode;
        buf[ptr++] = modrm;
        if (sib_present) buf[ptr++] = sib;
        for (int i = 0; i < disp_len; i++) buf[ptr++] = disp_bytes[i];
        rain_bin_append(bin, buf, ptr);
    }
}

static inline void rain_x86_64_encode_cond_jump(rain_bin_t *bin, uint8_t op8, uint8_t op32_low, rain_operand_t op1) {
    if (op1.imm >= -128 && op1.imm <= 127) {
        uint8_t bytes[2] = { op8, (uint8_t)op1.imm };
        rain_bin_append(bin, bytes, 2);
    } else {
        uint8_t bytes[6] = { 0x0F, op32_low };
        int32_t imm32 = (int32_t)op1.imm;
        memcpy(bytes + 2, &imm32, 4);
        rain_bin_append(bin, bytes, 6);
    }
}

static inline void rain_x86_64_compiler_impl(rain_bin_t *bin, rain_x86_64_ins_t ins, int op_count, rain_operand_t op1, rain_operand_t op2) {
    (void)op_count;
    switch (ins) {
        case rain_x86_64_mov:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                if (op2.imm >= -2147483648LL && op2.imm <= 2147483647LL) {
                    uint8_t rex = 0x48 | (op1.reg >= 8 ? 1 : 0);
                    uint8_t opcode = 0xC7;
                    uint8_t modrm = 0xC0 | (op1.reg & 7);
                    int32_t imm32 = (int32_t)op2.imm;
                    uint8_t bytes[7] = { rex, opcode, modrm };
                    memcpy(bytes + 3, &imm32, 4);
                    rain_bin_append(bin, bytes, 7);
                } else {
                    uint8_t rex = 0x48 | (op1.reg >= 8 ? 1 : 0);
                    uint8_t opcode = 0xB8 + (op1.reg & 7);
                    uint8_t bytes[10] = { rex, opcode };
                    memcpy(bytes + 2, &op2.imm, 8);
                    rain_bin_append(bin, bytes, 10);
                }
            } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                uint8_t rex = 0x48 | (op2.reg >= 8 ? 4 : 0) | (op1.reg >= 8 ? 1 : 0);
                uint8_t opcode = 0x89;
                uint8_t modrm = 0xC0 | ((op2.reg & 7) << 3) | (op1.reg & 7);
                uint8_t bytes[3] = { rex, opcode, modrm };
                rain_bin_append(bin, bytes, 3);
            } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_MEM) {
                uint8_t rex = 0x48 | (op1.reg >= 8 ? 4 : 0);
                uint8_t opcode = 0x8B;
                uint8_t modrm, sib;
                int sib_present, disp_len;
                uint8_t disp_bytes[4];
                rain_x86_64_encode_mem(&rex, &modrm, &sib, &sib_present, disp_bytes, &disp_len, op1.reg, op2.mem.base, op2.mem.disp);
                
                uint8_t buf[16];
                int ptr = 0;
                buf[ptr++] = rex;
                buf[ptr++] = opcode;
                buf[ptr++] = modrm;
                if (sib_present) buf[ptr++] = sib;
                for (int i = 0; i < disp_len; i++) buf[ptr++] = disp_bytes[i];
                rain_bin_append(bin, buf, ptr);
            } else if (op1.type == RAIN_OP_MEM && op2.type == RAIN_OP_REG) {
                uint8_t rex = 0x48 | (op2.reg >= 8 ? 4 : 0);
                uint8_t opcode = 0x89;
                uint8_t modrm, sib;
                int sib_present, disp_len;
                uint8_t disp_bytes[4];
                rain_x86_64_encode_mem(&rex, &modrm, &sib, &sib_present, disp_bytes, &disp_len, op2.reg, op1.mem.base, op1.mem.disp);
                
                uint8_t buf[16];
                int ptr = 0;
                buf[ptr++] = rex;
                buf[ptr++] = opcode;
                buf[ptr++] = modrm;
                if (sib_present) buf[ptr++] = sib;
                for (int i = 0; i < disp_len; i++) buf[ptr++] = disp_bytes[i];
                rain_bin_append(bin, buf, ptr);
            } else if (op1.type == RAIN_OP_MEM && op2.type == RAIN_OP_IMM) {
                uint8_t rex = 0x48;
                uint8_t opcode = 0xC7;
                uint8_t modrm, sib;
                int sib_present, disp_len;
                uint8_t disp_bytes[4];
                rain_x86_64_encode_mem(&rex, &modrm, &sib, &sib_present, disp_bytes, &disp_len, 0, op1.mem.base, op1.mem.disp);
                
                uint8_t buf[20];
                int ptr = 0;
                buf[ptr++] = rex;
                buf[ptr++] = opcode;
                buf[ptr++] = modrm;
                if (sib_present) buf[ptr++] = sib;
                for (int i = 0; i < disp_len; i++) buf[ptr++] = disp_bytes[i];
                int32_t imm32 = (int32_t)op2.imm;
                memcpy(buf + ptr, &imm32, 4);
                ptr += 4;
                rain_bin_append(bin, buf, ptr);
            }
            break;
            
        case rain_x86_64_add:  rain_x86_64_encode_alu(bin, 0x01, 0, op1, op2); break;
        case rain_x86_64_sub:  rain_x86_64_encode_alu(bin, 0x29, 5, op1, op2); break;
        case rain_x86_64_cmp:  rain_x86_64_encode_alu(bin, 0x39, 7, op1, op2); break;
        case rain_x86_64_and:  rain_x86_64_encode_alu(bin, 0x21, 4, op1, op2); break;
        case rain_x86_64_or:   rain_x86_64_encode_alu(bin, 0x09, 1, op1, op2); break;
        case rain_x86_64_xor:  rain_x86_64_encode_alu(bin, 0x31, 6, op1, op2); break;
            
        case rain_x86_64_push:
            if (op1.type == RAIN_OP_REG) {
                uint8_t rex = 0x41;
                uint8_t opcode = 0x50 + (op1.reg & 7);
                if (op1.reg >= 8) {
                    uint8_t bytes[2] = { rex, opcode };
                    rain_bin_append(bin, bytes, 2);
                } else {
                    rain_bin_append(bin, &opcode, 1);
                }
            } else if (op1.type == RAIN_OP_IMM) {
                if (op1.imm >= -128 && op1.imm <= 127) {
                    uint8_t bytes[2] = { 0x6A, (uint8_t)op1.imm };
                    rain_bin_append(bin, bytes, 2);
                } else {
                    uint8_t bytes[5] = { 0x68 };
                    int32_t imm32 = (int32_t)op1.imm;
                    memcpy(bytes + 1, &imm32, 4);
                    rain_bin_append(bin, bytes, 5);
                }
            }
            break;
            
        case rain_x86_64_pop:
            if (op1.type == RAIN_OP_REG) {
                uint8_t rex = 0x41;
                uint8_t opcode = 0x58 + (op1.reg & 7);
                if (op1.reg >= 8) {
                    uint8_t bytes[2] = { rex, opcode };
                    rain_bin_append(bin, bytes, 2);
                } else {
                    rain_bin_append(bin, &opcode, 1);
                }
            }
            break;
            
        case rain_x86_64_jmp:
            if (op1.type == RAIN_OP_IMM) {
                if (op1.imm >= -128 && op1.imm <= 127) {
                    uint8_t bytes[2] = { 0xEB, (uint8_t)op1.imm };
                    rain_bin_append(bin, bytes, 2);
                } else {
                    uint8_t bytes[5] = { 0xE9 };
                    int32_t imm32 = (int32_t)op1.imm;
                    memcpy(bytes + 1, &imm32, 4);
                    rain_bin_append(bin, bytes, 5);
                }
            } else if (op1.type == RAIN_OP_REG) {
                uint8_t rex = 0x41;
                uint8_t opcode = 0xFF;
                uint8_t modrm = 0xE0 | (op1.reg & 7);
                if (op1.reg >= 8) {
                    uint8_t bytes[3] = { rex, opcode, modrm };
                    rain_bin_append(bin, bytes, 3);
                } else {
                    uint8_t bytes[2] = { opcode, modrm };
                    rain_bin_append(bin, bytes, 2);
                }
            }
            break;
            
        case rain_x86_64_je:  rain_x86_64_encode_cond_jump(bin, 0x74, 0x84, op1); break;
        case rain_x86_64_jne: rain_x86_64_encode_cond_jump(bin, 0x75, 0x85, op1); break;
        case rain_x86_64_jl:  rain_x86_64_encode_cond_jump(bin, 0x7C, 0x8C, op1); break;
        case rain_x86_64_jle: rain_x86_64_encode_cond_jump(bin, 0x7E, 0x8E, op1); break;
        case rain_x86_64_jg:  rain_x86_64_encode_cond_jump(bin, 0x7F, 0x8F, op1); break;
        case rain_x86_64_jge: rain_x86_64_encode_cond_jump(bin, 0x7D, 0x8D, op1); break;
            
        case rain_x86_64_call:
            if (op1.type == RAIN_OP_IMM) {
                uint8_t bytes[5] = { 0xE8 };
                int32_t imm32 = (int32_t)op1.imm;
                memcpy(bytes + 1, &imm32, 4);
                rain_bin_append(bin, bytes, 5);
            } else if (op1.type == RAIN_OP_REG) {
                uint8_t rex = 0x41;
                uint8_t opcode = 0xFF;
                uint8_t modrm = 0xD0 | (op1.reg & 7);
                if (op1.reg >= 8) {
                    uint8_t bytes[3] = { rex, opcode, modrm };
                    rain_bin_append(bin, bytes, 3);
                } else {
                    uint8_t bytes[2] = { opcode, modrm };
                    rain_bin_append(bin, bytes, 2);
                }
            }
            break;
            
        case rain_x86_64_ret: {
            uint8_t ret_byte = 0xC3;
            rain_bin_append(bin, &ret_byte, 1);
            break;
        }
            
        case rain_x86_64_nop: {
            uint8_t nop_byte = 0x90;
            rain_bin_append(bin, &nop_byte, 1);
            break;
        }
            
        case rain_x86_64_syscall: {
            uint8_t bytes[2] = { 0x0F, 0x05 };
            rain_bin_append(bin, bytes, 2);
            break;
        }
    }
}

#define rain_x86_64_compiler_2(bin, ins) rain_x86_64_compiler_impl(bin, ins, 0, rain_op_none(), rain_op_none())
#define rain_x86_64_compiler_3(bin, ins, op1) rain_x86_64_compiler_impl(bin, ins, 1, op1, rain_op_none())
#define rain_x86_64_compiler_4(bin, ins, op1, op2) rain_x86_64_compiler_impl(bin, ins, 2, op1, op2)

#define rain_x86_64_compiler(...) RAIN_GET_MACRO(__VA_ARGS__, rain_x86_64_compiler_4, rain_x86_64_compiler_3, rain_x86_64_compiler_2)(__VA_ARGS__)

// -------------------------------------------------------------
// ARM64 (AARCH64) ARCHITECTURE DEFINITIONS & ENCODING
// -------------------------------------------------------------

typedef enum {
    rain_arm64_mov,
    rain_arm64_add,
    rain_arm64_sub,
    rain_arm64_cmp,
    rain_arm64_and,
    rain_arm64_orr,
    rain_arm64_eor,
    rain_arm64_b,
    rain_arm64_br,
    rain_arm64_bl,
    rain_arm64_blr,
    rain_arm64_ret,
    rain_arm64_ldr,
    rain_arm64_str,
    rain_arm64_nop,
} rain_arm64_ins_t;

#define rain_arm64_x0  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 0, .size = 8 })
#define rain_arm64_x1  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 1, .size = 8 })
#define rain_arm64_x2  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 2, .size = 8 })
#define rain_arm64_x3  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 3, .size = 8 })
#define rain_arm64_x4  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 4, .size = 8 })
#define rain_arm64_x5  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 5, .size = 8 })
#define rain_arm64_x6  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 6, .size = 8 })
#define rain_arm64_x7  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 7, .size = 8 })
#define rain_arm64_x8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 8, .size = 8 })
#define rain_arm64_x9  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 9, .size = 8 })
#define rain_arm64_x10 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 10, .size = 8 })
#define rain_arm64_x11 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 11, .size = 8 })
#define rain_arm64_x12 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 12, .size = 8 })
#define rain_arm64_x13 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 13, .size = 8 })
#define rain_arm64_x14 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 14, .size = 8 })
#define rain_arm64_x15 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 15, .size = 8 })
#define rain_arm64_x16 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 16, .size = 8 })
#define rain_arm64_x17 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 17, .size = 8 })
#define rain_arm64_x18 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 18, .size = 8 })
#define rain_arm64_x19 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 19, .size = 8 })
#define rain_arm64_x20 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 20, .size = 8 })
#define rain_arm64_x21 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 21, .size = 8 })
#define rain_arm64_x22 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 22, .size = 8 })
#define rain_arm64_x23 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 23, .size = 8 })
#define rain_arm64_x24 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 24, .size = 8 })
#define rain_arm64_x25 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 25, .size = 8 })
#define rain_arm64_x26 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 26, .size = 8 })
#define rain_arm64_x27 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 27, .size = 8 })
#define rain_arm64_x28 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 28, .size = 8 })
#define rain_arm64_x29 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 29, .size = 8 })
#define rain_arm64_x30 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 30, .size = 8 })
#define rain_arm64_sp  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 31, .size = 8 })
#define rain_arm64_xzr ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 31, .size = 8 })

static inline rain_operand_t rain_arm64_imd(int64_t val) {
    rain_operand_t op;
    op.type = RAIN_OP_IMM;
    op.reg = 0;
    op.imm = val;
    op.size = 8;
    memset(&op.mem, 0, sizeof(op.mem));
    return op;
}

static inline rain_operand_t rain_arm64_mem(rain_operand_t base, int32_t displacement) {
    rain_operand_t op;
    op.type = RAIN_OP_MEM;
    op.reg = 0;
    op.imm = 0;
    op.size = 8;
    op.mem.base = base.reg;
    op.mem.index = -1;
    op.mem.scale = 1;
    op.mem.disp = displacement;
    return op;
}

static inline void rain_arm64_emit_mov_imm(rain_bin_t *bin, int rd, int64_t val) {
    uint16_t chunk0 = (uint16_t)(val & 0xFFFF);
    uint16_t chunk1 = (uint16_t)((val >> 16) & 0xFFFF);
    uint16_t chunk2 = (uint16_t)((val >> 32) & 0xFFFF);
    uint16_t chunk3 = (uint16_t)((val >> 48) & 0xFFFF);
    
    uint32_t instr = 0xD2800000 | ((uint32_t)chunk0 << 5) | rd;
    rain_bin_append(bin, (uint8_t *)&instr, 4);
    
    if (chunk1 != 0 || val < 0) {
        uint32_t k_instr = 0xF2800000 | (1 << 21) | ((uint32_t)chunk1 << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&k_instr, 4);
    }
    if (chunk2 != 0 || val < 0) {
        uint32_t k_instr = 0xF2800000 | (2 << 21) | ((uint32_t)chunk2 << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&k_instr, 4);
    }
    if (chunk3 != 0 || val < 0) {
        uint32_t k_instr = 0xF2800000 | (3 << 21) | ((uint32_t)chunk3 << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&k_instr, 4);
    }
}

static inline void rain_arm64_emit_add_imm(rain_bin_t *bin, int rd, int rn, int64_t imm) {
    if (imm >= 0 && imm < 4096) {
        uint32_t instr = 0x91000000 | ((uint32_t)imm << 10) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else if (imm < 0 && -imm < 4096) {
        uint32_t instr = 0xD1000000 | ((uint32_t)(-imm) << 10) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else {
        rain_arm64_emit_mov_imm(bin, 16, imm);
        uint32_t instr = 0x8B000000 | (16 << 16) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    }
}

static inline void rain_arm64_emit_ldr(rain_bin_t *bin, int rd, int rn, int32_t disp) {
    if (disp >= 0 && disp < 32768 && (disp % 8) == 0) {
        uint32_t instr = 0xF9400000 | (((uint32_t)disp / 8) << 10) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else {
        rain_arm64_emit_mov_imm(bin, 16, disp);
        uint32_t instr = 0xF9400000 | (16 << 16) | (3 << 13) | (1 << 12) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    }
}

static inline void rain_arm64_emit_str(rain_bin_t *bin, int rd, int rn, int32_t disp) {
    if (disp >= 0 && disp < 32768 && (disp % 8) == 0) {
        uint32_t instr = 0xF9000000 | (((uint32_t)disp / 8) << 10) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else {
        rain_arm64_emit_mov_imm(bin, 16, disp);
        uint32_t instr = 0xF9000000 | (16 << 16) | (3 << 13) | (1 << 12) | (rn << 5) | rd;
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    }
}

static inline void rain_arm64_compiler_impl(rain_bin_t *bin, rain_arm64_ins_t ins, int op_count, rain_operand_t op1, rain_operand_t op2) {
    (void)op_count;
    uint32_t instr = 0;
    switch (ins) {
        case rain_arm64_mov:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                rain_arm64_emit_mov_imm(bin, op1.reg, op2.imm);
            } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0xAA0003E0 | (op2.reg << 16) | op1.reg;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_add:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x8B000000 | (op2.reg << 16) | (op1.reg << 5) | op1.reg;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                rain_arm64_emit_add_imm(bin, op1.reg, op1.reg, op2.imm);
            }
            break;
            
        case rain_arm64_sub:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0xCB000000 | (op2.reg << 16) | (op1.reg << 5) | op1.reg;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                rain_arm64_emit_add_imm(bin, op1.reg, op1.reg, -op2.imm);
            }
            break;
            
        case rain_arm64_cmp:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0xEB000000 | (op2.reg << 16) | (op1.reg << 5) | 31;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            } else if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                if (op2.imm >= 0 && op2.imm < 4096) {
                    instr = 0xF1000000 | ((uint32_t)op2.imm << 10) | (op1.reg << 5) | 31;
                    rain_bin_append(bin, (uint8_t *)&instr, 4);
                } else {
                    rain_arm64_emit_mov_imm(bin, 16, op2.imm);
                    instr = 0xEB000000 | (16 << 16) | (op1.reg << 5) | 31;
                    rain_bin_append(bin, (uint8_t *)&instr, 4);
                }
            }
            break;
            
        case rain_arm64_and:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x8A000000 | (op2.reg << 16) | (op1.reg << 5) | op1.reg;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_orr:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0xAA000000 | (op2.reg << 16) | (op1.reg << 5) | op1.reg;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_eor:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0xCA000000 | (op2.reg << 16) | (op1.reg << 5) | op1.reg;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_b:
            if (op1.type == RAIN_OP_IMM) {
                instr = 0x14000000 | (((uint32_t)op1.imm >> 2) & 0x03FFFFFF);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_br:
            if (op1.type == RAIN_OP_REG) {
                instr = 0xD61F0000 | (op1.reg << 5);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_bl:
            if (op1.type == RAIN_OP_IMM) {
                instr = 0x94000000 | (((uint32_t)op1.imm >> 2) & 0x03FFFFFF);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_blr:
            if (op1.type == RAIN_OP_REG) {
                instr = 0xD63F0000 | (op1.reg << 5);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_arm64_ret:
            instr = 0xD65F03C0;
            rain_bin_append(bin, (uint8_t *)&instr, 4);
            break;
            
        case rain_arm64_ldr:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_MEM) {
                rain_arm64_emit_ldr(bin, op1.reg, op2.mem.base, op2.mem.disp);
            }
            break;
            
        case rain_arm64_str:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_MEM) {
                rain_arm64_emit_str(bin, op1.reg, op2.mem.base, op2.mem.disp);
            }
            break;
            
        case rain_arm64_nop:
            instr = 0xD503201F;
            rain_bin_append(bin, (uint8_t *)&instr, 4);
            break;
    }
}

#define rain_arm64_compiler_2(bin, ins) rain_arm64_compiler_impl(bin, ins, 0, rain_op_none(), rain_op_none())
#define rain_arm64_compiler_3(bin, ins, op1) rain_arm64_compiler_impl(bin, ins, 1, op1, rain_op_none())
#define rain_arm64_compiler_4(bin, ins, op1, op2) rain_arm64_compiler_impl(bin, ins, 2, op1, op2)

#define rain_arm64_compiler(...) RAIN_GET_MACRO(__VA_ARGS__, rain_arm64_compiler_4, rain_arm64_compiler_3, rain_arm64_compiler_2)(__VA_ARGS__)

// -------------------------------------------------------------
// RISC-V (RV64) ARCHITECTURE DEFINITIONS & ENCODING
// -------------------------------------------------------------

typedef enum {
    rain_riscv_add,
    rain_riscv_sub,
    rain_riscv_addi,
    rain_riscv_and,
    rain_riscv_andi,
    rain_riscv_or,
    rain_riscv_ori,
    rain_riscv_xor,
    rain_riscv_xori,
    rain_riscv_li,
    rain_riscv_mv,
    rain_riscv_ld,
    rain_riscv_sd,
    rain_riscv_j,
    rain_riscv_jr,
    rain_riscv_jal,
    rain_riscv_jalr,
    rain_riscv_ret,
    rain_riscv_ecall,
    rain_riscv_nop,
} rain_riscv_ins_t;

#define rain_riscv_x0  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 0, .size = 8 })
#define rain_riscv_x1  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 1, .size = 8 })
#define rain_riscv_x2  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 2, .size = 8 })
#define rain_riscv_x3  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 3, .size = 8 })
#define rain_riscv_x4  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 4, .size = 8 })
#define rain_riscv_x5  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 5, .size = 8 })
#define rain_riscv_x6  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 6, .size = 8 })
#define rain_riscv_x7  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 7, .size = 8 })
#define rain_riscv_x8  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 8, .size = 8 })
#define rain_riscv_x9  ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 9, .size = 8 })
#define rain_riscv_x10 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 10, .size = 8 })
#define rain_riscv_x11 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 11, .size = 8 })
#define rain_riscv_x12 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 12, .size = 8 })
#define rain_riscv_x13 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 13, .size = 8 })
#define rain_riscv_x14 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 14, .size = 8 })
#define rain_riscv_x15 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 15, .size = 8 })
#define rain_riscv_x16 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 16, .size = 8 })
#define rain_riscv_x17 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 17, .size = 8 })
#define rain_riscv_x18 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 18, .size = 8 })
#define rain_riscv_x19 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 19, .size = 8 })
#define rain_riscv_x20 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 20, .size = 8 })
#define rain_riscv_x21 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 21, .size = 8 })
#define rain_riscv_x22 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 22, .size = 8 })
#define rain_riscv_x23 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 23, .size = 8 })
#define rain_riscv_x24 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 24, .size = 8 })
#define rain_riscv_x25 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 25, .size = 8 })
#define rain_riscv_x26 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 26, .size = 8 })
#define rain_riscv_x27 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 27, .size = 8 })
#define rain_riscv_x28 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 28, .size = 8 })
#define rain_riscv_x29 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 29, .size = 8 })
#define rain_riscv_x30 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 30, .size = 8 })
#define rain_riscv_x31 ((rain_operand_t){ .type = RAIN_OP_REG, .reg = 31, .size = 8 })

#define rain_riscv_zero rain_riscv_x0
#define rain_riscv_ra   rain_riscv_x1
#define rain_riscv_sp   rain_riscv_x2
#define rain_riscv_gp   rain_riscv_x3
#define rain_riscv_tp   rain_riscv_x4
#define rain_riscv_t0   rain_riscv_x5
#define rain_riscv_t1   rain_riscv_x6
#define rain_riscv_t2   rain_riscv_x7
#define rain_riscv_s0   rain_riscv_x8
#define rain_riscv_fp   rain_riscv_x8
#define rain_riscv_s1   rain_riscv_x9
#define rain_riscv_a0   rain_riscv_x10
#define rain_riscv_a1   rain_riscv_x11
#define rain_riscv_a2   rain_riscv_x12
#define rain_riscv_a3   rain_riscv_x13
#define rain_riscv_a4   rain_riscv_x14
#define rain_riscv_a5   rain_riscv_x15
#define rain_riscv_a6   rain_riscv_x16
#define rain_riscv_a7   rain_riscv_x17
#define rain_riscv_s2   rain_riscv_x18
#define rain_riscv_s3   rain_riscv_x19
#define rain_riscv_s4   rain_riscv_x20
#define rain_riscv_s5   rain_riscv_x21
#define rain_riscv_s6   rain_riscv_x22
#define rain_riscv_s7   rain_riscv_x23
#define rain_riscv_s8   rain_riscv_x24
#define rain_riscv_s9   rain_riscv_x25
#define rain_riscv_s10  rain_riscv_x26
#define rain_riscv_s11  rain_riscv_x27
#define rain_riscv_t3   rain_riscv_x28
#define rain_riscv_t4   rain_riscv_x29
#define rain_riscv_t5   rain_riscv_x30
#define rain_riscv_t6   rain_riscv_x31

static inline rain_operand_t rain_riscv_imd(int64_t val) {
    rain_operand_t op;
    op.type = RAIN_OP_IMM;
    op.reg = 0;
    op.imm = val;
    op.size = 8;
    memset(&op.mem, 0, sizeof(op.mem));
    return op;
}

static inline rain_operand_t rain_riscv_mem(rain_operand_t base, int32_t displacement) {
    rain_operand_t op;
    op.type = RAIN_OP_MEM;
    op.reg = 0;
    op.imm = 0;
    op.size = 8;
    op.mem.base = base.reg;
    op.mem.index = -1;
    op.mem.scale = 1;
    op.mem.disp = displacement;
    return op;
}

static inline void rain_riscv_emit_li(rain_bin_t *bin, int rd, int64_t val) {
    if (val >= -2048 && val <= 2047) {
        uint32_t instr = 0x13 | (rd << 7) | (0 << 12) | (0 << 15) | (((uint32_t)val & 0xFFF) << 20);
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else if (val >= -2147483648LL && val <= 2147483647LL) {
        int32_t lower = (int32_t)(val & 0xFFF);
        if (lower & 0x800) {
            lower -= 4096;
        }
        int32_t upper = (int32_t)(((val - lower) >> 12) & 0xFFFFF);
        
        uint32_t lui = 0x37 | (rd << 7) | (upper << 12);
        rain_bin_append(bin, (uint8_t *)&lui, 4);
        
        uint32_t addiw = 0x1B | (rd << 7) | (0 << 12) | (rd << 15) | (((uint32_t)lower & 0xFFF) << 20);
        rain_bin_append(bin, (uint8_t *)&addiw, 4);
    } else {
        int32_t upper32 = (int32_t)(val >> 32);
        int32_t lower32 = (int32_t)(val & 0xFFFFFFFF);
        
        rain_riscv_emit_li(bin, rd, upper32);
        
        uint32_t slli = 0x13 | (rd << 7) | (1 << 12) | (rd << 15) | (32 << 20);
        rain_bin_append(bin, (uint8_t *)&slli, 4);
        
        if (lower32 != 0) {
            rain_riscv_emit_li(bin, 31, lower32);
            uint32_t add = 0x33 | (rd << 7) | (0 << 12) | (rd << 15) | (31 << 20);
            rain_bin_append(bin, (uint8_t *)&add, 4);
        }
    }
}

static inline void rain_riscv_emit_ld(rain_bin_t *bin, int rd, int rn, int32_t disp) {
    if (disp >= -2048 && disp <= 2047) {
        uint32_t instr = 0x03 | (rd << 7) | (3 << 12) | (rn << 15) | (((uint32_t)disp & 0xFFF) << 20);
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else {
        rain_riscv_emit_li(bin, 31, disp);
        uint32_t add = 0x33 | (31 << 7) | (0 << 12) | (rn << 15) | (31 << 20);
        rain_bin_append(bin, (uint8_t *)&add, 4);
        uint32_t ld = 0x03 | (rd << 7) | (3 << 12) | (31 << 15) | (0 << 20);
        rain_bin_append(bin, (uint8_t *)&ld, 4);
    }
}

static inline void rain_riscv_emit_sd(rain_bin_t *bin, int rs, int rn, int32_t disp) {
    if (disp >= -2048 && disp <= 2047) {
        uint32_t instr = 0x23 | (((uint32_t)disp & 0x1F) << 7) | (3 << 12) | (rn << 15) | (rs << 20) | ((((uint32_t)disp >> 5) & 0x7F) << 25);
        rain_bin_append(bin, (uint8_t *)&instr, 4);
    } else {
        rain_riscv_emit_li(bin, 31, disp);
        uint32_t add = 0x33 | (31 << 7) | (0 << 12) | (rn << 15) | (31 << 20);
        rain_bin_append(bin, (uint8_t *)&add, 4);
        uint32_t sd = 0x23 | (0 << 7) | (3 << 12) | (31 << 15) | (rs << 20);
        rain_bin_append(bin, (uint8_t *)&sd, 4);
    }
}

static inline void rain_riscv_compiler_impl(rain_bin_t *bin, rain_riscv_ins_t ins, int op_count, rain_operand_t op1, rain_operand_t op2) {
    (void)op_count;
    uint32_t instr = 0;
    switch (ins) {
        case rain_riscv_add:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x33 | (op1.reg << 7) | (0 << 12) | (op1.reg << 15) | (op2.reg << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_sub:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x40000033 | (op1.reg << 7) | (op1.reg << 15) | (op2.reg << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_addi:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                instr = 0x13 | (op1.reg << 7) | (0 << 12) | (op1.reg << 15) | (((uint32_t)op2.imm & 0xFFF) << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_and:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x33 | (op1.reg << 7) | (7 << 12) | (op1.reg << 15) | (op2.reg << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_andi:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                instr = 0x13 | (op1.reg << 7) | (7 << 12) | (op1.reg << 15) | (((uint32_t)op2.imm & 0xFFF) << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_or:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x33 | (op1.reg << 7) | (6 << 12) | (op1.reg << 15) | (op2.reg << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_ori:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                instr = 0x13 | (op1.reg << 7) | (6 << 12) | (op1.reg << 15) | (((uint32_t)op2.imm & 0xFFF) << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_xor:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x33 | (op1.reg << 7) | (4 << 12) | (op1.reg << 15) | (op2.reg << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_xori:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                instr = 0x13 | (op1.reg << 7) | (4 << 12) | (op1.reg << 15) | (((uint32_t)op2.imm & 0xFFF) << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_li:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_IMM) {
                rain_riscv_emit_li(bin, op1.reg, op2.imm);
            }
            break;
            
        case rain_riscv_mv:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_REG) {
                instr = 0x13 | (op1.reg << 7) | (0 << 12) | (op2.reg << 15) | (0 << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_ld:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_MEM) {
                rain_riscv_emit_ld(bin, op1.reg, op2.mem.base, op2.mem.disp);
            }
            break;
            
        case rain_riscv_sd:
            if (op1.type == RAIN_OP_REG && op2.type == RAIN_OP_MEM) {
                rain_riscv_emit_sd(bin, op1.reg, op2.mem.base, op2.mem.disp);
            }
            break;
            
        case rain_riscv_j:
            if (op1.type == RAIN_OP_IMM) {
                uint32_t imm = (uint32_t)op1.imm;
                uint32_t bit20     = (imm >> 20) & 1;
                uint32_t bits10_1  = (imm >> 1) & 0x3FF;
                uint32_t bit11     = (imm >> 11) & 1;
                uint32_t bits19_12 = (imm >> 12) & 0xFF;
                uint32_t j_imm = (bit20 << 31) | (bits10_1 << 21) | (bit11 << 20) | (bits19_12 << 12);
                instr = 0x6F | (0 << 7) | j_imm;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_jr:
            if (op1.type == RAIN_OP_REG) {
                instr = 0x67 | (0 << 7) | (op1.reg << 15) | (0 << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_jal:
            if (op1.type == RAIN_OP_IMM) {
                uint32_t imm = (uint32_t)op1.imm;
                uint32_t bit20     = (imm >> 20) & 1;
                uint32_t bits10_1  = (imm >> 1) & 0x3FF;
                uint32_t bit11     = (imm >> 11) & 1;
                uint32_t bits19_12 = (imm >> 12) & 0xFF;
                uint32_t j_imm = (bit20 << 31) | (bits10_1 << 21) | (bit11 << 20) | (bits19_12 << 12);
                instr = 0x6F | (1 << 7) | j_imm;
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_jalr:
            if (op1.type == RAIN_OP_REG) {
                instr = 0x67 | (1 << 7) | (op1.reg << 15) | (0 << 20);
                rain_bin_append(bin, (uint8_t *)&instr, 4);
            }
            break;
            
        case rain_riscv_ret:
            instr = 0x00008067;
            rain_bin_append(bin, (uint8_t *)&instr, 4);
            break;
            
        case rain_riscv_ecall:
            instr = 0x00000073;
            rain_bin_append(bin, (uint8_t *)&instr, 4);
            break;
            
        case rain_riscv_nop:
            instr = 0x00000013;
            rain_bin_append(bin, (uint8_t *)&instr, 4);
            break;
    }
}

#define rain_riscv_compiler_2(bin, ins) rain_riscv_compiler_impl(bin, ins, 0, rain_op_none(), rain_op_none())
#define rain_riscv_compiler_3(bin, ins, op1) rain_riscv_compiler_impl(bin, ins, 1, op1, rain_op_none())
#define rain_riscv_compiler_4(bin, ins, op1, op2) rain_riscv_compiler_impl(bin, ins, 2, op1, op2)

#define rain_riscv_compiler(...) RAIN_GET_MACRO(__VA_ARGS__, rain_riscv_compiler_4, rain_riscv_compiler_3, rain_riscv_compiler_2)(__VA_ARGS__)

#endif // RAIN_H
