#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl;   
    class EmptyImpl;  
    class TextImpl;   
    class FormulaImpl;
    
	bool IsCircularDependency(const Impl& impl) const;
    void InvalidateCache();   
    
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
	// ячейки, на результат которых влияет данная
    std::unordered_set<Cell*> dest_cells_;
	// ячейки, влияющие на результат данной
    std::unordered_set<Cell*> sourc_cells_;

};
