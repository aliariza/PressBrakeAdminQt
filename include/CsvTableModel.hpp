#pragma once
#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>

class CsvTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit CsvTableModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  void setTable(const QStringList& headers, const QVector<QStringList>& rows);
  void clear();

  void addColumn(const QString& name);
  void deleteColumn(int col);

  void addRow();
  void deleteRow(int row);

  QStringList headers() const { return headers_; }
  QVector<QStringList> rows() const { return rows_; }

private:
  QStringList headers_;
  QVector<QStringList> rows_;
};
