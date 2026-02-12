#include "BackupUtils.hpp"

#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>

namespace {

void cleanupOldBackups(const QString& path, int keepN) {
  if (keepN <= 0) return;

  QFileInfo fi(path);
  QDir dir(fi.absolutePath());
  const QString baseName = fi.fileName(); // e.g. material.csv

  const QString pattern = baseName + ".*.bak";
  QStringList backups = dir.entryList({pattern}, QDir::Files, QDir::Name);

  const int extra = backups.size() - keepN;
  for (int i = 0; i < extra; ++i) {
    QFile::remove(dir.absoluteFilePath(backups[i]));
  }
}

} // namespace

namespace BackupUtils {

bool makeTimestampedBackupKeepN(const QString& path, QWidget* parent, int keepN) {
  QFileInfo fi(path);
  if (!fi.exists() || !fi.isFile()) return true;

  const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
  const QString bakPath = path + "." + ts + ".bak";

  if (!QFile::copy(path, bakPath)) {
    QMessageBox::warning(parent, "Backup failed", "Cannot create backup:\n" + bakPath);
    return false;
  }

  cleanupOldBackups(path, keepN);
  return true;
}

} // namespace BackupUtils
