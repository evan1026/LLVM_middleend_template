#include "llvm/Support/raw_ostream.h"

#include "CatVariableModificationVisitor.hpp"
#include "CatFunction.hpp"

void CatVariableModificationVisitor::visitCallInst(llvm::CallInst& callInst) {
    const CatFunction* func = CatFunction::get(callInst.getCalledFunction()->getName().str());
    if (func) {
        if (func->isModification()) {
            llvm::errs() << callInst.getCaller()->getName() << " ";
            if (func->isInitialAssignment()) {
                llvm::errs() << callInst << " " << callInst;
            } else if (callInst.getArgOperand(0)) {  // Should never be null but doesn't hurt to check
                llvm::errs() << *callInst.getArgOperand(0) << " " << callInst;  // operand 0 bc the modified variable is always the first in each function
            }
            llvm::errs() << "\n";
        }
    }
}
