#pragma once
#include <QString>
#include <QStringList>

namespace AdminDbPaths {
  QStringList allCsvPaths();                 // 5 csv files
  QString pathForKey(const QString& key);    // "MATERIAL" -> "data/material.csv"
}
