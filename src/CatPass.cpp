#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"

#include "CatInstructionVisitor.hpp"
#include "CatGenKillVisitor.hpp"
#include "CatInOutProcessor.hpp"
#include "DataStructureOutput.hpp"

namespace {
  struct CAT : public llvm::FunctionPass {
    static char ID;


    CAT() : FunctionPass(ID) {}

    // This function is invoked once at the initialization phase of the compiler
    // The LLVM IR of functions isn't ready at this point
    bool doInitialization (llvm::Module &M) override {
      return false;
    }

    void doConstantPropagation(CatInstructionVisitor& instVisitor, std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
      std::map<llvm::CallInst*, llvm::Value*> replacements;
      for (auto& inst : instVisitor.getMappedInstructions()) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        if (callInst) {
          const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
          if (func) {
            if (!func->isModification()) {
              // At this point we know we're looking at a CAT instruction and it is a candidate for constant propagation
              auto catVar = callInst->getArgOperand(0);
              auto dataDeps = dataDepsMap.at(callInst);

              llvm::errs() << "Analysing: " << *callInst << "\n";
              int index = dataDeps.inSet.find_first();
              while (index != -1) {
                llvm::Instruction* value = instVisitor.getMappedInstructions()[index];
                llvm::CallInst* callValue = llvm::cast<llvm::CallInst>(value);
                llvm::errs() << "    Checking value of: " << *value << "\n";
                  const CatFunction* func2 = CatFunction::get(callValue->getCalledFunction()->getName().str());
                  if (func2) {
                    if (func2->isInitialAssignment()) {
                      if (callValue == catVar) {
                        llvm::errs() << "        This instruction made the operand and is a constant (CAT_new)!\n";
                        replacements.insert({callInst, callValue->getArgOperand(0)});
                      }
                    } else if (func2->isModification() && !func2->isCalculation()) {
                      if (callValue->getArgOperand(0) == catVar) {
                        llvm::errs() << "        This instruction made the operand and is a constant (CAT_set)!\n";
                        replacements.insert({callInst, callValue->getArgOperand(1)});
                      }
                    }
                  }
                index = dataDeps.inSet.find_next(index);
              }
            }
          }
        }
      }

      for (auto it = replacements.begin(); it != replacements.end(); ++it) {
        llvm::errs() << "We will replace \"" << *it->first << "\" with \"" << *it->second << "\"\n";
        it->first->replaceAllUsesWith(it->second);
        it->first->eraseFromParent();
      }
    }

    // This function is invoked once per function compiled
    // The LLVM IR of the input functions is ready and it can be analyzed and/or transformed
    bool runOnFunction (llvm::Function &F) override {
      CatInstructionVisitor instVisitor;
      CatGenKillVisitor genKillVisitor;
      CatInOutProcessor inOutProcessor;

      llvm::errs() << "Function \"" << F.getName() << "\" \n";
      instVisitor.visit(F);

      genKillVisitor.setMappedInstructions(instVisitor.getMappedInstructions());
      genKillVisitor.setValueModifications(instVisitor.getValueModifications());
      genKillVisitor.visit(F);

      auto dataDepsMap = genKillVisitor.getGenKillMap(); // Make a copy bc we want to modify it a lot
      inOutProcessor.setDataDepsMap(dataDepsMap);
      inOutProcessor.setMappedInstructions(instVisitor.getMappedInstructions());
      inOutProcessor.process(F);

      //inOutProcessor.print();
      //genKillVisitor.print();

      doConstantPropagation(instVisitor, dataDepsMap);

      return false;
    }

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    }
  };
}

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static llvm::RegisterPass<CAT> X("CAT", "Homework for the CAT class");

// Next there is code to register your pass to "clang"
static CAT * _PassMaker = NULL;
static llvm::RegisterStandardPasses _RegPass1(llvm::PassManagerBuilder::EP_OptimizerLast,
    [](const llvm::PassManagerBuilder&, llvm::legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT());}}); // ** for -Ox
static llvm::RegisterStandardPasses _RegPass2(llvm::PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const llvm::PassManagerBuilder&, llvm::legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT()); }}); // ** for -O0
