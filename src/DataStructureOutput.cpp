#include "DataStructureOutput.hpp"

static int indent = 0;

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::vector<llvm::CallInst*>& callInsts) {
  os << "[\n";
  ++indent;
  for(llvm::CallInst* callInst : callInsts) {
    for (int i = 0; i < indent; ++i) {
      os << "    ";
    }
    os << *callInst << ",\n";
  }
  --indent;
  for (int i = 0; i < indent; ++i) {
    os << "    ";
  }
  os << "]\n";
  return os;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const std::unordered_set<llvm::CallInst*>& callInsts) {
  os << "[\n";
  ++indent;
  for(llvm::CallInst* callInst : callInsts) {
    for (int i = 0; i < indent; ++i) {
      os << "    ";
    }
    os << *callInst << ",\n";
  }
  --indent;
  for (int i = 0; i < indent; ++i) {
    os << "    ";
  }
  os << "]\n";
  return os;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const MAP_TYPE& map) {
  os << "{\n";
  ++indent;

  for (auto it = map.begin(); it != map.end(); ++it) {
    for (int i = 0; i < indent; ++i) {
      os << "    ";
    }
    os << *it->first << ": " << it->second << "\n";
  }

  --indent;
  for (int i = 0; i < indent; ++i) {
    os << "    ";
  }
  os << "}\n";
  return os;
}
