#include "add_relation_dialog.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace ravel::ui {

AddRelationDialog::AddRelationDialog(const QStringList& types, const QString& from_title,
                                     const QString& to_title, QWidget* parent)
    : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);
  type_ = new QComboBox(this);
  type_->addItems(types);
  auto* row = new QHBoxLayout;
  row->addWidget(new QLabel(from_title));
  row->addWidget(type_);
  row->addWidget(new QLabel(to_title));
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(4);
  description_ = new QLineEdit(this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  description_->setPlaceholderText(tr("Описание (необязательно)"));

  layout->addLayout(row);
  layout->addWidget(description_);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString AddRelationDialog::type() const {
  return type_->currentText();
}

QString AddRelationDialog::description() const {
  return description_->text();
}

}  // namespace ravel::ui
