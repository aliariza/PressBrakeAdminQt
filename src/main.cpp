#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include "MainWindow.hpp"
#include "PasswordDialog.hpp"

static QString readAdminPass(const QString& path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
  QString line = QString::fromUtf8(f.readLine()).trimmed();
  return line;
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  const QString passPath = "data/admin.pass";
  const QString expected = readAdminPass(passPath);
  if (expected.isEmpty()) {
    QMessageBox::critical(nullptr, "Error", "Cannot read data/admin.pass");
    return 1;
  }

  PasswordDialog dlg;
  if (dlg.exec() != QDialog::Accepted) return 0;

  if (dlg.password() != expected) {
    QMessageBox::critical(nullptr, "Access denied", "Wrong password.");
    return 1;
  }

  MainWindow w;
  w.show();
  return app.exec();
}
