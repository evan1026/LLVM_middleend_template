#pragma once

#include <llvm/IR/InstVisitor.h>
#include <map>

class CatVariableModificationVisitor : public llvm::InstVisitor<CatVariableModificationVisitor> {

    std::unordered_map<llvm::Value*, llvm::CallInst*> valueMap;

    public:

        CatVariableModificationVisitor() : valueMap() {}

        void visitCallInst(llvm::CallInst& callInst);
};
