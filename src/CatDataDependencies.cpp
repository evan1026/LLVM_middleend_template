#include "CatDataDependencies.hpp"

void CatDataDependencies::generateInstructionSets(const std::vector<llvm::Value*> instructions) {
    for (std::size_t i = 0; i < instructions.size(); ++i) {
        if (genSet.size() > i && genSet.test(i)) {
            instGenSet.insert(instructions.at(i));
        }
        if (killSet.size() > i && killSet.test(i)) {
            instKillSet.insert(instructions.at(i));
        }
        if (inSet.size() > i && inSet.test(i)) {
            instInSet.insert(instructions.at(i));
        }
        if (outSet.size() > i && outSet.test(i)) {
            instOutSet.insert(instructions.at(i));
        }
    }
}
