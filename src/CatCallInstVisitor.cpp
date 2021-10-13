#include "CatCallInstVisitor.hpp"
#include "CatFunction.hpp"

void CatCallInstVisitor::addModification(llvm::Value* value, llvm::CallInst* callInst) {
    valueModificationMap_[value].insert(callInst);  // operator[] will default construct vector if it's not there
}

void CatCallInstVisitor::visitInstruction(llvm::Instruction& inst) {
    mappedInstructions_.push_back(&inst);

    llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
    if (callInst) {
        const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
        if (func) {
            if (func->isModification()) {
                if (func->isInitialAssignment()) {
                    addModification(callInst, callInst);
                } else if (callInst->getArgOperand(0)) {
                    addModification(callInst->getArgOperand(0), callInst);
                }
            }
        }
    }
}

