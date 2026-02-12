#pragma once
#include <QMainWindow>

class AdminTab;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
private:
  AdminTab* adminTab_ = nullptr;
};
