// Ravel — точка входа десктопного приложения (Qt 6 Widgets).
// Неделя 4: каркас — пустое главное окно. Канвас (QGraphicsScene/View)
// появится здесь же, в ui/src/ (см. docs/roadmap.md, этап 2).

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QMainWindow>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("Ravel"));

  QMainWindow window;
  window.setWindowTitle(QStringLiteral("Ravel"));
  window.resize(1024, 768);
  window.show();

  QGraphicsScene* scene = new QGraphicsScene(&window);
  QGraphicsRectItem* rect1 = scene->addRect(QRectF(0, 0, 100, 50));
  rect1->setFlag(QGraphicsItem::ItemIsMovable);
  rect1->setBrush(Qt::blue);

  QGraphicsRectItem* rect2 = scene->addRect(QRectF(0, 200, 100, 50));
  rect2->setFlag(QGraphicsItem::ItemIsMovable);
  rect2->setBrush(QColor("darkblue"));

  QGraphicsRectItem* rect3 = scene->addRect(QRectF(200, 0, 100, 50));

  class NodeItem : QGraphicsRectItem {
    std::uint64_t id_ = 0;
  };
  
  QGraphicsView* view = new QGraphicsView(scene, &window);
  window.setCentralWidget(view);

  window.show();

  return QApplication::exec();
}
