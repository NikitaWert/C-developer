#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>


// Реализуйте следующие методы

Cell::Cell(Sheet& sheet, Position my_pos) 
    :impl_(std::make_unique<EmptyImpl>()), sheet_(sheet), my_pos_(my_pos) {
}

Cell::~Cell() {}

void Cell::InvalidateCash() {
    
    impl_.get()->DischargeCash();

    for (const auto& pos : parent_) {
        dynamic_cast<Cell*>(sheet_.GetCell(pos))->InvalidateCash();
    }
}

void Cell::UpdateSetParentForChild(std::set<Position>&& new_child){

    for (const auto& pos : child_) {
        dynamic_cast<Cell*>(sheet_.GetCell(pos))->parent_.erase(my_pos_);
    }
    child_.clear();
    child_ = std::move(new_child);

    for (auto& item : child_) {
        dynamic_cast<Cell*>(sheet_.GetCell(item))->parent_.insert(my_pos_);
    }
}

void Cell::Set(std::string text) {

    if (text.empty()) {
        UpdateSetParentForChild(std::set<Position>());
        InvalidateCash();
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if (text[0] == '=' && text.size() > 1) {
        std::unique_ptr<Impl> impl;
        try {
            impl = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        }
        catch (std::exception&) {
            throw FormulaException("Error");
        }

        auto vec_child = impl.get()->GetReferencedCells();

        CheckLoop(vec_child, this);
        UpdateSetParentForChild(std::set<Position>(vec_child.begin(), vec_child.end()));
        InvalidateCash();
        impl_.swap(impl);        
    } else {
        UpdateSetParentForChild(std::set<Position>());
        InvalidateCash();
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

    void Cell::Clear() {
        impl_ = std::make_unique<EmptyImpl>();
    }

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_.get()->GetReferencedCells();
}

void Cell::CheckLoop(const std::vector<Position>& leaves, const CellInterface* root) {

    for (const Position& leaflet : leaves) {
        const auto fd = sheet_.GetCell(leaflet);
        if (!fd) {
            sheet_.SetCell(leaflet, "");
        }
        if (fd == root) {
            throw CircularDependencyException("Error");
        }

        CheckLoop(sheet_.GetCell(leaflet)->GetReferencedCells(), root);
    }
}

bool Cell::IsReferenced() const {
    return !parent_.empty();
}
