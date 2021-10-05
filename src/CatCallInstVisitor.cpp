#include "CatCallInstVisitor.hpp"
#include "CatFunction.hpp"

void CatCallInstVisitor::addModification(llvm::Value* value, llvm::CallInst* callInst) {
    valueModificationMap_[value].push_back(callInst);  // operator[] will default construct vector if it's not there
}

void CatCallInstVisitor::visitCallInst(llvm::CallInst& callInst) {
    callInstructions_.push_back(&callInst);

    const CatFunction* func = CatFunction::get(callInst.getCalledFunction()->getName().str());
    if (func) {
        if (func->isModification()) {
            if (func->isInitialAssignment()) {
                addModification(&callInst, &callInst);
            } else if (callInst.getArgOperand(0)) {
                addModification(callInst.getArgOperand(0), &callInst);
            }
            llvm::errs() << "\n";
        }
    }
}
