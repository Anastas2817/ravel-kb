#ifndef RAVEL_UI_INCLUDE_OPEN_NODE_DIALOG_HPP_
#define RAVEL_UI_INCLUDE_OPEN_NODE_DIALOG_HPP_

#include "controller.hpp"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>

namespace ravel::ui {

class NodeItem;

class OpenNodeDialog : public QDialog {
  Q_OBJECT
 public:
  OpenNodeDialog(ravel::app::NodeView node, const QStringList& types, QWidget* parent = nullptr);
  [[nodiscard]] QString type() const;
  [[nodiscard]] QString title() const;
  [[nodiscard]] QString description() const;
  [[nodiscard]] double size() const;

 private:
  QComboBox* type_;
  QLineEdit* title_;
  QPlainTextEdit* description_;
  QDoubleSpinBox* size_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_OPEN_NODE_DIALOG_HPP_
