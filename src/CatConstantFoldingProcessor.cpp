#include "llvm/Support/raw_ostream.h"

#include "CatConstantFoldingProcessor.hpp"
#include "CatFunction.hpp"

llvm::Value* CatConstantFoldingProcessor::getArg(llvm::Value* argOperand, llvm::CallInst* callValue) {
    const CatFunction* func = CatFunction::get(callValue->getCalledFunction()->getName().str());
    llvm::Value* out = nullptr;
    if (func) {
        if (func->isInitialAssignment()) {
            if (callValue == argOperand) {
                out = callValue->getArgOperand(0);
                llvm::errs() << "        This instruction made an operand and is a constant (CAT_new)!\n";
            }
        } else if (func->isModification() && !func->isCalculation()) {
            if (callValue->getArgOperand(0) == argOperand) {
                out = callValue->getArgOperand(1);
                llvm::errs() << "        This instruction made and operand and is a constant (CAT_set)!\n";
            }
        }
    }
    return out;
}

void CatConstantFoldingProcessor::processFunction(llvm::CallInst* callInst, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
    // At this point we know we're looking at a CAT instruction and it is a candidate for constant folding
    auto dataDeps = dataDepsMap.at(callInst);

    llvm::errs() << "Analysing for constant folding: " << *callInst << "\n";

    llvm::Value* arg1Const = nullptr;
    llvm::Value* arg2Const = nullptr;
    bool phiFound = false;

    for (llvm::Value* value : dataDeps.instInSet) {
        llvm::CallInst* callValue = llvm::dyn_cast<llvm::CallInst>(value);
        if (callValue) {
            llvm::errs() << "    Checking value of: " << *value << "\n";
            llvm::Value* arg1 = getArg(callInst->getArgOperand(1), callValue);
            llvm::Value* arg2 = getArg(callInst->getArgOperand(2), callValue);

            if (arg1 != nullptr) {
                arg1Const = arg1;
            }
            if (arg2 != nullptr) {
                arg2Const = arg2;
            }
        }

        llvm::PHINode* phiNode = llvm::dyn_cast<llvm::PHINode>(value);
        if (phiNode) {
            if (phiNode == callInst->getArgOperand(1) || phiNode == callInst->getArgOperand(2)) {
                llvm::errs() << "    PHI node exists for this value " << *phiNode << "\n";
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

void CatConstantFoldingProcessor::calculate(std::vector<llvm::Value*> instructions, const std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
    llvm::errs() << "Doing constant folding\n";
    for (auto& inst : instructions) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        if (callInst) {
            const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
            if (func && func->isCalculation()) {
                processFunction(callInst, dataDepsMap);
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
