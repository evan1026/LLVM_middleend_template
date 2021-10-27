#include "CatDataDependencyProcessor.hpp"


static void populateDataDepsMap(std::map<llvm::Instruction*, CatDataDependencies>& dataDepsMap, const std::vector<llvm::Instruction*> instructions) {
    for (auto it = dataDepsMap.begin(); it != dataDepsMap.end(); ++it) {
        it->second.generateInstructionSets(instructions);
    }
}

const std::map<llvm::Instruction*, CatDataDependencies> CatDataDependencyProcessor::getDataDependencies(llvm::Function& func, CatInstructionVisitor& instVisitor) {
    CatGenKillVisitor genKillVisitor;
    CatInOutProcessor inOutProcessor;

    std::vector<llvm::Instruction*>& instructions = instVisitor.getMappedInstructions();
    genKillVisitor.setMappedInstructions(instructions);
    genKillVisitor.setValueModifications(instVisitor.getValueModifications());
    genKillVisitor.visit(func);
    llvm::errs() << "Gen/Kill sets complete\n";

    auto dataDepsMap = genKillVisitor.getGenKillMap(); // Make a copy bc we want to modify it a lot
    inOutProcessor.setDataDepsMap(dataDepsMap);
    inOutProcessor.setMappedInstructions(instructions);
    inOutProcessor.process(func);
    llvm::errs() << "In/Out sets complete\n";

    populateDataDepsMap(dataDepsMap, instructions);

    return dataDepsMap;
}
