#include "CatFunction.hpp"


CAT_MAP CatFunction::GET_CAT_FUNCTIONS() {
    CAT_MAP catFunctions;
    catFunctions.emplace(std::make_pair("CAT_new", CatFunction{"CAT_new", true,  true}));
    catFunctions.emplace(std::make_pair("CAT_add", CatFunction{"CAT_add", false, true}));
    catFunctions.emplace(std::make_pair("CAT_sub", CatFunction{"CAT_sub", false, true}));
    catFunctions.emplace(std::make_pair("CAT_get", CatFunction{"CAT_get", false, false}));
    catFunctions.emplace(std::make_pair("CAT_set", CatFunction{"CAT_set", false, true}));
    return catFunctions;
}

CAT_MAP CatFunction::CAT_FUNCTIONS = CatFunction::GET_CAT_FUNCTIONS();

CatFunction::CatFunction(std::string name, bool isInitialAssignment, bool isModification) :
    name_(name),
    isInitialAssignment_(isInitialAssignment),
    isModification_(isModification)
{}

const CatFunction* CatFunction::get(const std::string& name) {
    if (CAT_FUNCTIONS.find(name) != CAT_FUNCTIONS.end()) {
        return &CAT_FUNCTIONS.at(name);
    } else {
        return nullptr;
    }
}

const std::string& CatFunction::getName() const {
    return name_;
}

bool CatFunction::isInitialAssignment() const {
    return isInitialAssignment_;
}

bool CatFunction::isModification() const {
    return isModification_;
}
