#include "CatInstructionVisitor.hpp"
#include "CatFunction.hpp"

void CatInstructionVisitor::addModification(llvm::Value* value, llvm::Value* inst) {
    valueModificationMap_[value].insert(inst);  // operator[] will default construct vector if it's not there
}

void CatInstructionVisitor::checkVariableEscape(llvm::CallInst* callInst) {
    for (auto it = callInst->arg_begin(); it != callInst->arg_end(); ++it) {
        if (llvm::isa<llvm::Instruction>(*it)) {
            escapedInstructions_.insert(llvm::cast<llvm::Instruction>(*it));
        }
    }
}

void CatInstructionVisitor::visitInstruction(llvm::Instruction& inst) {
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
        } else {
            checkVariableEscape(callInst);
        }
    }

    llvm::PHINode* phiNode = llvm::dyn_cast<llvm::PHINode>(&inst);
    if (phiNode) {
        addModification(phiNode, phiNode);
    }
}

