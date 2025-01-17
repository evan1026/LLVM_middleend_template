#pragma once

#include <llvm/ADT/SmallBitVector.h>
#include <llvm/IR/Instructions.h>
#include <unordered_set>

/**
 * Data dependency information for the CAT language. Currently contains
 * genSet and killSet, but later will also have IN and OUT sets. The sets
 * are stored as llvm::SmallBitVector objects where each call instruction
 * in the IR is a bit (this saves some space since we know that CAT operations
 * will always be call instructions, so we don't have to store other instruction
 * types.
 */
struct CatDataDependencies {

    /**
     * GEN set for an instruction.
     */
    llvm::SmallBitVector genSet;

    /**
     * KILL set for an instruction.
     */
    llvm::SmallBitVector killSet;

    /**
     * IN set for an instruction.
     */
    llvm::SmallBitVector inSet;

    /**
     * OUT set for an instruction.
     */
    llvm::SmallBitVector outSet;

    std::unordered_set<llvm::Value*> instGenSet;
    std::unordered_set<llvm::Value*> instKillSet;
    std::unordered_set<llvm::Value*> instInSet;
    std::unordered_set<llvm::Value*> instOutSet;


    /**
     * Constructor.
     *
     * @param genSet_ GEN set for an instruction
     * @param killSet_ KILL set for an instruction
     */
    CatDataDependencies(const llvm::SmallBitVector& genSet_, const llvm::SmallBitVector& killSet_) :
        genSet(genSet_),
        killSet(killSet_),
        inSet(),
        outSet(),
        instGenSet(),
        instKillSet(),
        instInSet(),
        instOutSet()
    {}

    /**
     * Constructor.
     */
    CatDataDependencies() :
        genSet(),
        killSet(),
        inSet(),
        outSet(),
        instGenSet(),
        instKillSet(),
        instInSet(),
        instOutSet()
    {}

    void generateInstructionSets(const std::vector<llvm::Value*> instructions);
};
