#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <variant>
#include <vector>
#include <optional>
#include <set>
#include <stack>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet, Position my_pos);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:

    class Impl{
    public:
        virtual ~Impl() = default;

        virtual Value GetValue() const  = 0;

        virtual std::string GetText() const  = 0;

        virtual std::vector<Position> GetReferencedCells() const = 0;

        virtual void DischargeCash() {
            return;
        }
    };

    class EmptyImpl : public Impl{
    public:
        Value GetValue() const override{
            return std::move("");
        }

        std::string GetText() const override{
            return "";
        }

        std::vector<Position> GetReferencedCells() const override {
            return std::vector<Position>();
        }
    };

    class FormulaImpl :public Impl {
     public:
        FormulaImpl(std::string expression, SheetInterface& s)
            :formula_(ParseFormula(std::move(expression))), sheet_impl_(s){
        }

        Value GetValue() const override{

            if (cash_) {
                return *cash_;
            }

            child_impl_.clear();
            const auto result = formula_->Evaluate(sheet_impl_);

            Value value;
            if(auto* ptr = std::get_if<double>(&result)){
                value = *ptr;
                cash_ = *ptr;
            }else if(auto* ptr = std::get_if<FormulaError>(&result)){
                value = *ptr;
                cash_ = *ptr;
            }     

            return value;
        }

        std::string GetText() const override{
           auto result =formula_->GetExpression();
            return "=" + result;
        }

        std::vector<Position> GetReferencedCells() const override { 
            return formula_.get()->GetReferencedCells();
        }

        void DischargeCash() {
            cash_ = std::nullopt;
        }
        
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& sheet_impl_;
        mutable std::optional<CellInterface::Value> cash_;
        mutable std::vector<Position> child_impl_;
    };


    class TextImpl : public Impl{
    public:
        TextImpl(std::string expression)
            :expression_(std::move(expression))
        {}

        Value GetValue() const override{
           if(expression_[0] == ESCAPE_SIGN){
               return expression_.substr(1,expression_.npos);
           }
            return std::move(expression_);
        }

        std::string GetText() const override{
            return std::move(expression_);
        }

        std::vector<Position> GetReferencedCells() const override {
            return std::vector<Position>();
        }

    private:
        std::string expression_;
    };


    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    Position my_pos_;
    std::set<Position> child_;
    std::set<Position> parent_;

    void CheckLoop(const std::vector<Position>& leaves, const CellInterface* root);
    void UpdateSetParentForChild(std::set<Position>&& new_child);
    void InvalidateCash();
};
