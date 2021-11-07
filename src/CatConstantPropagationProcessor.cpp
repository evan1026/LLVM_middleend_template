#include "llvm/Support/raw_ostream.h"

#include "CatConstantPropagationProcessor.hpp"

std::vector<llvm::Value*> CatConstantPropagationProcessor::getReplaceValues(llvm::Value* catVar, llvm::Value* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Value*>& exploredNodes) {

    if (llvm::isa<llvm::CallInst>(replaceCandidate)) {
        return getCallInstReplaceValues(catVar, llvm::cast<llvm::CallInst>(replaceCandidate), nonConstFound, exploredNodes);
    } else if (llvm::isa<llvm::PHINode>(replaceCandidate)) {
        return getPhiNodeReplaceValues(catVar, llvm::cast<llvm::PHINode>(replaceCandidate), nonConstFound, exploredNodes);
    } else if (llvm::isa<llvm::LoadInst>(replaceCandidate)) {
        return {replaceCandidate};
    }

    return {};
}

std::vector<llvm::Value*> CatConstantPropagationProcessor::getCallInstReplaceValues(llvm::Value* catVar, llvm::CallInst* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Value*>& exploredNodes) {
    std::vector<llvm::Value*> out;
    llvm::errs() << "    Checking value of: " << *replaceCandidate << "\n";

    const CatFunction* func = CatFunction::get(replaceCandidate->getCalledFunction()->getName().str());
    if (func) {
        if (func->isInitialAssignment()) {
            if (replaceCandidate == catVar) {
                llvm::errs() << "        This instruction made the operand and is a constant (CAT_new)!\n";
                out.push_back(replaceCandidate->getArgOperand(0));
            }
        } else if (func->isModification()) {
            if (func->isCalculation()) {
                nonConstFound = true;
                llvm::errs() << "        This instruction made the operand is not a constant!\n";
            } else {
                if (replaceCandidate->getArgOperand(0) == catVar) {
                    llvm::errs() << "        This instruction made the operand and is a constant (CAT_set)!\n";
                    out.push_back(replaceCandidate->getArgOperand(1));
                }
            }
        }
    }
    return out;
}

std::vector<llvm::Value*> CatConstantPropagationProcessor::getPhiNodeReplaceValues(llvm::Value* catVar, llvm::PHINode* replaceCandidate, bool& nonConstFound, std::unordered_set<llvm::Value*>& exploredNodes) {
    std::vector<llvm::Value*> out;

    if (replaceCandidate == catVar) {
        llvm::errs() << "    PHI node exists for this value " << *replaceCandidate << "\n";
        for (auto& phiUse : replaceCandidate->incoming_values()) {
            llvm::Instruction* originatingInst = llvm::dyn_cast<llvm::Instruction>(phiUse.get());
            if (originatingInst) {
                if (exploredNodes.find(originatingInst) == exploredNodes.end()) {
                    exploredNodes.insert(originatingInst);
                    std::vector<llvm::Value*> replaceVals = getReplaceValues(originatingInst, originatingInst, nonConstFound, exploredNodes);
                    out.insert(out.end(), replaceVals.begin(), replaceVals.end());
                }
            } else if (llvm::isa<llvm::Argument>(phiUse.get())) {
                llvm::errs() << "        " << *phiUse.get() << " is an argument, so we won't be able to propagate.\n";
                nonConstFound = true;
            }
        }
    }

    return out;
}

void CatConstantPropagationProcessor::processFunction(llvm::CallInst* callInst, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap, std::unordered_set<llvm::Value*>& escapedVariables) {
    // At this point we know we're looking at a CAT instruction and it is a candidate for constant propagation
    auto catVar = callInst->getArgOperand(0);
    auto dataDeps = dataDepsMap.at(callInst);
    bool nonConstFound = false;
    std::vector<llvm::Value*> foundValues;

    llvm::errs() << "Analysing for constant propagation: " << *callInst << "\n";
    llvm::errs() << "    Modified CAT var: " << *catVar << "\n";

    if (llvm::isa<llvm::Argument>(catVar)) {
        llvm::errs() << "    CAT var is a function argument. We can't propagate it\n";
        return;
    } else if (llvm::isa<llvm::CallInst>(catVar) && escapedVariables.find(llvm::cast<llvm::CallInst>(catVar)) != escapedVariables.end()) {
        llvm::errs() << "    CAT var escapes. We can't propagate it\n";
        return;
    }

    for (llvm::Value* value : dataDeps.instInSet) {
        std::unordered_set<llvm::Value*> exploredNodes;
        std::vector<llvm::Value*> possibleReplacements = getReplaceValues(catVar, value, nonConstFound, exploredNodes);
        foundValues.insert(foundValues.end(), possibleReplacements.begin(), possibleReplacements.end());
    }

    bool allEqual = true;
    for (std::size_t i = 0; i + 1 < foundValues.size(); ++i) {
        llvm::errs() << "    Possible replacement candidate: " << *foundValues[i] << "\n";
        if (foundValues[i] != foundValues[i + 1]) {
            allEqual = false;
        }
    }
    if (foundValues.size() > 0) {
        llvm::errs() << "    Possible replacement candidate: " << *foundValues[foundValues.size() - 1] << "\n";
    }

    if (!nonConstFound && allEqual && foundValues.size() > 0) {
        replacements.insert({callInst, foundValues[0]});
    } else if (!allEqual) {
        llvm::errs() << "    Not all values are equal, so we can't propagate\n";
    } else if (nonConstFound) {
        llvm::errs() << "    Non-constant value found among replacements, so we can't propagate\n";
    }
}

void CatConstantPropagationProcessor::calculate(std::vector<llvm::Value*>& instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap, std::unordered_set<llvm::Value*>& escapedVariables) {
    llvm::errs() << "Doing constant propagation\n";

    for (auto& inst : instructions) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        if (callInst) {
            const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
            if (func && !func->isModification()) {
                processFunction(callInst, dataDepsMap, escapedVariables);
            }
        }
    }
}

bool CatConstantPropagationProcessor::execute() {
    bool modification = false;
    for (auto it = replacements.begin(); it != replacements.end(); ++it) {
        llvm::errs() << "We will replace \"" << *it->first << "\" with \"" << *it->second << "\"\n";
        it->first->replaceAllUsesWith(it->second);
        it->first->eraseFromParent();
        modification = true;
    }
    return modification;
}
