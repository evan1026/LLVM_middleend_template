#include "llvm/Support/raw_ostream.h"

#include "CatGenKillVisitor.hpp"
#include "CatFunction.hpp"

static void printBitVector(const llvm::SmallBitVector& bitVector, const std::vector<llvm::CallInst*>& callInstructions) {
    for (size_t i = 0; i < callInstructions.size(); ++i) {
        if (bitVector.test(i)) {
            llvm::errs() << " " << *callInstructions[i] << "\n";
        }
    }
}

static void printInstruction(const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::CallInst*>& callInstructions) {
    llvm::errs() << "INSTRUCTION: " << *callInst << "\n***************** GEN\n{\n";
    printBitVector(dataDeps.genSet, callInstructions);
    llvm::errs() << "}\n**************************************\n***************** KILL\n{\n";
    printBitVector(dataDeps.killSet, callInstructions);
    llvm::errs() << "}\n**************************************\n\n\n\n";
}

static llvm::Value* getModifiedValue(const CatFunction* func, llvm::CallInst& callInst) {
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
    llvm::SmallBitVector genSet(callInstructions_->size());
    llvm::SmallBitVector killSet(callInstructions_->size());

    if (llvm::isa<llvm::CallInst>(inst)) {
        llvm::CallInst& callInst = llvm::cast<llvm::CallInst>(inst);

        const CatFunction* func = CatFunction::get(callInst.getCalledFunction()->getName().str());
        if (func) {
            llvm::Value* modifiedValue = getModifiedValue(func, callInst);

            if (modifiedValue) {
                for (size_t i = 0; i < callInstructions_->size(); ++i) {
                    auto& thisInst = *(*callInstructions_)[i];
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
    }

    genKillMap_.emplace(std::piecewise_construct,
                        std::forward_as_tuple(&inst),
                        std::forward_as_tuple(genSet, killSet));

    // TODO: remove this after H1
    printInstruction(&inst, genKillMap_.at(&inst), *callInstructions_);
}
