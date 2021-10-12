#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"

#include <vector>
#include <unordered_set>
#include <map>

#include "CatDataDependencies.hpp"

using MAP_TYPE = std::map<llvm::Value*, std::unordered_set<llvm::CallInst*>>;

/**
 * Output operator for a list of call instructions.
 *
 * @param os The output stream to output to.
 * @param callInsts The list of call instructions.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::vector<llvm::CallInst*>& callInsts);

/**
 * Output operator for a set of call instructions.
 *
 * @param os The output stream to output to.
 * @param callInsts The set of call instructions.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::unordered_set<llvm::CallInst*>& callInsts);

/**
 * Output operator for a map of values to call instructions that modify them.
 *
 * @param os The output stream to output to.
 * @param callInsts The value-call instruction map.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const MAP_TYPE& map);

/**
 * Prints a single bit vector assuming the indeces correspond to the instructions
 * within callInstructions.
 *
 * @param os Output stream to output to
 * @param bitVector Bit vector to print
 * @param callInstructions List of call instructions that the bit vector corresponds to
 */
void printBitVector(llvm::raw_ostream& os, const llvm::SmallBitVector& bitVector, const std::vector<llvm::CallInst*>& callInstructions);

void printGenKillSets(llvm::raw_ostream& os, const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::CallInst*>& callInstructions);
void printInOutSets(llvm::raw_ostream& os, const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::CallInst*>& callInstructions);
