#ifndef RAVEL_UI_INCLUDE_ADD_NODE_DIALOG_HPP_
#define RAVEL_UI_INCLUDE_ADD_NODE_DIALOG_HPP_

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>

namespace ravel::ui {

class AddNodeDialog : public QDialog {
  Q_OBJECT
 public:
  AddNodeDialog(const QStringList& types, QWidget* parent = nullptr);
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

#endif  // RAVEL_UI_INCLUDE_ADD_NODE_DIALOG_HPP_
