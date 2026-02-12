#include "ChangePasswordDialog.hpp"
#include "BackupUtils.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>

ChangePasswordDialog::ChangePasswordDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Change Admin Password");
  setModal(true);

  auto* v = new QVBoxLayout(this);
  auto* form = new QFormLayout();

  oldPass_ = new QLineEdit(this);
  oldPass_->setEchoMode(QLineEdit::Password);
  oldPass_->setPlaceholderText("Current password");

  newPass_ = new QLineEdit(this);
  newPass_->setEchoMode(QLineEdit::Password);
  newPass_->setPlaceholderText("New password");

  newPass2_ = new QLineEdit(this);
  newPass2_->setEchoMode(QLineEdit::Password);
  newPass2_->setPlaceholderText("Repeat new password");

  form->addRow("Current:", oldPass_);
  form->addRow("New:", newPass_);
  form->addRow("Confirm:", newPass2_);
  v->addLayout(form);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
  saveBtn_ = new QPushButton("Save", this);
  buttons->addButton(saveBtn_, QDialogButtonBox::AcceptRole);
  v->addWidget(buttons);

  saveBtn_->setEnabled(false);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(saveBtn_, &QPushButton::clicked, this, &ChangePasswordDialog::onSave);

  connect(oldPass_,  &QLineEdit::textChanged, this, [this]{ updateButtonState(); });
  connect(newPass_,  &QLineEdit::textChanged, this, [this]{ updateButtonState(); });
  connect(newPass2_, &QLineEdit::textChanged, this, [this]{ updateButtonState(); });

  oldPass_->setFocus();
}

void ChangePasswordDialog::updateButtonState() {
  const bool ok = !oldPass_->text().trimmed().isEmpty()
               && !newPass_->text().trimmed().isEmpty()
               && (newPass_->text() == newPass2_->text());
  saveBtn_->setEnabled(ok);
}

void ChangePasswordDialog::onSave() {
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

  QMessageBox::information(this, "Success", "Admin password updated.");
  accept();
}
