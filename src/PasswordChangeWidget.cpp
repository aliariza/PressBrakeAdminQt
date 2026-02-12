// src/PasswordChangeWidget.cpp

#include "PasswordChangeWidget.hpp"

#include "BackupUtils.hpp"

#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStringConverter>

PasswordChangeWidget::PasswordChangeWidget(QWidget* parent) : QWidget(parent) {
  auto* v = new QVBoxLayout(this);

  box_ = new QGroupBox("Change Admin Password", this);
  auto* form = new QFormLayout(box_);

  oldPass_ = new QLineEdit(box_);
  oldPass_->setEchoMode(QLineEdit::Password);
  oldPass_->setPlaceholderText("Current password");

  newPass_ = new QLineEdit(box_);
  newPass_->setEchoMode(QLineEdit::Password);
  newPass_->setPlaceholderText("New password");

  newPass2_ = new QLineEdit(box_);
  newPass2_->setEchoMode(QLineEdit::Password);
  newPass2_->setPlaceholderText("Repeat new password");

  changeBtn_ = new QPushButton("Save New Password", box_);
  changeBtn_->setEnabled(false);

  form->addRow("Current:", oldPass_);
  form->addRow("New:", newPass_);
  form->addRow("Confirm:", newPass2_);
  form->addRow("", changeBtn_);

  v->addWidget(box_);

  connect(changeBtn_, &QPushButton::clicked, this, &PasswordChangeWidget::onChangePassword);

  connect(oldPass_,  &QLineEdit::textChanged, this, [this](const QString&) { updateButtonState(); });
  connect(newPass_,  &QLineEdit::textChanged, this, [this](const QString&) { updateButtonState(); });
  connect(newPass2_, &QLineEdit::textChanged, this, [this](const QString&) { updateButtonState(); });
}

void PasswordChangeWidget::updateButtonState() {
  const bool ok =
      !oldPass_->text().trimmed().isEmpty() &&
      !newPass_->text().trimmed().isEmpty() &&
      (newPass_->text() == newPass2_->text());
  changeBtn_->setEnabled(ok);
}

void PasswordChangeWidget::onChangePassword() {
  const QString passPath = "data/admin.pass";

  QFile rf(passPath);
  if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Error", "Cannot read: " + passPath);
    return;
  }
  const QString current = QString::fromUtf8(rf.readLine()).trimmed();
  rf.close();

  const QString oldP  = oldPass_->text();
  const QString newP  = newPass_->text();
  const QString newP2 = newPass2_->text();

  if (current.isEmpty()) {
    QMessageBox::critical(this, "Error", "admin.pass is empty or unreadable.");
    return;
  }
  if (oldP != current) {
    QMessageBox::warning(this, "Wrong password", "Current password is incorrect.");
    return;
  }
  if (newP.trimmed().isEmpty()) {
    QMessageBox::warning(this, "Invalid", "New password cannot be empty.");
    return;
  }
  if (newP != newP2) {
    QMessageBox::warning(this, "Mismatch", "New password and confirmation do not match.");
    return;
  }
  if (newP == current) {
    QMessageBox::information(this, "No change", "New password is the same as the current password.");
    return;
  }

  BackupUtils::makeTimestampedBackupKeepN(passPath, this, 10);

  QFile wf(passPath);
  if (!wf.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    QMessageBox::critical(this, "Error", "Cannot write: " + passPath);
    return;
  }
  QTextStream out(&wf);
  out.setEncoding(QStringConverter::Utf8);
  out << newP << "\n";
  wf.close();

  oldPass_->clear();
  newPass_->clear();
  newPass2_->clear();
  updateButtonState();

  QMessageBox::information(this, "Success", "Admin password updated.");
}
