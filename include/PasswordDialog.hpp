// include/PasswordDialog.hpp

#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;
class QWidget;
class QLabel;

class PasswordDialog : public QDialog {
  Q_OBJECT
public:
  explicit PasswordDialog(QWidget* parent = nullptr);
  QString password() const;

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void onChangePasswordClicked();
  void onPasswordEdited(const QString&);

private:
  bool isCurrentPasswordCorrect(const QString& typed) const;
  void updateCapsWarning();

  QLineEdit* edit_ = nullptr;
  QCheckBox* show_ = nullptr;

  QWidget* capsRow_ = nullptr;
  QLabel* capsIcon_ = nullptr;
  QLabel* capsText_ = nullptr;
  QPushButton* changeBtn_ = nullptr;
};
