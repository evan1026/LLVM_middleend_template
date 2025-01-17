#include "llvm/Support/raw_ostream.h"

#include "CatGenKillVisitor.hpp"
#include "DataStructureOutput.hpp"

llvm::Value* CatGenKillVisitor::getModifiedValue(const CatFunction* func, llvm::CallInst& callInst) {
    llvm::Value* modifiedValue = nullptr;
    if (func->isModification()) {
        if (func->isInitialAssignment()) {
            modifiedValue = &callInst;
        } else {
            modifiedValue = callInst.getArgOperand(0);
        }
    }
    return modifiedValue;
}

void CatGenKillVisitor::visitInstruction(llvm::Instruction& inst) {
    llvm::SmallBitVector genSet(mappedInstructions_->size());
    llvm::SmallBitVector killSet(mappedInstructions_->size());

    if (llvm::isa<llvm::CallInst>(inst)) {
        llvm::CallInst& callInst = llvm::cast<llvm::CallInst>(inst);

        const CatFunction* func = CatFunction::get(callInst.getCalledFunction()->getName().str());
        if (func) {
            llvm::Value* modifiedValue = getModifiedValue(func, callInst);

            if (modifiedValue) {
                for (size_t i = 0; i < mappedInstructions_->size(); ++i) {
                    auto& thisInst = *(*mappedInstructions_)[i];
                    if (&thisInst == &callInst) {  // LLVM doesn't define == operators but each instruction is only at one address so we do pointer comparison
                        genSet.set(i);
                    } else if (valueModificationMap_->find(modifiedValue) != valueModificationMap_->end()) {
                        auto& otherFuncs = valueModificationMap_->at(modifiedValue);
                        if (otherFuncs.find(&thisInst) != otherFuncs.end()) {
                            killSet.set(i);
                        }
                    }
                }
            }
        }
    } else if (llvm::isa<llvm::PHINode>(inst)) {
        llvm::PHINode& phiNode = llvm::cast<llvm::PHINode>(inst);
        for (size_t i = 0; i < mappedInstructions_->size(); ++i) {
            auto& thisInst = *(*mappedInstructions_)[i];
            if (&thisInst == &phiNode) {  // LLVM doesn't define == operators but each instruction is only at one address so we do pointer comparison
                genSet.resize(i + 1);
                genSet.set(i);
            } else if (valueModificationMap_->find(&phiNode) != valueModificationMap_->end()) {
                auto& otherFuncs = valueModificationMap_->at(&phiNode);
                if (otherFuncs.find(&thisInst) != otherFuncs.end()) {
                    killSet.set(i);
                }
            }
        }
    }

    genKillMap_.emplace(std::piecewise_construct,
                        std::forward_as_tuple(&inst),
                        std::forward_as_tuple(genSet, killSet));

}

void CatGenKillVisitor::print() {
    for (llvm::Value* callInst : *mappedInstructions_) {
        if (llvm::isa<llvm::Instruction>(callInst)) {
            llvm::Instruction* actualInst = llvm::cast<llvm::Instruction>(callInst);
            printGenKillSets(llvm::errs(), actualInst, genKillMap_.at(actualInst), *mappedInstructions_);
        }
    }
}
