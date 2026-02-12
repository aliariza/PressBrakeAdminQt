#include "CsvTableModel.hpp"

#include <QRegularExpression>
#include <QLocale>


CsvTableModel::CsvTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int CsvTableModel::rowCount(const QModelIndex&) const { return rows_.size(); }
int CsvTableModel::columnCount(const QModelIndex&) const { return headers_.size(); }

QVariant CsvTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return {};
  if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
  const int r = index.row(), c = index.column();
  if (r < 0 || r >= rows_.size()) return {};
  if (c < 0 || c >= headers_.size()) return {};
  const auto& row = rows_[r];
  if (c >= row.size()) return {};
  return row[c];
}

bool CsvTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) return false;
  if (role != Qt::EditRole && role != Qt::DisplayRole) return false;

  const int r = index.row();
  const int c = index.column();
  if (r < 0 || r >= rows_.size()) return false;
  if (c < 0 || c >= headers_.size()) return false;

  QString text = value.toString();

  // ---- Numeric validation (only for selected columns)
  if (isNumericColumn(c)) {
    QString trimmed = text.trimmed();

    // allow clearing the cell
    if (!trimmed.isEmpty()) {
      double num = 0.0;
      if (!parseNumber(trimmed, num)) {
        // reject invalid numeric input
        return false;
      }

      // normalize: store with '.' decimal, remove leading/trailing spaces
      // (optional) keep as user typed; but normalization helps consistency:
      trimmed.replace(',', '.');
      text = trimmed;
    } else {
      text.clear();
    }
  }

  // Resize row to match headers (safety)
  if (rows_[r].size() != headers_.size()) rows_[r].resize(headers_.size());

  if (rows_[r][c] == text) return true;

  rows_[r][c] = text;
  emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
  return true;
}


Qt::ItemFlags CsvTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) return Qt::NoItemFlags;
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant CsvTableModel::headerData(int section, Qt::Orientation o, int role) const {
  if (role != Qt::DisplayRole) return {};
  if (o == Qt::Horizontal) {
    if (section >= 0 && section < headers_.size()) return headers_[section];
  } else {
    return section + 1;
  }
  return {};
}

void CsvTableModel::setTable(const QStringList& headers, const QVector<QStringList>& rows) {
  beginResetModel();
  headers_ = headers;
  rows_ = rows;
  for (auto& r : rows_) r.resize(headers_.size());
  endResetModel();
}

void CsvTableModel::clear() {
  beginResetModel();
  headers_.clear();
  rows_.clear();
  endResetModel();
}

void CsvTableModel::addColumn(const QString& name) {
  const int newCol = headers_.size();
  beginInsertColumns(QModelIndex(), newCol, newCol);
  headers_.push_back(name);
  for (auto& r : rows_) r.push_back("");
  endInsertColumns();
}

void CsvTableModel::deleteColumn(int col) {
  if (col < 0 || col >= headers_.size()) return;
  beginRemoveColumns(QModelIndex(), col, col);
  headers_.removeAt(col);
  for (auto& r : rows_) {
    if (col < r.size()) r.removeAt(col);
    r.resize(headers_.size());
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

bool CsvTableModel::isNumericColumn(int col) const {
  if (col < 0 || col >= headers_.size()) return false;
  const QString h = headers_[col].trimmed().toLower();

  // ✅ Customize this list as you like
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

  // exact match
  if (keys.contains(h)) return true;

  // common variants like "min_ton", "max-ton", "minTon"
  QString simplified = h;
  simplified.remove('_');
  simplified.remove('-');
  simplified.remove(' ');
  if (keys.contains(simplified)) return true;

  // also accept headers ending with known units, e.g. "maxTon", "minTon"
  // (already covered by simplified list, but safe)
  if (simplified.endsWith("ton") || simplified.endsWith("tons")) return true;

  return false;
}

bool CsvTableModel::parseNumber(QString s, double& out) {
  s = s.trimmed();
  if (s.isEmpty()) return false;

  // Normalize decimal comma to dot
  s.replace(',', '.');

  // Allow: -12, 12, 12.5, .5, 0.5
  static const QRegularExpression re(R"(^[+-]?(\d+(\.\d*)?|\.\d+)$)");
  if (!re.match(s).hasMatch()) return false;

  bool ok = false;
  out = s.toDouble(&ok);
  return ok;
}
