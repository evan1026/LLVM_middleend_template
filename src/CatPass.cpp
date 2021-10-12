#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"

#include "CatCallInstVisitor.hpp"
#include "CatGenKillVisitor.hpp"
#include "CatInOutVisitor.hpp"
#include "DataStructureOutput.hpp"

using namespace llvm;

namespace {
  struct CAT : public FunctionPass {
    static char ID;

    CatCallInstVisitor callInstVisitor;
    CatGenKillVisitor genKillVisitor;
    CatInOutVisitor inOutVisitor;

    CAT() : FunctionPass(ID), callInstVisitor(), genKillVisitor(), inOutVisitor() {}

    // This function is invoked once at the initialization phase of the compiler
    // The LLVM IR of functions isn't ready at this point
    bool doInitialization (Module &M) override {
      return false;
    }

    // This function is invoked once per function compiled
    // The LLVM IR of the input functions is ready and it can be analyzed and/or transformed
    bool runOnFunction (Function &F) override {
      errs() << "Function \"" << F.getName() << "\" \n";
      callInstVisitor.visit(F);

      genKillVisitor.setCallInstructions(callInstVisitor.getCallInstructions());
      genKillVisitor.setValueModifications(callInstVisitor.getValueModifications());
      genKillVisitor.visit(F);

      auto genKillMap = genKillVisitor.getGenKillMap(); // Make a copy bc we want to modify it a lot
      inOutVisitor.setDataDepsMap(genKillMap);
      inOutVisitor.setCallInstructions(callInstVisitor.getCallInstructions());
      inOutVisitor.setValueModifications(callInstVisitor.getValueModifications());
      do {
        inOutVisitor.visit(F);
      } while (inOutVisitor.changesHappened());

      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    // The LLVM IR of functions isn't ready at this point
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Homework for the CAT class");

// Next there is code to register your pass to "clang"
static CAT * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT()); }}); // ** for -O0
