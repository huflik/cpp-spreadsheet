#pragma once

#include "common.h"
#include "formula.h"

#include <unordered_set>
#include <optional>

class Sheet;
class Cell;

  class Impl {
    public:
      virtual ~Impl() {}
      virtual CellInterface::Value GetValue() = 0;
      virtual std::string GetText() const = 0;
      virtual std::vector<Position> GetReferencedCells() const { return {};}
      virtual void InvalidateCache() {}
  };

  class EmptyImpl : public Impl {
    public:
      EmptyImpl() {}
      std::string GetText() const override {return {};}
      CellInterface::Value GetValue() override {return 0.0;}
  };

  class TextImpl : public Impl {
    public:
      explicit TextImpl(const std::string& text);
      CellInterface::Value GetValue()  override;
      std::string GetText() const override;
    private:
      std::string text_;
  };

  class FormulaImpl : public Impl {
    public:
      explicit FormulaImpl(const SheetInterface& sheet,const std::string& expression );
      CellInterface::Value GetValue()  override;
      std::string GetText() const override;
      void InvalidateCache() override;
      std::vector<Position> GetReferencedCells() const override;
    private:
      std::unique_ptr<FormulaInterface> formula_ptr_;
      const SheetInterface& sheet_;
      mutable std::optional<FormulaInterface::Value> cache_;
  };

class Cell : public CellInterface {
  public:
    explicit Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
  private:
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    // ячейки, на результат которых влияет данная
    std::unordered_set<Cell*> dst_cells_;
	// ячейки, влияющие на результат данной
    std::unordered_set<Cell*> src_cells_;
    
    void CheckCircularDependency(const std::vector<Position> &src_cells, const Cell* cell) const;
    void InvalidateCache();
};
