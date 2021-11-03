#pragma once

#include <unordered_set>

#include <llvm/IR/InstVisitor.h>

using MAP_TYPE = std::map<llvm::Value*, std::unordered_set<llvm::Value*>>;

/**
 * An llvm::InstVisitor that collects call instructions and stores them in various
 * data structures.
 */
class CatInstructionVisitor : public llvm::InstVisitor<CatInstructionVisitor> {

    std::vector<llvm::Value*> mappedInstructions_; // List of all noteworthy instructions
    MAP_TYPE valueModificationMap_; // Maps values to all instructions that modify them
    std::unordered_set<llvm::Instruction*> escapedInstructions_;

    /**
     * Adds the call instruction to the value modification map for the given value.
     */
    void addModification(llvm::Value*, llvm::Value*);

    void checkVariableEscape(llvm::CallInst* callInst);

    public:

        /**
         * Constructor.
         */
        CatInstructionVisitor() = default;

        /**
         * Processes instructions.
         *
         * This function is called once per llvm::Instruction that exists in the IR.
         * It will look at the instruction and store it in the data structures
         * appropriately
         *
         * @param inst The instruction to process
         */
        void visitInstruction(llvm::Instruction& inst);

        /**
         * Gets all processed instructions.
         *
         * @return A vector of instructions that have been processed by the visitor.
         */
        auto& getMappedInstructions() { return mappedInstructions_; }

        /**
         * Gets a map from llvm::Value pointers to a list of pointers to the call instructions that modify them.
         *
         * @return The map
         */
        MAP_TYPE& getValueModifications() { return valueModificationMap_; }

        auto& getEscapedVars() { return escapedInstructions_; }
};
