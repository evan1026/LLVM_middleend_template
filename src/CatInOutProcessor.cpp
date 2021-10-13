#include <llvm/IR/CFG.h>
#include <llvm/IR/Instructions.h>
#include "llvm/Support/raw_ostream.h"

#include "CatInOutProcessor.hpp"
#include "DataStructureOutput.hpp"

void CatInOutProcessor::processOnce(llvm::Function& func) {
    changesHappened_ = false;
    for (auto& bb : func) {
        processBasicBlock(bb);
    }
}

void CatInOutProcessor::processBasicBlock(llvm::BasicBlock& bb) {
    llvm::Instruction* prevInst = nullptr;
    CatDataDependencies& bbDeps = bbDataDepsMap_[&bb];

    bbDeps.inSet.clear();
    for (llvm::BasicBlock* pred : llvm::predecessors(&bb)) {
        bbDeps.inSet |= bbDataDepsMap_[pred].outSet; // Will automatically create empty entry if it doesn't exist
    }

    for (auto& inst : bb) {
        prevInst = processInstruction(inst, prevInst);
    }

    if (prevInst != nullptr) {
        bbDeps.outSet = dataDepsMap_->at(prevInst).outSet;
    }
}

llvm::Instruction* CatInOutProcessor::processInstruction(llvm::Instruction& inst, llvm::Instruction* prevInst) {
    llvm::Instruction* newPrevInst = prevInst;
    llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
    if (callInst) {
        newPrevInst = callInst;

        CatDataDependencies& instDeps = dataDepsMap_->at(callInst);
        llvm::SmallBitVector prevOutSet = instDeps.outSet;
        if (prevInst != nullptr) {
            instDeps.inSet = dataDepsMap_->at(prevInst).outSet;
        } else {
            instDeps.inSet = bbDataDepsMap_[inst.getParent()].inSet;
        }

        llvm::SmallBitVector inSetCopy = instDeps.inSet;
        instDeps.outSet = instDeps.genSet | inSetCopy.reset(instDeps.killSet);
        if (instDeps.outSet != prevOutSet) {
            changesHappened_ = true;
        }
    }

    return newPrevInst;
}

void CatInOutProcessor::print() {
    for (llvm::Instruction* callInst : *mappedInstructions_) {
        printInOutSets(llvm::errs(), callInst, dataDepsMap_->at(callInst), *mappedInstructions_);
    }
}
