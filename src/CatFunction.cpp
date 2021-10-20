#include "CatFunction.hpp"


///////////////////////////
// Static Initialization //
///////////////////////////
CAT_MAP CatFunction::GET_CAT_FUNCTIONS() {
    CAT_MAP catFunctions;
    catFunctions.emplace(std::make_pair("CAT_new", CatFunction{"CAT_new", true,  true,  false}));
    catFunctions.emplace(std::make_pair("CAT_add", CatFunction{"CAT_add", false, true,  true}));
    catFunctions.emplace(std::make_pair("CAT_sub", CatFunction{"CAT_sub", false, true,  true}));
    catFunctions.emplace(std::make_pair("CAT_get", CatFunction{"CAT_get", false, false, false}));
    catFunctions.emplace(std::make_pair("CAT_set", CatFunction{"CAT_set", false, true,  false}));
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
