#include "AdminTab.hpp"
#include "DbEditorWidget.hpp"
#include "PasswordChangeWidget.hpp"

#include <QVBoxLayout>

AdminTab::AdminTab(QWidget* parent) : QWidget(parent) {
  auto* v = new QVBoxLayout(this);
  auto* db   = new DbEditorWidget(this);

  v->addWidget(db, 1); // stretch: table takes remaining space
}
