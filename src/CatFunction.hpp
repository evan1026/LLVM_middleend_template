#pragma once

#include <unordered_map>
#include <string>
#include <ostream>

class CatFunction;

using CAT_MAP = std::unordered_map<std::string, CatFunction>;

/**
 * Utility class that contains attributes related to CAT API functions.
 *
 * Call CatFunction::get to get the CatFunction object related to a
 * particular API call.
 *
 * Example usage:
 * \code{.cpp}
 * const CatFunction* catNewFunc = CatFunction::get("CAT_new");
 * if (catNewFunc) {
 *     llvm::outs() << "CAT_new info: " << *catNewFunc << "\n";
 * } else {
 *     llvm::errs() << "No function with the name \"CAT_new\" found!\n";
 * }
 * \endcode
 */
class CatFunction {

    std::string name_;
    bool isInitialAssignment_;
    bool isModification_;
    bool isCalculation_;

    /**
     * Constructor. Made private because CAT functions are declared
     * statically at compile time.
     */
    CatFunction(std::string name, bool isInitialAssignment, bool isModification, bool isCalculation) :
        name_(name),
        isInitialAssignment_(isInitialAssignment),
        isModification_(isModification),
        isCalculation_(isCalculation)
    {}

    static CAT_MAP GET_CAT_FUNCTIONS();
    static CAT_MAP CAT_FUNCTIONS;

    public:

        /**
         * Retrieves a CatFunction object by name.
         *
         * @param name The name of the CatFunction to retrieve.
         * @return A CatFunction pointer pointing to the object if it was found, or `nullptr` if not
         */
        static const CatFunction* get(const std::string& name);

        /**
         * Gets the name of the CatFunction.
         *
         * @return The name of the CatFunction
         */
        const std::string& getName() const { return name_; }

        /**
         * Returns whether this CatFunction performs an initial assignment
         * to a CatVariable. An initial assignment means that before the
         * function was called, the CatVariable was unassigned/unallocated,
         * and it got allocated and assigned by this function (e.g. by a call
         * to CAT_new).
         *
         * @return `true` if this CatFunction performs an initial assignment, `false` otherwise.
         */
        bool isInitialAssignment() const { return isInitialAssignment_; }

        /**
         * Returns whether this CatFunction modifies the value of a CAT object.
         * Objects that perform an initial assignment (see isInitialAssignment())
         * are also considered to perform a modification.
         *
         * @return `true` if this CatFunction performs a modification, `false` otherwise.
         */
        bool isModification() const { return isModification_; }

        bool isCalculation() const { return isCalculation_; }

        /**
         * Output operator for CatFunction objects.
         *
         * @param os Output stream
         * @param catFunc CatFunction to output
         * @return `os` with `catFunc` now inserted into it
         */
        friend std::ostream& operator<<(std::ostream& os, const CatFunction& catFunc);

};

