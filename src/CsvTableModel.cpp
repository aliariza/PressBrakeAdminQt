#include "CsvTableModel.hpp"

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
  if (!index.isValid() || role != Qt::EditRole) return false;
  const int r = index.row(), c = index.column();
  if (r < 0 || r >= rows_.size() || c < 0 || c >= headers_.size()) return false;

  auto& row = rows_[r];
  if (row.size() < headers_.size()) row.resize(headers_.size());
  row[c] = value.toString();
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