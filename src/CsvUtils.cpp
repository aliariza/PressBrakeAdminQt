#include "CsvUtils.hpp"
#include <QTextStream>

namespace CsvUtils {

QStringList parseCsvRecord(const QString& record) {
  QStringList fields;
  QString cur;
  bool inQuotes = false;

  for (int i = 0; i < record.size(); ++i) {
    const QChar ch = record[i];

    if (inQuotes) {
      if (ch == '"') {
        if (i + 1 < record.size() && record[i + 1] == '"') {
          cur += '"';
          ++i;
        } else {
          inQuotes = false;
        }
      } else {
        cur += ch;
      }
    } else {
      if (ch == '"') {
        inQuotes = true;
      } else if (ch == ',') {
        fields.push_back(cur);
        cur.clear();
      } else if (ch == '\r') {
        // ignore
      } else if (ch == '\n') {
        break;
      } else {
        cur += ch;
      }
    }
  }
  fields.push_back(cur);
  return fields;
}

static QString encodeCsvField(const QString& field) {
  bool mustQuote = field.contains(',') || field.contains('"') || field.contains('\n') || field.contains('\r');
  QString out = field;
  out.replace("\"", "\"\"");
  if (mustQuote) out = "\"" + out + "\"";
  return out;
}

QString encodeCsvRecord(const QStringList& fields) {
  QStringList encoded;
  encoded.reserve(fields.size());
  for (const auto& f : fields) encoded.push_back(encodeCsvField(f));
  return encoded.join(',');
}

QString readCsvRecord(QTextStream& in) {
  if (in.atEnd()) return QString();

  QString record;
  bool inQuotes = false;

  while (true) {
    QString line = in.readLine();
    if (line.isNull()) break;

    if (!record.isEmpty()) record += "\n";
    record += line;

    for (int i = 0; i < line.size(); ++i) {
      if (line[i] == '"') {
        if (i + 1 < line.size() && line[i + 1] == '"') {
          ++i;
        } else {
          inQuotes = !inQuotes;
        }
      }
    }

    if (!inQuotes) break;
    if (in.atEnd()) break;
  }

  return record;
}

} // namespace CsvUtils
