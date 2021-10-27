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
      catSetFunc = M.getFunction("CAT_set");
      return false;
    }

    bool isInInSet(llvm::SmallBitVector& inSet, llvm::Instruction* inst, std::vector<llvm::Instruction*>& insts) {
      for (std::size_t i = 0; i < insts.size(); ++i) {
        if (insts[i] == inst && inSet.test(i)) {
          return true;
        }
      }
      return false;
    }

    void populateDataDepsMap(std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap, const std::vector<llvm::Instruction*> instructions) {
      for (auto it = dataDepsMap.begin(); it != dataDepsMap.end(); ++it) {
        it->second.generateInstructionSets(instructions);
      }
    }

    // This function is invoked once per function compiled
    // The LLVM IR of the input functions is ready and it can be analyzed and/or transformed
    bool runOnFunction (llvm::Function &F) override {
      CatInstructionVisitor instVisitor;
      CatGenKillVisitor genKillVisitor;
      CatInOutProcessor inOutProcessor;
      CatConstantPropagationProcessor constPropProcessor;
      CatConstantFoldingProcessor constFoldProcessor;

      llvm::errs() << "Function \"" << F.getName() << "\" \n";
      instVisitor.visit(F);

      std::vector<llvm::Instruction*>& instructions = instVisitor.getMappedInstructions();
      genKillVisitor.setMappedInstructions(instructions);
      genKillVisitor.setValueModifications(instVisitor.getValueModifications());
      genKillVisitor.visit(F);
      llvm::errs() << "Gen/Kill sets complete\n";

      auto dataDepsMap = genKillVisitor.getGenKillMap(); // Make a copy bc we want to modify it a lot
      inOutProcessor.setDataDepsMap(dataDepsMap);
      inOutProcessor.setMappedInstructions(instructions);
      inOutProcessor.process(F);
      llvm::errs() << "In/Out sets complete\n";

      populateDataDepsMap(dataDepsMap, instructions);

      //inOutProcessor.print();
      //genKillVisitor.print();

      constPropProcessor.calculate(instructions, dataDepsMap);
      constFoldProcessor.calculate(instructions, dataDepsMap);

      bool modification = false;
      modification |= constPropProcessor.execute(catSetFunc);
      modification |= constFoldProcessor.execute(catSetFunc);

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
