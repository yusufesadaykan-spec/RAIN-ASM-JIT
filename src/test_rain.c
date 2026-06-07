#include "rain.h"
#include <stdio.h>

int main() {
    RainBin rb;
    RainBinInit(&rb);

    printf("[Rain Tutorial] Starting simple loop code generation...\n");

    // ============================================================================
    // STEP 1: Variable Allocation (Simulating 'int x = 10;' and 'int step = 1;')
    // ============================================================================
    // We move the immediate value 10 into REG_RCX (acts as variable x)
    // Assembly: mov rcx, 10
    RainX86MovRegImm(&rb, REG_RCX, 10);
    
    // We move the immediate value 1 into REG_RAX (acts as the step decrement)
    // Assembly: mov rax, 1
    RainX86MovRegImm(&rb, REG_RAX, 1);

    // ============================================================================
    // STEP 2: Define Label IDs for Control Flow
    // ============================================================================
    int labelLoopStart = 0;
    int labelLoopEnd = 1;

    // ============================================================================
    // STEP 3: Mark the Loop Start Anchor (Backward Jump Destination)
    // ============================================================================
    // This function saves the current 'rb.Size' into 'LabelOffsets[0]'
    RainLabel(&rb, labelLoopStart);

    // ============================================================================
    // STEP 4: Loop Body & Condition Modification (x = x - 1)
    // ============================================================================
    // Assembly: sub rcx, rax (rcx = rcx - rax)
    // This ALU operation automatically updates the CPU's Zero Flag (ZF).
    RainX86Sub(&rb, REG_RAX, REG_RCX); 

    // ============================================================================
    // STEP 5: Conditional Branching (Loop back if not zero!)
    // ============================================================================
    // Assembly: jnz <labelLoopStart>
    // CRITICAL CONCEPT: The exact relative offset to labelLoopStart is unknown yet.
    // Rain emits placeholder dummy bytes (00 00 00 00) and registers a patch entry.
    RainX86JnzLabel(&rb, labelLoopStart);

    // ============================================================================
    // STEP 6: Mark the Loop End Anchor
    // ============================================================================
    RainLabel(&rb, labelLoopEnd);

    // Emit function return instruction for x86_64
    // Assembly: ret
    uint8_t retOp = 0xC3;
    RainDump(&rb, &retOp, 1);

    printf("[Rain Tutorial] Code emission complete. Current buffer size: %zu bytes.\n", rb.Size);
    printf("[Rain Tutorial] Registered jump patches in registry: %d\n", rb.JumpPatchCount);

    // ============================================================================
    // STEP 7: THE MAGIC MOMENT (RainFinishBin)
    // ============================================================================
    // This function acts as the final resolution stage of our compiler backend:
    // 1. It opens the patch registry and looks up the absolute offset of labelLoopStart.
    // 2. It computes the precise mathematical relative distance from the JNZ instruction.
    // 3. It overwrites the (00 00 00 00) placeholder with the actual relative offset.
    // 4. Safely appends the reserved data section (.rdata) behind the code section.
    printf("[Rain Tutorial] Invoking RainFinishBin to execute automatic back-patching...\n");
    RainFinishBin(&rb);

    // ============================================================================
    // STEP 8: Exporting to Native Executable
    // ============================================================================
    // The patched, standalone machine code buffer is now converted into an ELF executable.
    rainExportAsLinuxExecutable("./rain_demo_bin", &rb);

    // Resource Cleanup
    RainBinFree(&rb);
    printf("[Rain Tutorial] Process successful! Raw executable './rain_demo_bin' generated.\n");

    return 0;
}
