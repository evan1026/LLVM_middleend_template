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

void CatInOutProcessor::process(llvm::Function& func) {
    do {
        processOnce(func);
    } while (changesHappened());
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
    if (dataDepsMap_->find(&inst) != dataDepsMap_->end()) {
        newPrevInst = &inst;

        CatDataDependencies& instDeps = dataDepsMap_->at(&inst);
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
    for (llvm::Value* inst : *mappedInstructions_) {
        if (llvm::isa<llvm::Instruction>(inst)) {
            llvm::Instruction* instInst = llvm::cast<llvm::Instruction>(inst);
            printInOutSets(llvm::errs(), instInst, dataDepsMap_->at(instInst), *mappedInstructions_);
        }
    }
}
