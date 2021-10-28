#pragma once

#include <map>
#include <vector>
#include <llvm/IR/Instructions.h>
#include "CatDataDependencies.hpp"

class CatConstantFoldingProcessor {
    struct ArgPair {
      llvm::Value* arg1;
      llvm::Value* arg2;
    };

    std::map<llvm::CallInst*, ArgPair> replacements;

    void processFunction(llvm::CallInst* callInst, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);
    llvm::Value* getArg(llvm::Value* argOperand, llvm::CallInst* callValue);

    public:
        void calculate(std::vector<llvm::Instruction*> instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);
        bool execute();
};
