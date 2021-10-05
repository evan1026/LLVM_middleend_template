#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"

/**
 * Output operator for a list of call instructions.
 *
 * @param os The output stream to output to.
 * @param callInsts The list of call instructions.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::vector<llvm::CallInst*>& callInsts);

/**
 * Output operator for a map of values to call instructions that modify them.
 *
 * @param os The output stream to output to.
 * @param callInsts The value-call instruction map.
 * @return os
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::map<llvm::Value*, std::vector<llvm::CallInst*>>& map);
