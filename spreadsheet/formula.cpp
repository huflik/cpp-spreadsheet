#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const { return category_; }

bool FormulaError::operator==(FormulaError rhs) const { return category_ == rhs.category_; }

std::string_view FormulaError::ToString() const {
	switch (category_) {
		case Category::Ref:
		return "#REF!"sv;
		case Category::Value:
		return "#VALUE!"sv;
		case Category::Div0:
		return "#DIV/0!"sv;
	}
	return "";
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
	class Formula : public FormulaInterface {
	public:
		explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}

		catch (const std::exception& e) {
			std::throw_with_nested(FormulaException(e.what()));
		}

		Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            }
            catch (FormulaError& e) {
                return e;
            }
		}

		std::vector<Position> GetReferencedCells() const override {
			std::vector<Position> cells;
			for (auto cell : ast_.GetCells()) {
			if (cell.IsValid()) {
				cells.push_back(cell);
			}
			}
			cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
			return cells;
		}

		std::string GetExpression() const override {
			std::ostringstream result;
			ast_.PrintFormula(result);
			return result.str();
		}
	private:
		const FormulaAST ast_;
	};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	try {
		return std::make_unique<Formula>(std::move(expression));
	}
	catch (...) {
		throw FormulaException("");
	}
}
