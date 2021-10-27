#include "llvm/Support/raw_ostream.h"

#include "CatConstantPropagationProcessor.hpp"
#include "CatFunction.hpp"

void CatConstantPropagationProcessor::calculateConstantPropagations(std::vector<llvm::Instruction*>& instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
    llvm::errs() << "Doing constant propagation\n";

    for (auto& inst : instructions) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        bool phiFound = false;
        bool nonConstFound = false;
        std::vector<llvm::Value*> foundValues;
        if (callInst) {
            const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
            if (func) {
                if (!func->isModification()) {
                    // At this point we know we're looking at a CAT instruction and it is a candidate for constant propagation
                    auto catVar = callInst->getArgOperand(0);
                    auto dataDeps = dataDepsMap.at(callInst);

                    llvm::errs() << "Analysing for constant propagation: " << *callInst << "\n";
                    int index = dataDeps.inSet.find_first();
                    while (index != -1) {
                        llvm::Instruction* value = instructions[index];
                        llvm::CallInst* callValue = llvm::dyn_cast<llvm::CallInst>(value);
                        llvm::Value* replaceValue = nullptr;
                        if (callValue) {
                            llvm::errs() << "    Checking value of: " << *value << "\n";
                            const CatFunction* func2 = CatFunction::get(callValue->getCalledFunction()->getName().str());
                            if (func2) {
                                if (func2->isInitialAssignment()) {
                                    if (callValue == catVar) {
                                        llvm::errs() << "        This instruction made the operand and is a constant (CAT_new)!\n";
                                        replaceValue = callValue->getArgOperand(0);
                                    }
                                } else if (func2->isModification()) {
                                    if (func2->isCalculation()) {
                                        nonConstFound = true;
                                    } else {
                                        if (callValue->getArgOperand(0) == catVar) {
                                            llvm::errs() << "        This instruction made the operand and is a constant (CAT_set)!\n";
                                            replaceValue = callValue->getArgOperand(1);
                                        }
                                    }
                                }
                            }
                        }

                        llvm::PHINode* phi = llvm::dyn_cast<llvm::PHINode>(value);
                        if (phi && phi == catVar) {
                            llvm::errs() << "PHI node exists for this value which means it could be undefined. We won't propagate.\n";
                            phiFound = true;
                        }

                        if (replaceValue != nullptr) {
                            foundValues.push_back(replaceValue);
                        }
                        index = dataDeps.inSet.find_next(index);
                    }

                    bool allEqual = true;
                    for (std::size_t i = 0; i + 1 < foundValues.size(); ++i) {
                        if (foundValues[i] != foundValues[i + 1]) {
                            allEqual = false;
                        }
                    }

                    if (!phiFound && !nonConstFound && allEqual && foundValues.size() > 0) {
                        replacements.insert({callInst, foundValues[0]});
                    }
                }
            }
        }
    }
}

bool CatConstantPropagationProcessor::doReplacements(llvm::Function* catSetFunc) {
    bool modification = false;
    for (auto it = replacements.begin(); it != replacements.end(); ++it) {
        llvm::errs() << "We will replace \"" << *it->first << "\" with \"" << *it->second << "\"\n";
        it->first->replaceAllUsesWith(it->second);
        it->first->eraseFromParent();
        modification = true;
    }
    return modification;
}
