#include "AdminDbPaths.hpp"

namespace AdminDbPaths {

QStringList allCsvPaths() {
  return {
    "data/material.csv",
    "data/machine.csv",
    "data/machines.csv",
    "data/tooling.csv",
    "data/options.csv"
  };
}

QString pathForKey(const QString& key) {
  if (key == "MATERIAL") return "data/material.csv";
  if (key == "MACHINE")  return "data/machine.csv";
  if (key == "MACHINES") return "data/machines.csv";
  if (key == "TOOLING")  return "data/tooling.csv";
  if (key == "OPTIONS")  return "data/options.csv";
  return {};
}

} // namespace AdminDbPaths
