#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"

#include "CatInstructionVisitor.hpp"
#include "CatDataDependencyProcessor.hpp"
#include "DataStructureOutput.hpp"
#include "CatConstantPropagationProcessor.hpp"
#include "CatConstantFoldingProcessor.hpp"

namespace {
  struct CAT : public llvm::FunctionPass {
    static char ID;


    CAT() : FunctionPass(ID) {}

    llvm::Function* catSetFunc;

    // This function is invoked once at the initialization phase of the compiler
    // The LLVM IR of functions isn't ready at this point
    bool doInitialization (llvm::Module &M) override {
      CatFunction::init(M);
      return false;
    }


    // This function is invoked once per function compiled
    // The LLVM IR of the input functions is ready and it can be analyzed and/or transformed
    bool runOnFunction (llvm::Function &F) override {
      CatInstructionVisitor instVisitor;
      CatDataDependencyProcessor catDepsProcessor;
      CatConstantPropagationProcessor constPropProcessor;
      CatConstantFoldingProcessor constFoldProcessor;

      llvm::errs() << "Function \"" << F.getName() << "\" \n";
      instVisitor.visit(F);

      std::vector<llvm::Instruction*>& instructions = instVisitor.getMappedInstructions();
      auto dataDepsMap = catDepsProcessor.getDataDependencies(F, instVisitor);

      auto& escapedVariables = instVisitor.getEscapedVars();
      constPropProcessor.calculate(instructions, dataDepsMap, escapedVariables);
      constFoldProcessor.calculate(instructions, dataDepsMap);

      bool modification = false;
      modification |= constPropProcessor.execute();
      modification |= constFoldProcessor.execute();

      return modification;
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
