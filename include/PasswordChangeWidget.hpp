#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;
class QGroupBox;

class PasswordChangeWidget : public QWidget {
  Q_OBJECT
public:
  explicit PasswordChangeWidget(QWidget* parent = nullptr);

private slots:
  void onChangePassword();

private:
  void updateButtonState();

  QGroupBox* box_ = nullptr;
  QLineEdit* oldPass_ = nullptr;
  QLineEdit* newPass_ = nullptr;
  QLineEdit* newPass2_ = nullptr;
  QPushButton* changeBtn_ = nullptr;
};
