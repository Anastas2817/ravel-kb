#ifdef _MSC_VER
#pragma warning(disable : 26495 26813)
#endif

#include <QApplication>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>

#include "node_item.hpp"
#include "relation_item.hpp"

using ravel::ui::NodeItem;
using ravel::ui::RelationItem;

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("Ravel"));

  QMainWindow window;
  window.setWindowTitle(QStringLiteral("Ravel"));
  window.resize(1024, 768);
  window.show();

  QGraphicsScene* scene = new QGraphicsScene(&window);

  auto* node1 = new NodeItem(1, QRectF(0, 0, 120, 60));
  scene->addItem(node1);
  auto* node2 = new NodeItem(2, QRectF(0, 0, 300, 60));
  scene->addItem(node2);

  auto* relation = new RelationItem(node1, node2);
  scene->addItem(relation);

  QObject::connect(node1, &NodeItem::Moved, [&window](std::uint64_t id, const QPointF& p) {
    window.setWindowTitle(QStringLiteral("node %1: (%2, %3)").arg(id).arg(p.x()).arg(p.y()));
  });

  QGraphicsView* view = new QGraphicsView(scene, &window);
  window.setCentralWidget(view);

  window.show();

  return QApplication::exec();
}
