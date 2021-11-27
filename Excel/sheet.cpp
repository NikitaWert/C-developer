#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <variant>
#include <optional>
#include <set>
#include <sstream>
#include <cmath>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::ChechSizeUp(const Position& pos) {
    cols_.insert(pos.col + 1);
    rows_.insert(pos.row + 1);
}

void Sheet::ChechSizeDown(const Position& pos) {
    cols_.erase(cols_.find(pos.col + 1));
    rows_.erase(rows_.find(pos.row + 1));
}

void Sheet::SetCell(const Position& pos, const std::string& text) {

    ValidCell(pos);

    if (!column_.count(pos.col) || !column_.at(pos.col).count(pos.row)) {
        column_[pos.col][pos.row] = std::make_unique<Cell>(*this, pos);
    }

    column_[pos.col][pos.row].get()->Set(text);

    ChechSizeUp(pos);
}

bool Sheet::CheckCell(const Position& pos) const {
    return column_.count(pos.col) && column_.at(pos.col).count(pos.row);
}

void Sheet::ValidCell(const Position& pos) const {
    if (pos.row < 0 || pos.col < 0 || pos.row >= Position::MAX_ROWS || pos.col >= Position::MAX_COLS) {
        throw InvalidPositionException("");
    }
}

const CellInterface* Sheet::GetCell(const Position& pos) const {

    ValidCell(pos);

    if (CheckCell(pos)) {
        return column_.at(pos.col).at(pos.row).get();
    }

    return nullptr;
}
CellInterface* Sheet::GetCell(const Position& pos) {

    ValidCell(pos);

    if (CheckCell(pos)) {
        return column_.at(pos.col).at(pos.row).get();
    }

    return nullptr;
}

void Sheet::ClearCell(const Position& pos) {

    ValidCell(pos);

     if (CheckCell(pos)) {
         column_.at(pos.col).erase(pos.row);
         ChechSizeDown(pos);
     }
}

Size Sheet::GetPrintableSize() const {
    return Size{*rows_.rbegin(), *cols_.rbegin()};
}

std::string Sheet::GetCellValueStr(const Cell::Value& value) const {
   
    if (std::holds_alternative<double>(value)) {
        return std::to_string(static_cast<int>(std::round(std::get<double>(value))));
    }
    else if (std::holds_alternative<FormulaError>(value)) {
        std::stringstream ss;
        ss << std::get<FormulaError>(value);
        return ss.str();
    }
    else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return ""s;
}

void Sheet::PrintValues(std::ostream& output) const {

    for (int i = 0; i < *rows_.rbegin(); ++i) {
        for (int j = 0; j < *cols_.rbegin(); ++j) {
            if (column_.count(j) && column_.at(j).count(i)) {
                output << GetCellValueStr(column_.at(j).at(i).get()->GetValue());
            }
            if (j < *cols_.rbegin() - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {

    for (int i = 0; i < *rows_.rbegin(); ++i) {
        for (int j = 0; j < *cols_.rbegin(); ++j) {
            if (column_.count(j) && column_.at(j).count(i)) {
                output << column_.at(j).at(i).get()->GetText();
            }
            if (j < *cols_.rbegin() - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}