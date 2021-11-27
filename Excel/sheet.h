#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <deque>


class Sheet : public SheetInterface {
public:

    using Line = std::unordered_map<int, std::unique_ptr<Cell>>;
    using Column = std::unordered_map<int, Line>;

    ~Sheet();

    void SetCell(const Position& pos, const std::string& text) override;

    const CellInterface* GetCell(const Position& pos) const override;
    CellInterface* GetCell(const Position& pos) override;

    void ClearCell(const Position& pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void ValidCell(const Position& pos) const;
    bool CheckCell(const Position& pos) const;

    std::string GetCellValueStr(const Cell::Value& value) const;

private:

    std::multiset<int> cols_ = {0};
    std::multiset<int> rows_ = {0};
    Column column_;

    void ChechSizeUp(const Position& pos);
    void ChechSizeDown(const Position& pos);  
};