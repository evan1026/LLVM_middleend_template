#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"

#include "CatCallInstVisitor.hpp"
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

    // This function is invoked once per function compiled
    // The LLVM IR of the input functions is ready and it can be analyzed and/or transformed
    bool runOnFunction (llvm::Function &F) override {
      CatCallInstVisitor callInstVisitor;
      CatGenKillVisitor genKillVisitor;
      CatInOutProcessor inOutProcessor;

      llvm::errs() << "Function \"" << F.getName() << "\" \n";
      callInstVisitor.visit(F);

      genKillVisitor.setMappedInstructions(callInstVisitor.getMappedInstructions());
      genKillVisitor.setValueModifications(callInstVisitor.getValueModifications());
      genKillVisitor.visit(F);

      auto dataDepsMap = genKillVisitor.getGenKillMap(); // Make a copy bc we want to modify it a lot
      inOutProcessor.setDataDepsMap(dataDepsMap);
      inOutProcessor.setMappedInstructions(callInstVisitor.getMappedInstructions());

      do {
        inOutProcessor.processOnce(F);
      } while (inOutProcessor.changesHappened());

      inOutProcessor.print();
      //genKillVisitor.print();

      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    // The LLVM IR of functions isn't ready at this point
    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
      AU.setPreservesAll();
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
