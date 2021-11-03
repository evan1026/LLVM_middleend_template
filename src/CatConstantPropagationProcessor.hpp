#pragma once

#include <map>
#include <vector>
#include <llvm/IR/Instructions.h>
#include "CatDataDependencies.hpp"
#include "CatFunction.hpp"

class CatConstantPropagationProcessor {

    std::map<llvm::CallInst*, llvm::Value*> replacements;

    std::vector<llvm::Value*> getReplaceValues(llvm::Value* catVar, llvm::Value* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Value*>& exploredNodes);
    std::vector<llvm::Value*> getPhiNodeReplaceValues(llvm::Value* catVar, llvm::PHINode* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Value*>& exploredNodes);
    std::vector<llvm::Value*> getCallInstReplaceValues(llvm::Value* catVar, llvm::CallInst* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Value*>& exploredNodes);
    void processFunction(llvm::CallInst* callInst, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap, std::unordered_set<llvm::Instruction*>& escapedVariables);

    public:
        void calculate(std::vector<llvm::Value*>& instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap, std::unordered_set<llvm::Instruction*>& escapedVariables);
        bool execute();
};
