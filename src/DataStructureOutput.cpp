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

void printBitVector(llvm::raw_ostream& os, const llvm::SmallBitVector& bitVector, const std::vector<llvm::Value*>& instructions) {
    llvm::SmallBitVector resizedBitVector = bitVector;
    resizedBitVector.resize(instructions.size());
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (resizedBitVector.test(i)) {
            os << " " << *instructions[i] << "\n";
        }
    }
}

static void printHeader(llvm::raw_ostream& os, const std::string& name) {
    os << "***************** " << name << "\n{\n";
}

static void printFooter(llvm::raw_ostream& os) {
    os << "}\n**************************************\n";
}

void printGenKillSets(llvm::raw_ostream& os, const llvm::Value* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::Value*>& instructions) {
    os << "INSTRUCTION: " << *callInst << "\n";
    printHeader(os, "GEN");
    printBitVector(os, dataDeps.genSet, instructions);
    printFooter(os);
    printHeader(os, "KILL");
    printBitVector(os, dataDeps.killSet, instructions);
    printFooter(os);
    os << "\n\n\n";
}

void printInOutSets(llvm::raw_ostream& os, const llvm::Value* callInst, const CatDataDependencies& dataDeps, const std::vector<llvm::Value*>& instructions) {
    os << "INSTRUCTION: " << *callInst << "\n";
    printHeader(os, "IN");
    printBitVector(os, dataDeps.inSet, instructions);
    printFooter(os);
    printHeader(os, "OUT");
    printBitVector(os, dataDeps.outSet, instructions);
    printFooter(os);
    os << "\n\n\n";
}
