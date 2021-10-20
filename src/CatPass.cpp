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

    llvm::Function* catSetFunc;

    // This function is invoked once at the initialization phase of the compiler
    // The LLVM IR of functions isn't ready at this point
    bool doInitialization (llvm::Module &M) override {
      catSetFunc = M.getFunction("CAT_set");
      return false;
    }

    //TODO create function to check if value in IN set

    std::map<llvm::CallInst*, llvm::Value*> doConstantPropagation(CatInstructionVisitor& instVisitor, std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
      llvm::errs() << "Doing constant propagation\n";
      std::map<llvm::CallInst*, llvm::Value*> replacements;
      for (auto& inst : instVisitor.getMappedInstructions()) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        bool phiFound = false;
        if (callInst) {
          const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
          if (func) {
            if (!func->isModification()) {
              // At this point we know we're looking at a CAT instruction and it is a candidate for constant propagation
              auto catVar = callInst->getArgOperand(0);
              auto dataDeps = dataDepsMap.at(callInst);

              llvm::errs() << "Analysing for constant propagation: " << *callInst << "\n";
              int index = dataDeps.inSet.find_first();
              while (index != -1) {
                llvm::Instruction* value = instVisitor.getMappedInstructions()[index];
                llvm::CallInst* callValue = llvm::dyn_cast<llvm::CallInst>(value);
                llvm::Value* replaceValue = nullptr;
                if (callValue) {
                  llvm::errs() << "    Checking value of: " << *value << "\n";
                  const CatFunction* func2 = CatFunction::get(callValue->getCalledFunction()->getName().str());
                  if (func2) {
                    // TODO bug if multiple constants are possible
                    if (func2->isInitialAssignment()) {
                      if (callValue == catVar) {
                        llvm::errs() << "        This instruction made the operand and is a constant (CAT_new)!\n";
                        replaceValue = callValue->getArgOperand(0);
                      }
                    } else if (func2->isModification() && !func2->isCalculation()) {
                      if (callValue->getArgOperand(0) == catVar) {
                        llvm::errs() << "        This instruction made the operand and is a constant (CAT_set)!\n";
                        replaceValue = callValue->getArgOperand(1);
                      }
                    }
                  }
                }

                llvm::PHINode* phi = llvm::dyn_cast<llvm::PHINode>(value);
                if (phi && phi == catVar) {
                  llvm::errs() << "PHI node exists for this value which means it could be undefined. We won't propagate.\n";
                  phiFound = true;
                }

                if (replaceValue != nullptr) {
                  replacements.insert({callInst, replaceValue});
                }
                index = dataDeps.inSet.find_next(index);
              }

              if (phiFound) {
                auto replacementLoc = replacements.find(callInst);
                if (replacementLoc != replacements.end()) {
                  replacements.erase(replacementLoc);
                }
              }
            }
          }
        }
      }
      return replacements;
    }

    bool isInInSet(llvm::SmallBitVector& inSet, llvm::Instruction* inst, std::vector<llvm::Instruction*>& insts) {
      for (std::size_t i = 0; i < insts.size(); ++i) {
        if (insts[i] == inst && inSet.test(i)) {
          return true;
        }
      }
      return false;
    }

    struct ArgPair {
      llvm::Value* arg1;
      llvm::Value* arg2;
    };

    std::map<llvm::CallInst*, ArgPair> doConstantFolding(CatInstructionVisitor& instVisitor, std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap) {
      llvm::errs() << "Doing constant folding\n";
      std::map<llvm::CallInst*, ArgPair> replacements;
      for (auto& inst : instVisitor.getMappedInstructions()) {
        llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
        bool phiFound = false;
        if (callInst) {
          const CatFunction* func = CatFunction::get(callInst->getCalledFunction()->getName().str());
          if (func) {
            if (func->isCalculation()) {
              // At this point we know we're looking at a CAT instruction and it is a candidate for constant folding
              auto dataDeps = dataDepsMap.at(callInst);

              llvm::errs() << "Analysing for constant folding: " << *callInst << "\n";

              llvm::Value* arg1Const = nullptr;
              llvm::Value* arg2Const = nullptr;

              int index = dataDeps.inSet.find_first();
              while (index != -1) {
                llvm::Instruction* value = instVisitor.getMappedInstructions()[index];
                llvm::CallInst* callValue = llvm::dyn_cast<llvm::CallInst>(value);
                if (callValue) {
                  llvm::errs() << "    Checking value of: " << *value << "\n";
                  const CatFunction* func2 = CatFunction::get(callValue->getCalledFunction()->getName().str());
                  if (func2) {
                    if (func2->isInitialAssignment()) {
                      if (callValue == callInst->getArgOperand(1)) {
                        arg1Const = callValue->getArgOperand(0);;
                        llvm::errs() << "        This instruction made operand 1 and is a constant (CAT_new)!\n";
                      }
                      if (callValue == callInst->getArgOperand(2)) {
                        arg2Const = callValue->getArgOperand(0);
                        llvm::errs() << "        This instruction made operand 2 and is a constant (CAT_new)!\n";
                      }
                    } else if (func2->isModification() && !func2->isCalculation()) {
                      if (callValue->getArgOperand(0) == callInst->getArgOperand(1)) {
                        arg1Const = callValue->getArgOperand(1);
                        llvm::errs() << "        This instruction made operand 1 and is a constant (CAT_set)!\n";
                      }
                      if (callValue->getArgOperand(0) == callInst->getArgOperand(2)) {
                        arg2Const = callValue->getArgOperand(1);
                        llvm::errs() << "        This instruction made operand 2 and is a constant (CAT_set)!\n";
                      }
                    }
                  }
                }

                llvm::PHINode* phiNode = llvm::dyn_cast<llvm::PHINode>(value);
                if (phiNode) {
                  if (phiNode == callInst->getArgOperand(1) || phiNode == callInst->getArgOperand(2)) {
                    llvm::errs() << "PHI node exists for this value which means it could be undefined. We won't propagate.\n";
                    phiFound = true;
                  }
                }

                index = dataDeps.inSet.find_next(index);
              }

              if (arg1Const != nullptr && arg2Const != nullptr && !phiFound) {
                llvm::errs() << "    This is a constant expression!\n";
                ArgPair pair{arg1Const, arg2Const};
                replacements.insert({callInst, pair});
              }
            }
          }
        }
      }
      return replacements;
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
      llvm::errs() << "Gen/Kill sets complete\n";

      auto dataDepsMap = genKillVisitor.getGenKillMap(); // Make a copy bc we want to modify it a lot
      inOutProcessor.setDataDepsMap(dataDepsMap);
      inOutProcessor.setMappedInstructions(instVisitor.getMappedInstructions());
      inOutProcessor.process(F);
      llvm::errs() << "In/Out sets complete\n";

      //inOutProcessor.print();
      //genKillVisitor.print();

      auto constantProps = doConstantPropagation(instVisitor, dataDepsMap);
      auto constantFolds = doConstantFolding(instVisitor, dataDepsMap);

      bool modification = false;

      for (auto it = constantProps.begin(); it != constantProps.end(); ++it) {
        llvm::errs() << "We will replace \"" << *it->first << "\" with \"" << *it->second << "\"\n";
        it->first->replaceAllUsesWith(it->second);
        it->first->eraseFromParent();
        modification = true;
      }

      for (auto it = constantFolds.begin(); it != constantFolds.end(); ++it) {
        llvm::errs() << "Generating const value for \"" << *it->first << "\" using \"" << *it->second.arg1 << "\" and \"" << *it->second.arg2 << "\"\n";
        const CatFunction* calcFunc = CatFunction::get(it->first->getCalledFunction()->getName().str());
        llvm::Value* result = calcFunc->applyOperation(it->second.arg1, it->second.arg2);
        if (result) {
          llvm::errs() << "    New val is " << *result << "\n";
          llvm::CallInst* newCatSet = llvm::CallInst::Create(catSetFunc, std::vector<llvm::Value*>(std::initializer_list<llvm::Value*>{it->first->getArgOperand(0), result}));
          newCatSet->insertBefore(it->first);
          it->first->eraseFromParent();
          llvm::errs() << "    New cat set: " << *newCatSet << "\n";
          modification = true;
        } else {
          llvm::errs() << "    Somthing went wrong with folding (probably the values weren't actually constants). Continuing...\n";
        }
      }

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
