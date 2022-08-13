#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	std::unique_ptr<Impl> impl;
	if (text.empty()) {
		impl = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
		impl = std::make_unique<FormulaImpl>(sheet_,std::move(text));
		CheckCircularDependency(impl->GetReferencedCells(), this);
	}
	else {
		impl = std::make_unique<TextImpl>(std::move(text));
	}
	impl_ = std::move(impl);
	
	for (Cell* cell : dst_cells_) {
		cell->src_cells_.erase(this);
	}
	dst_cells_.clear();
	
	for (const auto& pos : impl_->GetReferencedCells()) {
	Cell* cell = sheet_.GetCellPtr(pos);
	if (!cell) {
		sheet_.SetCell(pos, "");
		cell = sheet_.GetCellPtr(pos);
	}
	dst_cells_.insert(cell);
		cell->src_cells_.insert(this);
	}
	
	InvalidateCache();
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
	return impl_->GetReferencedCells();
}

void Cell::InvalidateCache() {
	impl_->InvalidateCache();
	for (Cell* cell : src_cells_) {
		cell->InvalidateCache();
	}
}

void Cell::CheckCircularDependency(const std::vector<Position> &src_cells, const Cell* cell) const {
    for (const auto &cell_pos : src_cells) {
        if (auto src_cell = sheet_.GetCellPtr(cell_pos)) {
            if (src_cell == cell) {
                throw CircularDependencyException("Found circular dependency");
            }
            src_cell->CheckCircularDependency(src_cell->GetReferencedCells(), cell);
        }
    }
}

FormulaImpl::FormulaImpl(const SheetInterface& sheet,const std::string& expression) : sheet_(sheet) {
    formula_ptr_ = ParseFormula(expression.substr(1));
}

CellInterface::Value  FormulaImpl::GetValue() {
    if (!cache_) {
      cache_ = formula_ptr_->Evaluate(sheet_);
    }
    auto value = formula_ptr_->Evaluate(sheet_);
    if (std::holds_alternative<double>(value)) {
      return std::get<double>(value);
     }
    return std::get<FormulaError>(value);
  }

std::string FormulaImpl::GetText() const  {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}

void FormulaImpl::InvalidateCache(){
    cache_.reset();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return formula_ptr_->GetReferencedCells();
}

TextImpl::TextImpl(const std::string &text) : text_(std::move(text)) {}

CellInterface::Value  TextImpl::GetValue() {
    if (text_[0] == ESCAPE_SIGN) {
      return text_.substr(1);
    }
    return text_;
}

std::string TextImpl::GetText() const {
    return text_;
}
