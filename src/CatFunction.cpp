#include "CatFunction.hpp"


///////////////////////////
// Static Initialization //
///////////////////////////
CAT_MAP CatFunction::GET_CAT_FUNCTIONS() {
    CAT_MAP catFunctions;
    catFunctions.emplace(std::make_pair("CAT_new", CatFunction{CatFunc::CAT_new, "CAT_new", true,  true,  false}));
    catFunctions.emplace(std::make_pair("CAT_add", CatFunction{CatFunc::CAT_add, "CAT_add", false, true,  true}));
    catFunctions.emplace(std::make_pair("CAT_sub", CatFunction{CatFunc::CAT_sub, "CAT_sub", false, true,  true}));
    catFunctions.emplace(std::make_pair("CAT_get", CatFunction{CatFunc::CAT_get, "CAT_get", false, false, false}));
    catFunctions.emplace(std::make_pair("CAT_set", CatFunction{CatFunc::CAT_set, "CAT_set", false, true,  false}));
    return catFunctions;
}

CAT_MAP CatFunction::CAT_FUNCTIONS = CatFunction::GET_CAT_FUNCTIONS();


/////////////////
// Actual Code //
/////////////////

const CatFunction* CatFunction::get(const std::string& name) {
    if (CAT_FUNCTIONS.find(name) != CAT_FUNCTIONS.end()) {
        return &CAT_FUNCTIONS.at(name);
    } else {
        return nullptr;
    }
}

std::ostream& operator<<(std::ostream& os, const CatFunction& catFunc) {
    os << "CatFunc[name=\"" << catFunc.name_ << "\", isInitialAssignment=" << catFunc.isInitialAssignment_
        << ", isModification=" << catFunc.isModification_ << ", isCalculation=" << catFunc.isCalculation_ << "]";
    return os;
}

static int64_t getConstantValue(llvm::Value* val) {
    llvm::ConstantInt* constInt = llvm::cast<llvm::ConstantInt>(val);
    return constInt->getSExtValue();
}

llvm::Value* CatFunction::applyOperation(llvm::Value* val1, llvm::Value* val2) const {
    int64_t intVal1 = getConstantValue(val1);
    int64_t intVal2 = getConstantValue(val2);
    int64_t res;

    switch (func_) {
        case CatFunc::CAT_sub:
            res = intVal1 - intVal2;
            break;
        case CatFunc::CAT_add:
            res = intVal1 + intVal2;
            break;
        default:
            return nullptr;
            break;
    }

    llvm::Value* newConst = llvm::ConstantInt::get(val1->getType(), res);

    return newConst;


}
