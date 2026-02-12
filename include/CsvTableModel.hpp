#pragma once

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>
#include <QSet>

class CsvTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit CsvTableModel(QObject* parent = nullptr);

  // QAbstractTableModel
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  // Table content
  void clear();
  void setTable(const QStringList& headers, const QVector<QStringList>& rows);

  QStringList headers() const { return headers_; }
  QVector<QStringList> rows() const { return rows_; }

  // Columns
  void addColumn(const QString& name);                 // keeps old behavior (text by default)
  void addColumn(const QString& name, bool isNumeric); // NEW: explicit type
  void deleteColumn(int col);

  // Rows
  void addRow();
  void deleteRow(int row);

  // Column type schema (stored as lowercase header keys)
  void setNumericColumns(const QSet<QString>& colsLower);
  QSet<QString> numericColumns() const;

private:
  QStringList headers_;
  QVector<QStringList> rows_;

  // explicit numeric columns (lowercase header keys)
  QSet<QString> numericColsLower_;

  bool isNumericColumn(int col) const;
  static bool parseNumber(QString s, double& out);
};
