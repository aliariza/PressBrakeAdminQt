cmake_minimum_required(VERSION 3.20)
project(PressBrakeAdminQt LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()  # enables AUTOMOC/AUTOUIC/AUTORCC

add_executable(PressBrakeAdminQt
  src/main.cpp
  src/MainWindow.cpp
  src/AdminTab.cpp
  src/DbEditorWidget.cpp
  src/PasswordChangeWidget.cpp
  src/PasswordDialog.cpp
  src/CsvTableModel.cpp
  src/CapsLock_macos.mm
  src/CsvUtils.cpp
  src/BackupUtils.cpp
  src/AdminDbPaths.cpp
  src/ChangePasswordDialog.cpp

  include/MainWindow.hpp
  include/AdminTab.hpp
  include/DbEditorWidget.hpp
  include/PasswordChangeWidget.hpp
  include/PasswordDialog.hpp
  include/CsvTableModel.hpp
  include/CsvUtils.hpp
  include/BackupUtils.hpp
  include/AdminDbPaths.hpp
  include/ChangePasswordDialog.hpp
)




target_include_directories(PressBrakeAdminQt PRIVATE include)
target_link_libraries(PressBrakeAdminQt PRIVATE Qt6::Widgets)
