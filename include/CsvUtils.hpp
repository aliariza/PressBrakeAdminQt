#pragma once
#include <QString>
#include <QStringList>

class QTextStream;

namespace CsvUtils {
  QString readCsvRecord(QTextStream& in);        // supports embedded newlines in quotes
  QStringList parseCsvRecord(const QString& rec);
  QString encodeCsvRecord(const QStringList& fields);
}
