#pragma once

#include <map>
#include <vector>
#include <llvm/IR/Instructions.h>
#include "CatDataDependencies.hpp"
#include "CatFunction.hpp"

class CatConstantPropagationProcessor {

    std::map<llvm::CallInst*, llvm::Value*> replacements;

    std::vector<llvm::Value*> getReplaceValues(llvm::Value* catVar, llvm::Instruction* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Instruction*>& exploredNodes);
    std::vector<llvm::Value*> getPhiNodeReplaceValues(llvm::Value* catVar, llvm::PHINode* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Instruction*>& exploredNodes);
    std::vector<llvm::Value*> getCallInstReplaceValues(llvm::Value* catVar, llvm::CallInst* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Instruction*>& exploredNodes);
    void processFunction(llvm::CallInst* callInst, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);

    public:
        void calculate(std::vector<llvm::Instruction*>& instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap);
        bool execute();
};
