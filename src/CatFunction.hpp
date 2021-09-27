#pragma once

#include <unordered_map>
#include <string>

class CatFunction;

using CAT_MAP = std::unordered_map<std::string, CatFunction>;

class CatFunction {

    std::string name_;
    bool isInitialAssignment_;
    bool isModification_;

    CatFunction(std::string name, bool isInitialAssignment, bool isModification);

    static CAT_MAP GET_CAT_FUNCTIONS();
    static CAT_MAP CAT_FUNCTIONS;

    public:

        static const CatFunction* get(const std::string& name);;

        const std::string& getName() const;
        bool isInitialAssignment() const;
        bool isModification() const;

};
