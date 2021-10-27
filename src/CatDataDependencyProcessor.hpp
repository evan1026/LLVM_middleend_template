#pragma once

#include <map>

#include "CatGenKillVisitor.hpp"
#include "CatInOutProcessor.hpp"
#include "CatInstructionVisitor.hpp"

class CatDataDependencyProcessor {

    public:
        const std::map<llvm::Instruction*, CatDataDependencies> getDataDependencies(llvm::Function& func, CatInstructionVisitor& instVisitor);
};
