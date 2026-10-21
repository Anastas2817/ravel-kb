#ifdef _MSC_VER
#pragma warning(disable : 26495 26813)
#endif

#include "mainwindow.hpp"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("Ravel"));

  ravel::ui::MainWindow window;
  window.setWindowTitle(QStringLiteral("Ravel"));
  window.resize(1024, 768);
  window.show();

  return QApplication::exec();
}
