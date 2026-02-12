#pragma once
#include <QWidget>

class QComboBox;
class QTableView;
class QPushButton;
class CsvTableModel;
class QLineEdit;
class QSortFilterProxyModel;


class DbEditorWidget : public QWidget {
  Q_OBJECT
public:
  explicit DbEditorWidget(QWidget* parent = nullptr);

private slots:
  void onDatabaseChanged(int idx);
  void onLoad();
  void onSave();
  void onSaveAll();
  void onAddRow();
  void onDeleteRow();
  void onAddColumn();
  void onDeleteColumn();

private:
  void loadDb(const QString& path);
  void saveDb(const QString& path);
  void setDirty(bool on);

  QComboBox* dbSelector_ = nullptr;
  QTableView* table_ = nullptr;
  CsvTableModel* model_ = nullptr;

  QPushButton* loadBtn_ = nullptr;
  QPushButton* saveBtn_ = nullptr;
  QPushButton* saveAllBtn_ = nullptr;

  QPushButton* addRowBtn_ = nullptr;
  QPushButton* delRowBtn_ = nullptr;

  QPushButton* addColBtn_ = nullptr;
  QPushButton* delColBtn_ = nullptr;

  QLineEdit* search_ = nullptr;
  QSortFilterProxyModel* proxy_ = nullptr;

  QString currentPath_;
  bool dirty_ = false;
  int lastHeaderCol_ = -1;

  int lastIndex_ = 0;
};
