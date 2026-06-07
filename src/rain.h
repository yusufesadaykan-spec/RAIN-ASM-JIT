#ifndef RAIN_H
#define RAIN_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#define RAIN_MAX_LABELS 2048
#define RAIN_MAX_JUMPS  8192

typedef enum {
    PATCH_X86_REL32,
    PATCH_ARM64_REL26,
    PATCH_RISCV_REL20
} RainPatchType;

typedef struct {
    uint64_t JumpOffset;
    int LabelId;
    RainPatchType Type;
} RainJumpPatch;

typedef struct {
    uint8_t* Code;
    uint8_t* Resv;
    uint64_t Size;
    uint64_t ResvSize;
    uint64_t Capacity;
    uint64_t ResvCapacity;

    uint64_t LabelOffsets[RAIN_MAX_LABELS];
    RainJumpPatch JumpPatches[RAIN_MAX_JUMPS];
    int JumpPatchCount;
} RainBin;

typedef enum {
    REG_RAX = 0, REG_RCX = 1, REG_RDX = 2, REG_RBX = 3,
    REG_RSP = 4, REG_RBP = 5, REG_RSI = 6, REG_RDI = 7
} RainX86Reg;

typedef enum {
    REG_X0 = 0,  REG_X1 = 1,  REG_X2 = 2,  REG_X3 = 3,
    REG_X8 = 8,  REG_X10 = 10, REG_X11 = 11, REG_X29 = 29, REG_X30 = 30
} RainArmRiscVReg;

static inline void RainBinInit(RainBin* B) {
    B->Capacity = 256;
    B->Size = 0;
    B->Code = (uint8_t*)malloc(B->Capacity);

    B->ResvCapacity = 256;
    B->ResvSize = 0;
    B->Resv = (uint8_t*)malloc(B->ResvCapacity);

    B->JumpPatchCount = 0;
    memset(B->LabelOffsets, 0, sizeof(B->LabelOffsets));
}

static inline void RainDump(RainBin* B, const uint8_t* Bytes, size_t Len) {
    if (B->Size + Len > B->Capacity) {
        while (B->Size + Len > B->Capacity) B->Capacity *= 2;
        B->Code = (uint8_t*)realloc(B->Code, B->Capacity);
    }
    memcpy(&B->Code[B->Size], Bytes, Len);
    B->Size += Len;
}

static inline void RainLabel(RainBin* B, int LabelId) {
    if (LabelId >= 0 && LabelId < RAIN_MAX_LABELS) {
        B->LabelOffsets[LabelId] = B->Size;
    }
}

static inline uint64_t RainResb(RainBin* B, size_t Len) {
    if (B->ResvSize + Len > B->ResvCapacity) {
        while (B->ResvSize + Len > B->ResvCapacity) B->ResvCapacity *= 2;
        B->Resv = (uint8_t*)realloc(B->Resv, B->ResvCapacity);
    }
    uint64_t allocatedOffset = B->ResvSize;
    memset(&B->Resv[B->ResvSize], 0, Len);
    B->ResvSize += Len;
    return allocatedOffset;
}

static inline void RainBinFree(RainBin* B) {
    if (B->Code) { free(B->Code); B->Code = NULL; }
    if (B->Resv) { free(B->Resv); B->Resv = NULL; }
    B->Size = 0; B->Capacity = 0;
    B->ResvSize = 0; B->ResvCapacity = 0;
}

static inline void RainFinishBin(RainBin* B) {
    for (int i = 0; i < B->JumpPatchCount; i++) {
        RainJumpPatch patch = B->JumpPatches[i];
        uint64_t targetOffset = B->LabelOffsets[patch.LabelId];

        if (patch.Type == PATCH_X86_REL32) {
            int32_t relativeAddr = (int32_t)(targetOffset - (patch.JumpOffset + 4));
            memcpy(&B->Code[patch.JumpOffset], &relativeAddr, 4);
        }
        else if (patch.Type == PATCH_ARM64_REL26) {
            int32_t diff = ((int32_t)targetOffset - (int32_t)patch.JumpOffset) / 4;
            uint32_t op;
            memcpy(&op, &B->Code[patch.JumpOffset], 4);
            op = (op & 0xFC000000) | (diff & 0x03FFFFFF);
            memcpy(&B->Code[patch.JumpOffset], &op, 4);
        }
        else if (patch.Type == PATCH_RISCV_REL20) {
            int32_t diff = (int32_t)targetOffset - (int32_t)patch.JumpOffset;
            uint32_t op;
            memcpy(&op, &B->Code[patch.JumpOffset], 4);
            uint32_t imm20 = (diff >> 20) & 0x1;
            uint32_t imm10_1 = (diff >> 1) & 0x3FF;
            uint32_t imm11 = (diff >> 11) & 0x1;
            uint32_t imm19_12 = (diff >> 12) & 0xFF;
            uint32_t jmppatch = (imm20 << 31) | (imm10_1 << 21) | (imm11 << 20) | (imm19_12 << 12);
            op = (op & 0x0000007F) | jmppatch;
            memcpy(&B->Code[patch.JumpOffset], &op, 4);
        }
    }

    if (B->ResvSize == 0) return;
    uint64_t totalSize = B->Size + B->ResvSize;
    if (totalSize > B->Capacity) {
        B->Capacity = totalSize + 64;
        B->Code = (uint8_t*)realloc(B->Code, B->Capacity);
    }
    memcpy(&B->Code[B->Size], B->Resv, B->ResvSize);
    free(B->Resv);
    B->Resv = NULL;
    B->ResvSize = 0;
    B->ResvCapacity = 0;
}

// ============================================================================
// --- X86_64 MAÇİNE KODU EMİSYONLARI ---
// ============================================================================
static inline void RainX86Push(RainBin* B, int Reg) { uint8_t op = 0x50 + (Reg & 7); RainDump(B, &op, 1); }
static inline void RainX86Pop(RainBin* B, int Reg) { uint8_t op = 0x58 + (Reg & 7); RainDump(B, &op, 1); }
static inline void RainX86MovRegImm(RainBin* B, int Reg, uint64_t Imm) { uint8_t op[] = { 0x48, (uint8_t)(0xB8 + (Reg & 7)) }; RainDump(B, op, 2); RainDump(B, (uint8_t*)&Imm, 8); }
static inline void RainX86Xor(RainBin* B, int SrcReg, int DestReg) { uint8_t op[] = { 0x48, 0x31, (uint8_t)(0xC0 + ((SrcReg & 7) << 3) + (DestReg & 7)) }; RainDump(B, op, 3); }
static inline void RainX86And(RainBin* B, int SrcReg, int DestReg) { uint8_t op[] = { 0x48, 0x21, (uint8_t)(0xC0 + ((SrcReg & 7) << 3) + (DestReg & 7)) }; RainDump(B, op, 3); }
static inline void RainX86Or(RainBin* B, int SrcReg, int DestReg)  { uint8_t op[] = { 0x48, 0x01, (uint8_t)(0xC0 + ((SrcReg & 7) << 3) + (DestReg & 7)) }; RainDump(B, op, 3); }
static inline void RainX86Add(RainBin* B, int SrcReg, int DestReg) { uint8_t op[] = { 0x48, 0x01, (uint8_t)(0xC0 + ((SrcReg & 7) << 3) + (DestReg & 7)) }; RainDump(B, op, 3); }
static inline void RainX86Sub(RainBin* B, int SrcReg, int DestReg) { uint8_t op[] = { 0x48, 0x29, (uint8_t)(0xC0 + ((SrcReg & 7) << 3) + (DestReg & 7)) }; RainDump(B, op, 3); }
static inline void RainX86Mul(RainBin* B, int SrcReg) { uint8_t op[] = { 0x48, 0xF7, (uint8_t)(0xE0 + (SrcReg & 7)) }; RainDump(B, op, 3); }
static inline void RainX86Div(RainBin* B, int SrcReg) { uint8_t op[] = { 0x48, 0xF7, (uint8_t)(0xF0 + (SrcReg & 7)) }; RainDump(B, op, 3); }

static inline void RainX86CallLabel(RainBin* B, int LabelId) {
    uint8_t op = 0xE8; RainDump(B, &op, 1);
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_X86_REL32};
    int32_t dummy = 0; RainDump(B, (uint8_t*)&dummy, 4);
}
static inline void RainX86JmpLabel(RainBin* B, int LabelId) {
    uint8_t op = 0xE9; RainDump(B, &op, 1);
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_X86_REL32};
    int32_t dummy = 0; RainDump(B, (uint8_t*)&dummy, 4);
}
static inline void RainX86JeLabel(RainBin* B, int LabelId) {
    uint8_t op[] = { 0x0F, 0x84 }; RainDump(B, op, 2);
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_X86_REL32};
    int32_t dummy = 0; RainDump(B, (uint8_t*)&dummy, 4);
}
static inline void RainX86JzLabel(RainBin* B, int LabelId) { RainX86JeLabel(B, LabelId); }
static inline void RainX86JlLabel(RainBin* B, int LabelId) {
    uint8_t op[] = { 0x0F, 0x8C }; RainDump(B, op, 2);
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_X86_REL32};
    int32_t dummy = 0; RainDump(B, (uint8_t*)&dummy, 4);
}
static inline void RainX86JgLabel(RainBin* B, int LabelId) {
    uint8_t op[] = { 0x0F, 0x8F }; RainDump(B, op, 2);
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_X86_REL32};
    int32_t dummy = 0; RainDump(B, (uint8_t*)&dummy, 4);
}


static inline void RainArm64Add(RainBin* B, int Src, int Dest) { uint32_t op = 0x8B000000 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainArm64Sub(RainBin* B, int Src, int Dest) { uint32_t op = 0xCB000000 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainArm64Mul(RainBin* B, int Src, int Dest) { uint32_t op = 0x9B007C00 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainArm64Div(RainBin* B, int Src, int Dest) { uint32_t op = 0x9AC00C00 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainArm64And(RainBin* B, int Src, int Dest) { uint32_t op = 0x8A000000 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainArm64Or(RainBin* B, int Src, int Dest)  { uint32_t op = 0xAA000000 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainArm64Xor(RainBin* B, int Src, int Dest) { uint32_t op = 0xCA000000 | ((Src & 31) << 16) | ((Dest & 31) << 5) | (Dest & 31); RainDump(B, (uint8_t*)&op, 4); }

static inline void RainArm64JmpLabel(RainBin* B, int LabelId) {
    uint32_t op = 0x14000000;
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_ARM64_REL26};
    RainDump(B, (uint8_t*)&op, 4);
}
static inline void RainArm64CallLabel(RainBin* B, int LabelId) {
    uint32_t op = 0x94000000;
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_ARM64_REL26};
    RainDump(B, (uint8_t*)&op, 4);
}

static inline void RainRiscVAdd(RainBin* B, int Src, int Dest) { uint32_t op = 0x00000033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainRiscVSub(RainBin* B, int Src, int Dest) { uint32_t op = 0x40000033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainRiscVMul(RainBin* B, int Src, int Dest) { uint32_t op = 0x02000033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainRiscVDiv(RainBin* B, int Src, int Dest) { uint32_t op = 0x02004033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainRiscVAnd(RainBin* B, int Src, int Dest) { uint32_t op = 0x00007033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainRiscVOr(RainBin* B, int Src, int Dest)  { uint32_t op = 0x00006033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }
static inline void RainRiscVXor(RainBin* B, int Src, int Dest) { uint32_t op = 0x00004033 | ((Dest & 31) << 7) | ((Dest & 31) << 15) | ((Src & 31) << 20); RainDump(B, (uint8_t*)&op, 4); }

static inline void RainRiscVJmpLabel(RainBin* B, int LabelId) {
    uint32_t op = 0x000000ef;
    if (B->JumpPatchCount < RAIN_MAX_JUMPS) B->JumpPatches[B->JumpPatchCount++] = (RainJumpPatch){B->Size, LabelId, PATCH_RISCV_REL20};
    RainDump(B, (uint8_t*)&op, 4);
}

#ifndef RAIN_ELF_STRUCTURES
#define RAIN_ELF_STRUCTURES
typedef struct {
    uint8_t  e_ident[16]; uint16_t e_type; uint16_t e_machine; uint32_t e_version;
    uint64_t e_entry; uint64_t e_phoff; uint64_t e_shoff; uint32_t e_flags;
    uint16_t e_ehsize; uint16_t e_phentsize; uint16_t e_phnum;
    uint16_t e_shentsize; uint16_t e_shnum; uint16_t e_shstrndx;
} RainElf64Ehdr;

typedef struct {
    uint32_t sh_name; uint32_t sh_type; uint64_t sh_flags; uint64_t sh_addr;
    uint64_t sh_offset; uint64_t sh_size; uint32_t sh_link; uint32_t sh_info;
    uint64_t sh_addralign; uint64_t sh_entsize;
} RainElf64Shdr;

typedef struct {
    uint32_t st_name; uint8_t st_info; uint8_t st_other; uint16_t st_shndx;
    uint64_t st_value; uint64_t st_size;
} RainElf64Sym;
#endif

#ifndef RAIN_COFF_STRUCTURES
#define RAIN_COFF_STRUCTURES
#pragma pack(push, 1)
typedef struct {
    uint16_t Machine; uint16_t NumberOfSections; uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable; uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader; uint16_t Characteristics;
} RainCoffHeader;

typedef struct {
    char Name[8]; uint32_t VirtualSize; uint32_t VirtualAddress;
    uint32_t SizeOfRawData; uint32_t PointerToRawData; uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers; uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers; uint32_t Characteristics;
} RainCoffSectionHeader;

typedef struct {
    union {
        char ShortName[8];
        struct { uint32_t Zeroes; uint32_t Offset; } LongName;
    } Name;
    uint32_t Value; int16_t SectionNumber; uint16_t Type;
    uint8_t StorageClass; uint8_t NumberOfAuxSymbols;
} RainCoffSymbol;
#pragma pack(pop)
#endif

#ifndef RAIN_MACHO_STRUCTURES
#define RAIN_MACHO_STRUCTURES
typedef struct {
    uint32_t magic; int32_t cputype; int32_t cpusubtype; uint32_t filetype;
    uint32_t ncmds; uint32_t sizeofcmds; uint32_t flags; uint32_t reserved;
} RainMachHeader64;

typedef struct {
    uint32_t cmd; uint32_t cmdsize; char segname[16]; uint64_t vmaddr;
    uint64_t vmsize; uint64_t fileoff; uint64_t filesize; int32_t maxprot;
    int32_t initprot; uint32_t nsects; uint32_t flags;
} RainMachSegmentCommand64;

typedef struct {
    char sectname[16]; char segname[16]; uint64_t addr; uint64_t size;
    uint32_t offset; uint32_t align; uint32_t reloff; uint32_t nreloc;
    uint32_t flags; uint32_t reserved1; uint32_t reserved2; uint32_t reserved3;
} RainMachSection64;
#endif

static inline void rainExportAsLinuxExecutable(const char* OutFilename, RainBin* B) {
    FILE* f = fopen(OutFilename, "wb");
    if (!f) return;
    size_t headerSz = 64 + 56; uint64_t baseAddr = 0x400000;
    RainElf64Ehdr ehdr = {
        .e_type = 2, .e_machine = 0x3E, .e_version = 1, .e_entry = baseAddr + headerSz,
        .e_phoff = 64, .e_shoff = 0, .e_flags = 0, .e_ehsize = 64, .e_phentsize = 56, .e_phnum = 1
    };
    memcpy(ehdr.e_ident, "\x7F" "ELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    struct {
        uint32_t p_type, p_flags; uint64_t p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align;
    } phdr = { .p_type = 1, .p_flags = 7, .p_offset = 0, .p_vaddr = baseAddr, .p_paddr = baseAddr, .p_filesz = headerSz + B->Size, .p_memsz = headerSz + B->Size, .p_align = 0x1000 };
    fwrite(&ehdr, 64, 1, f); fwrite(&phdr, 56, 1, f); fwrite(B->Code, 1, B->Size, f); fclose(f);
    chmod(OutFilename, 0755);
    printf("[Rain] Linux ELF Executable Build Success: %s\n", OutFilename);
}

static inline void rainExportAsLinuxObject(const char* OutFilename, RainBin* B) {
    FILE* f = fopen(OutFilename, "wb"); if (!f) return;
    RainElf64Ehdr ehdr = { .e_type = 1, .e_machine = 0x3E, .e_version = 1, .e_shoff = sizeof(RainElf64Ehdr) + B->Size, .e_ehsize = sizeof(RainElf64Ehdr), .e_shentsize = sizeof(RainElf64Shdr), .e_shnum = 3, .e_shstrndx = 2 };
    memcpy(ehdr.e_ident, "\x7F" "ELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    fwrite(&ehdr, sizeof(RainElf64Ehdr), 1, f); size_t textOffset = ftell(f);
    fwrite(B->Code, 1, B->Size, f); const char* shstrtab = "\0.text\0.shstrtab\0"; size_t shstrtabOffset = ftell(f); fwrite(shstrtab, 1, 18, f);
    RainElf64Shdr shdrs[3] = { {0}, {.sh_name = 1, .sh_type = 1, .sh_flags = 6, .sh_offset = textOffset, .sh_size = B->Size, .sh_addralign = 16}, {.sh_name = 7, .sh_type = 3, .sh_offset = shstrtabOffset, .sh_size = 18, .sh_addralign = 1} };
    fseek(f, ehdr.e_shoff, SEEK_SET); fwrite(shdrs, sizeof(RainElf64Shdr), 3, f); fclose(f);
}

static inline void rainLinkLinuxObject(RainBin* B, const char* ObjFilename, const char* FuncName) {
    FILE* f = fopen(ObjFilename, "rb"); if (!f) return;
    RainElf64Ehdr ehdr; fread(&ehdr, sizeof(RainElf64Ehdr), 1, f);
    RainElf64Shdr shdr, textShdr, symtabShdr, strtabShdr; int hasText=0, hasSym=0, hasStr=0;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        fseek(f, ehdr.e_shoff + (i * ehdr.e_shentsize), SEEK_SET); fread(&shdr, sizeof(RainElf64Shdr), 1, f);
        if (shdr.sh_type == 1) { textShdr = shdr; hasText = 1; }
        else if (shdr.sh_type == 2) { symtabShdr = shdr; hasSym = 1; }
        else if (shdr.sh_type == 3) { strtabShdr = shdr; hasStr = 1; }
    }
    if (hasText && hasSym && hasStr) {
        uint8_t* textCode = (uint8_t*)malloc(textShdr.sh_size); fseek(f, textShdr.sh_offset, SEEK_SET); fread(textCode, 1, textShdr.sh_size, f);
        size_t baseOffset = B->Size; RainDump(B, textCode, textShdr.sh_size);
        size_t numSyms = symtabShdr.sh_size / symtabShdr.sh_entsize; char* strtab = (char*)malloc(strtabShdr.sh_size); fseek(f, strtabShdr.sh_offset, SEEK_SET); fread(strtab, 1, strtabShdr.sh_size, f);
        for (size_t i = 0; i < numSyms; i++) {
            RainElf64Sym sym; fseek(f, symtabShdr.sh_offset + (i * symtabShdr.sh_entsize), SEEK_SET); fread(&sym, sizeof(RainElf64Sym), 1, f);
            if (strcmp(strtab + sym.st_name, FuncName) == 0) { printf("[Rain Linker] Linux %s linked at offset: %zu\n", FuncName, baseOffset + sym.st_value); break; }
        }
        free(textCode); free(strtab);
    }
    fclose(f);
}

// i hope it works? i am not using mac & win so dont excpet much you suckers

static inline void rainExportAsWindowsExecutable(const char* OutFilename, RainBin* B) {
    FILE* f = fopen(OutFilename, "wb"); if (!f) return;
    uint8_t mzStub[64] = {0}; mzStub[0] = 'M'; mzStub[1] = 'Z'; mzStub[0x3C] = 0x40; fwrite(mzStub, 1, 64, f);
    uint32_t peSig = 0x00004550; fwrite(&peSig, 4, 1, f);
    RainCoffHeader coff = { .Machine = 0x8664, .NumberOfSections = 1, .Characteristics = 0x0022 }; fwrite(&coff, sizeof(RainCoffHeader), 1, f);
    RainCoffSectionHeader textSec = { .Name = ".text", .SizeOfRawData = B->Size, .PointerToRawData = 512, .Characteristics = 0x60000020 }; fwrite(&textSec, sizeof(RainCoffSectionHeader), 1, f);
    fseek(f, 512, SEEK_SET); fwrite(B->Code, 1, B->Size, f); fclose(f);
}

static inline void rainExportAsWindowsObject(const char* OutFilename, RainBin* B) {
    FILE* f = fopen(OutFilename, "wb"); if (!f) return;
    RainCoffHeader coff = { .Machine = 0x8664, .NumberOfSections = 1, .PointerToSymbolTable = sizeof(RainCoffHeader) + sizeof(RainCoffSectionHeader) + B->Size };
    RainCoffSectionHeader textSec = { .Name = ".text", .SizeOfRawData = B->Size, .PointerToRawData = sizeof(RainCoffHeader) + sizeof(RainCoffSectionHeader), .Characteristics = 0x60000020 };
    fwrite(&coff, sizeof(RainCoffHeader), 1, f); fwrite(&textSec, sizeof(RainCoffSectionHeader), 1, f); fwrite(B->Code, 1, B->Size, f); fclose(f);
}

static inline void rainLinkWindowsObject(RainBin* B, const char* ObjFilename, const char* FuncName) {
    FILE* f = fopen(ObjFilename, "rb"); if (!f) return;
    RainCoffHeader coff; fread(&coff, sizeof(RainCoffHeader), 1, f); RainCoffSectionHeader shdr; int hasText = 0;
    for (int i = 0; i < coff.NumberOfSections; i++) {
        fseek(f, sizeof(RainCoffHeader) + (i * sizeof(RainCoffSectionHeader)), SEEK_SET); fread(&shdr, sizeof(RainCoffSectionHeader), 1, f);
        if (strncmp(shdr.Name, ".text", 5) == 0) { hasText = 1; break; }
    }
    if (hasText) {
        uint8_t* textCode = (uint8_t*)malloc(shdr.SizeOfRawData); fseek(f, shdr.PointerToRawData, SEEK_SET); fread(textCode, 1, shdr.SizeOfRawData, f);
        size_t baseOffset = B->Size; RainDump(B, textCode, shdr.SizeOfRawData);
        uint32_t strtabPos = coff.PointerToSymbolTable + (coff.NumberOfSymbols * sizeof(RainCoffSymbol));
        for (uint32_t i = 0; i < coff.NumberOfSymbols; i++) {
            RainCoffSymbol sym; fseek(f, coff.PointerToSymbolTable + (i * sizeof(RainCoffSymbol)), SEEK_SET); fread(&sym, sizeof(RainCoffSymbol), 1, f);
            char symName[256] = {0};
            if (sym.Name.LongName.Zeroes == 0) { fseek(f, strtabPos + sym.Name.LongName.Offset, SEEK_SET); int ch, len = 0; while ((ch = fgetc(f)) != EOF && ch != 0 && len < 255) symName[len++] = ch; }
            else { memcpy(symName, sym.Name.ShortName, 8); }
            char* cleanName = (symName[0] == '_') ? symName + 1 : symName;
            if (strcmp(cleanName, FuncName) == 0) { printf("[Rain Linker] Windows %s linked at offset: %zu\n", FuncName, baseOffset + sym.Value); break; }
            i += sym.NumberOfAuxSymbols;
        }
        free(textCode);
    }
    fclose(f);
}

static inline void rainExportAsMacExecutable(const char* OutFilename, RainBin* B) {
    FILE* f = fopen(OutFilename, "wb"); if (!f) return;
    RainMachHeader64 hdr = { .magic = 0xFEEDFACF, .cputype = 0x01000007, .cpusubtype = 3, .filetype = 2, .ncmds = 1, .sizeofcmds = sizeof(RainMachSegmentCommand64), .flags = 0x85 };
    RainMachSegmentCommand64 seg = { .cmd = 0x19, .cmdsize = sizeof(RainMachSegmentCommand64), .segname = "__TEXT", .vmaddr = 0x100000000, .vmsize = 0x1000, .filesize = sizeof(RainMachHeader64) + sizeof(RainMachSegmentCommand64) + B->Size, .maxprot = 7, .initprot = 5 };
    fwrite(&hdr, sizeof(RainMachHeader64), 1, f); fwrite(&seg, sizeof(RainMachSegmentCommand64), 1, f); fwrite(B->Code, 1, B->Size, f); fclose(f); chmod(OutFilename, 0755);
}

static inline void rainExportAsMacObject(const char* OutFilename, RainBin* B) {
    FILE* f = fopen(OutFilename, "wb"); if (!f) return;
    RainMachHeader64 hdr = { .magic = 0xFEEDFACF, .cputype = 0x01000007, .cpusubtype = 3, .filetype = 1, .ncmds = 1, .sizeofcmds = sizeof(RainMachSegmentCommand64) };
    RainMachSegmentCommand64 seg = { .cmd = 0x19, .cmdsize = sizeof(RainMachSegmentCommand64), .vmsize = B->Size, .fileoff = sizeof(RainMachHeader64) + sizeof(RainMachSegmentCommand64), .filesize = B->Size, .maxprot = 7, .initprot = 7, .nsects = 1 };
    fwrite(&hdr, sizeof(RainMachHeader64), 1, f); fwrite(&seg, sizeof(RainMachSegmentCommand64), 1, f); fwrite(B->Code, 1, B->Size, f); fclose(f);
}

static inline void rainLinkMacObject(RainBin* B, const char* ObjFilename, const char* FuncName) {
    FILE* f = fopen(ObjFilename, "rb"); if (!f) return;
    fseek(f, 0, SEEK_END); size_t sz = ftell(f); fseek(f, sizeof(RainMachHeader64) + sizeof(RainMachSegmentCommand64), SEEK_SET);
    size_t codeSz = sz - (sizeof(RainMachHeader64) + sizeof(RainMachSegmentCommand64)); uint8_t* buf = (uint8_t*)malloc(codeSz); fread(buf, 1, codeSz, f);
    RainDump(B, buf, codeSz); free(buf); fclose(f);
    printf("[Rain Linker] Mac Mach-O linked: %s\n", FuncName);
}

#endif
