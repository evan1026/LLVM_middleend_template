#pragma once

#include <map>
#include <vector>
#include <llvm/IR/Instructions.h>
#include "CatDataDependencies.hpp"
#include "CatFunction.hpp"

class CatConstantPropagationProcessor {

    std::map<llvm::CallInst*, llvm::Value*> replacements;

    llvm::Value* getReplaceValue(llvm::Value* catVar, llvm::CallInst* callValue, bool& nonConstFound);
    void processFunction(llvm::CallInst* callInst, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);

    public:
        void calculate(std::vector<llvm::Instruction*>& instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);
        bool execute();
};
