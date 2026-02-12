#pragma once
#include <QString>

class QWidget;

namespace BackupUtils {
  bool makeTimestampedBackupKeepN(const QString& path, QWidget* parent, int keepN);
}
