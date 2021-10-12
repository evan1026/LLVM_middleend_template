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

void printBitVector(llvm::raw_ostream& os, const llvm::SmallBitVector& bitVector, const std::vector<llvm::CallInst*>& callInstructions) {
    llvm::SmallBitVector resizedBitVector = bitVector;
    resizedBitVector.resize(callInstructions.size());
    for (size_t i = 0; i < callInstructions.size(); ++i) {
        if (resizedBitVector.test(i)) {
            os << " " << *callInstructions[i] << "\n";
        }
    }
}

static void printHeader(llvm::raw_ostream& os, const std::string& name) {
    os << "***************** " << name << "\n{\n";
}

static void printFooter(llvm::raw_ostream& os) {
    os << "}\n**************************************\n";
}

void printGenKillSets(llvm::raw_ostream& os, const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::CallInst*>& callInstructions) {
    os << "INSTRUCTION: " << *callInst << "\n";
    printHeader(os, "GEN");
    printBitVector(os, dataDeps.genSet, callInstructions);
    printFooter(os);
    printHeader(os, "KILL");
    printBitVector(os, dataDeps.killSet, callInstructions);
    printFooter(os);
    os << "\n\n\n";
}

void printInOutSets(llvm::raw_ostream& os, const llvm::Instruction* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::CallInst*>& callInstructions) {
    os << "INSTRUCTION: " << *callInst << "\n";
    printHeader(os, "IN");
    printBitVector(os, dataDeps.inSet, callInstructions);
    printFooter(os);
    printHeader(os, "OUT");
    printBitVector(os, dataDeps.outSet, callInstructions);
    printFooter(os);
    os << "\n\n\n";
}
