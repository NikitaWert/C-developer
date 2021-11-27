#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <memory>
#include <vector>
#include <functional>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    // Реализуйте следующие методы:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(std::move(expression))){
        
    }
    Value Evaluate(const SheetInterface& sheet) const override {

        std::function<double(Position)> fun = [&sheet](Position pos) {


            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }
            const CellInterface* cell = sheet.GetCell(pos);
            CellInterface::Value value = cell->GetValue();

            if (const auto it = std::get_if<double>(&value)) {
                return *it;
            }
            else if (const auto it = std::get_if<std::string>(&value)) {
                double  res = 0;

                try {
                    res = std::stod(*it);
                }
                catch (...) {
                    throw FormulaError(FormulaError::Category::Value);
                }
                return  res;
            }
            else {
                throw std::get<FormulaError>(value);
            }
        };
        Value result;
        try {
            result = ast_.Execute(fun);
        }
        catch (FormulaError& exep) {
            result = exep;
        }
        return  result;
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        std::string result = out.str();

        return result;
    }

    std::vector<Position> GetReferencedCells() const override { 
        return std::move(std::vector<Position>(ast_.GetCells().begin(), ast_.GetCells().end()));
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));

    }  catch (FormulaException& expt) {
        throw expt;
    }
}
