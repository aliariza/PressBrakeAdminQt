// src/PasswordDialog.cpp

#include "PasswordDialog.hpp"
#include "ChangePasswordDialog.hpp"

#ifdef __APPLE__
extern "C" bool pb_capslock_on();
#endif

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFile>
#include <QStringConverter>
#include <QTextStream>

PasswordDialog::PasswordDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Admin Login");
  setModal(true);

  auto* v = new QVBoxLayout(this);
  v->addWidget(new QLabel("Enter admin password:", this));

  // Password field
  edit_ = new QLineEdit(this);
  edit_->setEchoMode(QLineEdit::Password);
  edit_->setPlaceholderText("Password");
  v->addWidget(edit_);

  // Caps Lock warning row (icon + text, hidden unless Caps Lock is on)
  capsRow_ = new QWidget(this);
  auto* capsLay = new QHBoxLayout(capsRow_);
  capsLay->setContentsMargins(2, -2, 0, 0);
  capsLay->setSpacing(6);

  capsIcon_ = new QLabel(capsRow_);
  const QIcon warnIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
  capsIcon_->setPixmap(warnIcon.pixmap(14, 14));

  capsText_ = new QLabel("Caps Lock is on", capsRow_);
  capsText_->setStyleSheet("color: rgba(0,0,0,0.45); font-size: 12px;");

  capsLay->addWidget(capsIcon_);
  capsLay->addWidget(capsText_);
  capsLay->addStretch();

  capsRow_->setVisible(false);
  v->addWidget(capsRow_);

  // Show password checkbox
  show_ = new QCheckBox("Show password", this);
  v->addWidget(show_);

  // ---- Bottom row: Change password... (left) + Cancel/OK (right)
  auto* bottom = new QHBoxLayout();
  bottom->setContentsMargins(0, 6, 0, 0);

  changeBtn_ = new QPushButton("Change password…", this);
  changeBtn_->setFlat(true);
  changeBtn_->setCursor(Qt::PointingHandCursor);
  changeBtn_->setVisible(false); // ✅ hidden until current password is correct
  bottom->addWidget(changeBtn_);

  bottom->addStretch();

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
  auto* okBtn = buttons->button(QDialogButtonBox::Ok);
  okBtn->setDefault(true);
  okBtn->setAutoDefault(true);
  okBtn->setEnabled(false);

  bottom->addWidget(buttons);
  v->addLayout(bottom);

  // Toggle visibility
  connect(show_, &QCheckBox::toggled, this, [this](bool on) {
    edit_->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
  });

  // OK enabled only when text entered
  connect(edit_, &QLineEdit::textChanged, this, [okBtn](const QString& t) {
    okBtn->setEnabled(!t.trimmed().isEmpty());
  });

  // Change password button appears only when typed password matches current password
  connect(edit_, &QLineEdit::textChanged, this, &PasswordDialog::onPasswordEdited);

  connect(changeBtn_, &QPushButton::clicked, this, &PasswordDialog::onChangePasswordClicked);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  // Pressing Enter triggers OK (not accept() directly)
  connect(edit_, &QLineEdit::returnPressed, okBtn, &QPushButton::click);

  // Track caps lock changes while typing / focusing
  edit_->installEventFilter(this);
  this->installEventFilter(this);

  // Cursor starts in the password box
  edit_->setFocus(Qt::TabFocusReason);

  // Initial caps state + change button state
  updateCapsWarning();
  onPasswordEdited(edit_->text());
}

QString PasswordDialog::password() const {
  return edit_->text();
}

bool PasswordDialog::eventFilter(QObject* obj, QEvent* event) {
  switch (event->type()) {
    case QEvent::FocusIn:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::WindowActivate:
    case QEvent::Show:
      updateCapsWarning();
      break;
    default:
      break;
  }
  return QDialog::eventFilter(obj, event);
}

void PasswordDialog::onChangePasswordClicked() {
  ChangePasswordDialog dlg(this);
  dlg.exec();

  // After changing password, update visibility based on what is currently typed
  onPasswordEdited(edit_->text());
}

void PasswordDialog::updateCapsWarning() {
#ifdef __APPLE__
  const bool capsOn = pb_capslock_on();
  if (capsRow_) capsRow_->setVisible(capsOn);
#else
  if (capsRow_) capsRow_->setVisible(false);
#endif
}

bool PasswordDialog::isCurrentPasswordCorrect(const QString& typed) const {
  const QString passPath = "data/admin.pass";
  QFile rf(passPath);
  if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

  const QString current = QString::fromUtf8(rf.readLine()).trimmed();
  rf.close();

  return !current.isEmpty() && typed == current;
}

void PasswordDialog::onPasswordEdited(const QString& text) {
  if (!changeBtn_) return;
  changeBtn_->setVisible(isCurrentPasswordCorrect(text)); // ✅ show only when correct
}
