#pragma once

#include <map>
#include <vector>
#include <llvm/IR/Instructions.h>
#include "CatDataDependencies.hpp"

class CatConstantPropagationProcessor {

    std::map<llvm::CallInst*, llvm::Value*> replacements;

    public:
        void calculate(std::vector<llvm::Instruction*>& instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);
        bool execute();
};
