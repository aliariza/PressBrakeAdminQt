// src/DbEditorWidget.cpp

#include "DbEditorWidget.hpp"

#include "CsvTableModel.hpp"
#include "CsvUtils.hpp"
#include "BackupUtils.hpp"
#include "AdminDbPaths.hpp"

#include <QComboBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <algorithm>


class RowFilterProxy : public QSortFilterProxyModel {
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override {
    if (filterRegularExpression().pattern().isEmpty()) return true;

    const int cols = sourceModel() ? sourceModel()->columnCount(sourceParent) : 0;
    for (int c = 0; c < cols; ++c) {
      const QModelIndex idx = sourceModel()->index(sourceRow, c, sourceParent);
      const QString text = sourceModel()->data(idx, Qt::DisplayRole).toString();
      if (text.contains(filterRegularExpression())) return true;
    }
    return false;
  }
};

DbEditorWidget::DbEditorWidget(QWidget* parent) : QWidget(parent) {
  auto* v = new QVBoxLayout(this);

  // Top bar
  auto* top = new QHBoxLayout();
  dbSelector_ = new QComboBox(this);

  // Keep these labels consistent with your project
  dbSelector_->addItem("MATERIAL", "data/material.csv");
  dbSelector_->addItem("MACHINE",  "data/machine.csv");
  dbSelector_->addItem("MACHINES", "data/machines.csv");
  dbSelector_->addItem("TOOLING",  "data/tooling.csv");
  dbSelector_->addItem("OPTIONS",  "data/options.csv");
  lastIndex_ = dbSelector_->currentIndex();

  loadBtn_    = new QPushButton("Load", this);
  saveBtn_    = new QPushButton("Save", this);
  saveAllBtn_ = new QPushButton("Save All", this);

  addRowBtn_  = new QPushButton("Add Row", this);
  delRowBtn_ = new QPushButton("Delete Selected Rows", this);

  addColBtn_  = new QPushButton("Add Column", this);
  delColBtn_  = new QPushButton("Delete Column", this);

  top->addWidget(dbSelector_);
  top->addStretch();
  top->addWidget(loadBtn_);
  top->addWidget(saveBtn_);
  top->addWidget(saveAllBtn_);
  top->addWidget(addRowBtn_);
  top->addWidget(delRowBtn_);
  top->addWidget(addColBtn_);
  top->addWidget(delColBtn_);

  v->addLayout(top);
  // Search row
  auto* searchRow = new QHBoxLayout();
  search_ = new QLineEdit(this);
  search_->setPlaceholderText("Search… (filters rows)");
  searchRow->addWidget(search_);
  v->addLayout(searchRow);
  // Table + Model
  model_ = new CsvTableModel(this);
  
  proxy_ = new RowFilterProxy(this);
  proxy_->setSourceModel(model_);
  proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
  
  table_ = new QTableView(this);
  table_->setModel(proxy_);
  table_->setAlternatingRowColors(true);
  table_->setSortingEnabled(false);
  table_->horizontalHeader()->setSectionsClickable(true);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::ExtendedSelection);

  v->addWidget(table_, 1);

  // Track last clicked header column for Delete Column UX
  connect(table_->horizontalHeader(), &QHeaderView::sectionClicked,
          this, [this](int logicalIndex) { lastHeaderCol_ = logicalIndex; });
    // Search box -> filter proxy
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& t){
  // Escape user text so it behaves like plain “contains”, not regex syntax
  QRegularExpression re(QRegularExpression::escape(t),
                        QRegularExpression::CaseInsensitiveOption);
  proxy_->setFilterRegularExpression(re);
});

  // Dirty tracking
  connect(model_, &QAbstractItemModel::dataChanged, this,
          [this](const QModelIndex&, const QModelIndex&, const QList<int>&) { setDirty(true); });
  connect(model_, &QAbstractItemModel::rowsInserted, this,
          [this](const QModelIndex&, int, int) { setDirty(true); });
  connect(model_, &QAbstractItemModel::rowsRemoved, this,
          [this](const QModelIndex&, int, int) { setDirty(true); });
  connect(model_, &QAbstractItemModel::columnsInserted, this,
          [this](const QModelIndex&, int, int) { setDirty(true); });
  connect(model_, &QAbstractItemModel::columnsRemoved, this,
          [this](const QModelIndex&, int, int) { setDirty(true); });

  // UI connections
  connect(dbSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &DbEditorWidget::onDatabaseChanged);

  connect(loadBtn_,    &QPushButton::clicked, this, &DbEditorWidget::onLoad);
  connect(saveBtn_,    &QPushButton::clicked, this, &DbEditorWidget::onSave);
  connect(saveAllBtn_, &QPushButton::clicked, this, &DbEditorWidget::onSaveAll);

  connect(addRowBtn_, &QPushButton::clicked, this, &DbEditorWidget::onAddRow);
  connect(delRowBtn_, &QPushButton::clicked, this, &DbEditorWidget::onDeleteRow);

  connect(addColBtn_, &QPushButton::clicked, this, &DbEditorWidget::onAddColumn);
  connect(delColBtn_, &QPushButton::clicked, this, &DbEditorWidget::onDeleteColumn);

  // Initial
  onDatabaseChanged(dbSelector_->currentIndex());
  onLoad();
}

void DbEditorWidget::setDirty(bool on) {
  dirty_ = on;

  if (auto* w = window()) {
    QString t = w->windowTitle();
    t.remove(" *");
    if (dirty_) t += " *";
    w->setWindowTitle(t);
  }
}

void DbEditorWidget::onDatabaseChanged(int idx) {
  const QString nextPath = dbSelector_->itemData(idx).toString();

  if (dirty_) {
    auto r = QMessageBox::question(
        this,
        "Unsaved changes",
        "You have unsaved changes. Save before switching database?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (r == QMessageBox::Cancel) {
      // revert selection
      dbSelector_->blockSignals(true);
      dbSelector_->setCurrentIndex(lastIndex_);
      dbSelector_->blockSignals(false);
      return;
    }
    if (r == QMessageBox::Yes) {
      saveDb(currentPath_);
      setDirty(false);
    }
    // No = discard changes
  }

  currentPath_ = nextPath;
  lastIndex_ = idx;
  onLoad();
}


void DbEditorWidget::onLoad() {
  loadDb(currentPath_);
  setDirty(false);
}

void DbEditorWidget::onSave() {
  saveDb(currentPath_);
  setDirty(false);
}

void DbEditorWidget::onSaveAll() {
  const QString cur = currentPath_;
  const auto paths = AdminDbPaths::allCsvPaths();

  // Save current view first
  saveDb(cur);

  // Normalize/backup-save others by loading + saving each
  for (const auto& p : paths) {
    if (p == cur) continue;
    loadDb(p);
    saveDb(p);
  }

  // Restore original
  loadDb(cur);
  setDirty(false);

  QMessageBox::information(this, "Save All", "All databases saved (with backups).");
}

void DbEditorWidget::loadDb(const QString& path) {
  QFile f(path);
  if (!f.exists()) {
    model_->clear();
    return;
  }
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Error", "Cannot open: " + path);
    return;
  }

  QTextStream in(&f);
  in.setEncoding(QStringConverter::Utf8);

  const QString headerRec = CsvUtils::readCsvRecord(in);
  if (headerRec.isNull() || headerRec.isEmpty()) {
    model_->clear();
    return;
  }

  QStringList headers = CsvUtils::parseCsvRecord(headerRec);

  QVector<QStringList> rows;
  while (!in.atEnd()) {
    const QString rec = CsvUtils::readCsvRecord(in);
    if (rec.isNull() || rec.trimmed().isEmpty()) continue;

    auto fields = CsvUtils::parseCsvRecord(rec);
    fields.resize(headers.size());
    rows.push_back(fields);
  }

  model_->setTable(headers, rows);

  // UX: ensure something is selected
  if (model_->rowCount() > 0 && model_->columnCount() > 0) {
    table_->setCurrentIndex(model_->index(0, 0));
  }
  if (proxy_->rowCount() > 0 && proxy_->columnCount() > 0) {
    table_->setCurrentIndex(proxy_->index(0, 0));
    }
}

void DbEditorWidget::saveDb(const QString& path) {
  BackupUtils::makeTimestampedBackupKeepN(path, this, 10);

  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Error", "Cannot write: " + path);
    return;
  }

  QTextStream out(&f);
  out.setEncoding(QStringConverter::Utf8);

  out << CsvUtils::encodeCsvRecord(model_->headers()) << "\n";
  for (const auto& row : model_->rows()) {
    out << CsvUtils::encodeCsvRecord(row) << "\n";
  }
}

void DbEditorWidget::onAddRow() {
  model_->addRow();

  // Select last row in source, but might be filtered out; just clear filter first is optional.
  // We'll select it via proxy if visible.
  const int sourceRow = model_->rowCount() - 1;
  if (sourceRow >= 0) {
    QModelIndex srcIdx = model_->index(sourceRow, 0);
    QModelIndex pIdx = proxy_->mapFromSource(srcIdx);
    if (pIdx.isValid())
      table_->selectRow(pIdx.row());
  }
}

void DbEditorWidget::onDeleteRow() {
  if (!table_->selectionModel()) return;

  const QModelIndexList selected = table_->selectionModel()->selectedRows();
  if (selected.isEmpty()) {
    QMessageBox::information(this, "Delete Rows", "Select one or more rows to delete.");
    return;
  }

  const int n = selected.size();
  auto reply = QMessageBox::question(
      this,
      "Delete Rows",
      QString("Delete %1 selected row(s)?").arg(n));

  if (reply != QMessageBox::Yes) return;

  // Map proxy rows -> source rows
  std::vector<int> sourceRows;
  sourceRows.reserve(selected.size());
  for (const auto& pIdx : selected) {
    const int srcRow = proxy_ ? proxy_->mapToSource(pIdx).row() : pIdx.row();
    if (srcRow >= 0) sourceRows.push_back(srcRow);
  }

  // Sort descending so deletions don't shift later indices
  std::sort(sourceRows.begin(), sourceRows.end());
  sourceRows.erase(std::unique(sourceRows.begin(), sourceRows.end()), sourceRows.end());
  std::sort(sourceRows.rbegin(), sourceRows.rend());

  for (int r : sourceRows) model_->deleteRow(r);
}


void DbEditorWidget::onAddColumn() {
  bool ok = false;
  QString name = QInputDialog::getText(this, "Add Column", "Column name:",
                                       QLineEdit::Normal, "", &ok);
  if (!ok || name.trimmed().isEmpty()) return;
  model_->addColumn(name.trimmed());
}

void DbEditorWidget::onDeleteColumn() {
  int col = table_->currentIndex().column();
  if (col < 0) col = lastHeaderCol_;

  if (col < 0) {
    QMessageBox::information(this, "Delete Column",
                             "Click a column header or select a cell in the column you want to delete.");
    return;
  }

  auto reply = QMessageBox::question(
      this,
      "Delete Column",
      "Delete column '" + model_->headers().value(col) + "' ?");

  if (reply != QMessageBox::Yes) return;

  model_->deleteColumn(col);

  if (lastHeaderCol_ == col) lastHeaderCol_ = -1;
  else if (lastHeaderCol_ > col) lastHeaderCol_--;
}
