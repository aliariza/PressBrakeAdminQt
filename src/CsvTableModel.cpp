#include "CsvTableModel.hpp"

#include <QRegularExpression>

CsvTableModel::CsvTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int CsvTableModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  return rows_.size();
}

int CsvTableModel::columnCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  return headers_.size();
}

QVariant CsvTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return {};
  if (role != Qt::DisplayRole && role != Qt::EditRole) return {};

  const int r = index.row();
  const int c = index.column();
  if (r < 0 || r >= rows_.size()) return {};
  if (c < 0 || c >= headers_.size()) return {};

  const auto& row = rows_[r];
  if (c >= row.size()) return {};
  return row[c];
}

QVariant CsvTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) return {};
  if (orientation == Qt::Horizontal) {
    if (section >= 0 && section < headers_.size()) return headers_[section];
    return {};
  }
  // Vertical header: 1-based row numbers
  return section + 1;
}

Qt::ItemFlags CsvTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) return Qt::NoItemFlags;
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void CsvTableModel::clear() {
  beginResetModel();
  headers_.clear();
  rows_.clear();
  numericColsLower_.clear();
  endResetModel();
}

void CsvTableModel::setTable(const QStringList& headers, const QVector<QStringList>& rows) {
  beginResetModel();
  headers_ = headers;
  rows_ = rows;

  // Ensure all rows are sized to header count
  for (auto& r : rows_) r.resize(headers_.size());

  endResetModel();
}

void CsvTableModel::addColumn(const QString& name) {
  const int newCol = headers_.size();
  beginInsertColumns(QModelIndex(), newCol, newCol);
  headers_.push_back(name);
  for (auto& r : rows_) r.push_back("");
  endInsertColumns();
}

void CsvTableModel::addColumn(const QString& name, bool isNumeric) {
  addColumn(name);

  const QString key = name.trimmed().toLower();
  if (isNumeric) numericColsLower_.insert(key);
  else numericColsLower_.remove(key);
}

void CsvTableModel::deleteColumn(int col) {
  if (col < 0 || col >= headers_.size()) return;

  const QString key = headers_[col].trimmed().toLower();
  numericColsLower_.remove(key);

  beginRemoveColumns(QModelIndex(), col, col);
  headers_.removeAt(col);
  for (auto& r : rows_) {
    if (col >= 0 && col < r.size()) r.removeAt(col);
  }
  endRemoveColumns();
}

void CsvTableModel::addRow() {
  const int r = rows_.size();
  beginInsertRows(QModelIndex(), r, r);
  QStringList row;
  row.resize(headers_.size());
  for (int i = 0; i < row.size(); ++i) row[i] = "";
  rows_.push_back(row);
  endInsertRows();
}

void CsvTableModel::deleteRow(int row) {
  if (row < 0 || row >= rows_.size()) return;
  beginRemoveRows(QModelIndex(), row, row);
  rows_.removeAt(row);
  endRemoveRows();
}

void CsvTableModel::setNumericColumns(const QSet<QString>& colsLower) {
  numericColsLower_.clear();
  for (const auto& s : colsLower) numericColsLower_.insert(s.trimmed().toLower());
}

QSet<QString> CsvTableModel::numericColumns() const {
  return numericColsLower_;
}

bool CsvTableModel::isNumericColumn(int col) const {
  if (col < 0 || col >= headers_.size()) return false;

  // 1) Explicit schema wins
  const QString key = headers_[col].trimmed().toLower();
  if (numericColsLower_.contains(key)) return true;

  // 2) Fallback heuristic (for older CSVs without schema)
  QString h = key;
  QString simplified = h;
  simplified.remove('_');
  simplified.remove('-');
  simplified.remove(' ');

  static const QStringList keys = {
    "minton", "maxton", "ton", "tons",
    "length", "width", "height",
    "thickness", "radius",
    "kg", "weight",
    "power", "kw",
    "v", "volt", "voltage",
    "a", "amp", "current",
    "price", "cost"
  };

  if (keys.contains(h) || keys.contains(simplified)) return true;
  if (simplified.endsWith("ton") || simplified.endsWith("tons")) return true;

  return false;
}

bool CsvTableModel::parseNumber(QString s, double& out) {
  s = s.trimmed();
  if (s.isEmpty()) return false;

  // accept decimal comma, normalize to dot
  s.replace(',', '.');

  // Allow: -12, 12, 12.5, 12., .5
  static const QRegularExpression re(R"(^[+-]?(\d+(\.\d*)?|\.\d+)$)");
  if (!re.match(s).hasMatch()) return false;

  bool ok = false;
  out = s.toDouble(&ok);
  return ok;
}

bool CsvTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) return false;
  if (role != Qt::EditRole && role != Qt::DisplayRole) return false;

  const int r = index.row();
  const int c = index.column();
  if (r < 0 || r >= rows_.size()) return false;
  if (c < 0 || c >= headers_.size()) return false;

  QString text = value.toString();

  // Numeric validation for numeric columns
  if (isNumericColumn(c)) {
    QString trimmed = text.trimmed();

    // allow clearing
    if (!trimmed.isEmpty()) {
      double num = 0.0;
      if (!parseNumber(trimmed, num)) return false;

      // normalize decimal comma to dot
      trimmed.replace(',', '.');
      text = trimmed;
    } else {
      text.clear();
    }
  }

  // Ensure row size matches headers
  if (rows_[r].size() != headers_.size()) rows_[r].resize(headers_.size());

  if (rows_[r][c] == text) return true;

  rows_[r][c] = text;
  emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
  return true;
}
