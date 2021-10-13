#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"

#include <vector>
#include <unordered_set>
#include <map>

#include "CatDataDependencies.hpp"

using MAP_TYPE = std::map<llvm::Value*, std::unordered_set<llvm::Instruction*>>;

/**
 * Output operator for a list of instructions.
 *
 * @param os The output stream to output to.
 * @param insts The list of instructions.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::vector<llvm::Instruction*>& insts);

/**
 * Output operator for a set of instructions.
 *
 * @param os The output stream to output to.
 * @param insts The set of instructions.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::unordered_set<llvm::Instruction*>& insts);

/**
 * Output operator for a map of values to instructions that modify them.
 *
 * @param os The output stream to output to.
 * @param map The value-instruction map.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const MAP_TYPE& map);

/**
 * Prints a single bit vector assuming the indeces correspond to the instructions
 * within instructions.
 *
 * @param os Output stream to output to
 * @param bitVector Bit vector to print
 * @param instructions List of instructions that the bit vector corresponds to
 */
void printBitVector(llvm::raw_ostream& os, const llvm::SmallBitVector& bitVector, const std::vector<llvm::Instruction*>& instructions);

void printGenKillSets(llvm::raw_ostream& os, const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::Instruction*>& instructions);
void printInOutSets(llvm::raw_ostream& os, const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::Instruction*>& instructions);
