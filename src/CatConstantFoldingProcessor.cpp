#include "llvm/Support/raw_ostream.h"

#include "CatConstantFoldingProcessor.hpp"
#include "CatFunction.hpp"

void CatConstantFoldingProcessor::calculate(std::vector<llvm::Instruction*> instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
    llvm::errs() << "Doing constant folding\n";
    for (auto& inst : instructions) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        bool phiFound = false;
        if (callInst) {
            const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
            if (func) {
                if (func->isCalculation()) {
                    // At this point we know we're looking at a CAT instruction and it is a candidate for constant folding
                    auto dataDeps = dataDepsMap.at(callInst);

                    llvm::errs() << "Analysing for constant folding: " << *callInst << "\n";

                    llvm::Value* arg1Const = nullptr;
                    llvm::Value* arg2Const = nullptr;

                    for (llvm::Instruction* value : dataDeps.instInSet) {
                        llvm::CallInst* callValue = llvm::dyn_cast<llvm::CallInst>(value);
                        if (callValue) {
                            llvm::errs() << "    Checking value of: " << *value << "\n";
                            const CatFunction* func2 = CatFunction::get(callValue->getCalledFunction()->getName().str());
                            if (func2) {
                                if (func2->isInitialAssignment()) {
                                    if (callValue == callInst->getArgOperand(1)) {
                                        arg1Const = callValue->getArgOperand(0);;
                                        llvm::errs() << "        This instruction made operand 1 and is a constant (CAT_new)!\n";
                                    }
                                    if (callValue == callInst->getArgOperand(2)) {
                                        arg2Const = callValue->getArgOperand(0);
                                        llvm::errs() << "        This instruction made operand 2 and is a constant (CAT_new)!\n";
                                    }
                                } else if (func2->isModification() && !func2->isCalculation()) {
                                    if (callValue->getArgOperand(0) == callInst->getArgOperand(1)) {
                                        arg1Const = callValue->getArgOperand(1);
                                        llvm::errs() << "        This instruction made operand 1 and is a constant (CAT_set)!\n";
                                    }
                                    if (callValue->getArgOperand(0) == callInst->getArgOperand(2)) {
                                        arg2Const = callValue->getArgOperand(1);
                                        llvm::errs() << "        This instruction made operand 2 and is a constant (CAT_set)!\n";
                                    }
                                }
                            }
                        }

                        llvm::PHINode* phiNode = llvm::dyn_cast<llvm::PHINode>(value);
                        if (phiNode) {
                            if (phiNode == callInst->getArgOperand(1) || phiNode == callInst->getArgOperand(2)) {
                                llvm::errs() << "    PHI node exists for this value which means it could be undefined. We won't propagate.\n";
                                phiFound = true;
                            }
                        }
                    }

                    if (arg1Const != nullptr && arg2Const != nullptr && !phiFound) {
                        llvm::errs() << "    This is a constant expression!\n";
                        ArgPair pair{arg1Const, arg2Const};
                        replacements.insert({callInst, pair});
                    }
                }
            }
        }
    }
}

bool CatConstantFoldingProcessor::execute() {
    llvm::Function* catSetFunc = CatFunction::get("CAT_set")->getFunc();
    bool modification = false;
    for (auto it = replacements.begin(); it != replacements.end(); ++it) {
        llvm::errs() << "Generating const value for \"" << *it->first << "\" using \"" << *it->second.arg1 << "\" and \"" << *it->second.arg2 << "\"\n";
        const CatFunction* calcFunc = CatFunction::get(it->first->getCalledFunction()->getName().str());
        llvm::Value* result = calcFunc->applyOperation(it->second.arg1, it->second.arg2);
        if (result) {
            llvm::errs() << "    New val is " << *result << "\n";
            llvm::CallInst* newCatSet = llvm::CallInst::Create(catSetFunc, std::vector<llvm::Value*>(std::initializer_list<llvm::Value*>{it->first->getArgOperand(0), result}));
            newCatSet->insertBefore(it->first);
            it->first->eraseFromParent();
            llvm::errs() << "    New cat set: " << *newCatSet << "\n";
            modification = true;
        } else {
            llvm::errs() << "    Somthing went wrong with folding (probably the values weren't actually constants). Continuing...\n";
        }
    }
    return modification;
}
