#pragma once

#include <llvm/IR/InstVisitor.h>

/**
 * An llvm::InstVisitor that prints out every time a CAT variable is modified via
 * a call to the CAT API.
 */
class CatVariableModificationVisitor : public llvm::InstVisitor<CatVariableModificationVisitor> {

    public:

        /**
         * Constructor.
         */
        CatVariableModificationVisitor() = default;

        /**
         * Processes call instructions.
         *
         * This function is called once per llvm::CallInst that exists in the IR.
         * It will look at the instruction, determine if it's a call to a CAT
         * function, and if so it will print the appropriate data to llvm::errs().
         *
         * @param callInst The call instruction to process
         */
        void visitCallInst(llvm::CallInst& callInst) const;
};
