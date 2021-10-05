#pragma once

#include <vector>

#include <llvm/IR/InstVisitor.h>

using MAP_TYPE = std::map<llvm::Value*, std::vector<llvm::CallInst*>>;

/**
 * An llvm::InstVisitor that collects call instructions and stores them in various
 * data structures.
 */
class CatCallInstVisitor : public llvm::InstVisitor<CatCallInstVisitor> {

    std::vector<llvm::CallInst*> callInstructions_; // After H1 this will probably be changed to only store CAT instructions
    MAP_TYPE valueModificationMap_; // Maps values to all instructions that modify them

    /**
     * Adds the call instruction to the value modification map for the given value.
     */
    void addModification(llvm::Value*, llvm::CallInst*);

    public:

        /**
         * Constructor.
         */
        CatCallInstVisitor() = default;

        /**
         * Processes call instructions.
         *
         * This function is called once per llvm::CallInst that exists in the IR.
         * It will look at the instruction and store it in the data structures
         * appropriately
         *
         * @param callInst The call instruction to process
         */
        void visitCallInst(llvm::CallInst& callInst);

        /**
         * Gets all processed call instructions.
         *
         * @return A vector of call instructions that have been processed by the visitor.
         */
        const auto& getCallInstructions() const { return callInstructions_; }

        /**
         * Gets a map from llvm::Value pointers to a list of pointers to the call instructions that modify them.
         *
         * @return The map
         */
        const MAP_TYPE& getValueModifications() const { return valueModificationMap_; }
};
