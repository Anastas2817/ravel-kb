#ifndef RAVEL_UI_INCLUDE_ADD_RELATION_DIALOG_HPP_
#define RAVEL_UI_INCLUDE_ADD_RELATION_DIALOG_HPP_

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>

namespace ravel::ui {

class AddRelationDialog : public QDialog {
  Q_OBJECT
 public:
  AddRelationDialog(const QStringList& types, const QString& from_title, const QString& to_title,
                    QWidget* parent = nullptr);
  [[nodiscard]] QString type() const;
  [[nodiscard]] QString description() const;

 private:
  QComboBox* type_;
  QLineEdit* description_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_ADD_RELATION_DIALOG_HPP_
