#pragma once
#include <QDialog>

class QLineEdit;
class QPushButton;

class ChangePasswordDialog : public QDialog {
  Q_OBJECT
public:
  explicit ChangePasswordDialog(QWidget* parent = nullptr);

private slots:
  void onSave();

private:
  void updateButtonState();

  QLineEdit* oldPass_ = nullptr;
  QLineEdit* newPass_ = nullptr;
  QLineEdit* newPass2_ = nullptr;
  QPushButton* saveBtn_ = nullptr;
};
