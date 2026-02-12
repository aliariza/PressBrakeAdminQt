#include "MainWindow.hpp"
#include "AdminTab.hpp"
#include <QTabWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  auto* tabs = new QTabWidget(this);

  adminTab_ = new AdminTab(tabs);
  tabs->addTab(adminTab_, "ADMIN");

  setCentralWidget(tabs);
  setWindowTitle("Press Brake - ADMIN");
  resize(1100, 700);
}
