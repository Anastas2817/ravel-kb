#include "open_relation_dialog.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace ravel::ui {

OpenRelationDialog::OpenRelationDialog(ravel::app::RelationView rel, const QStringList& types,
                                       const QString& from_title, const QString& to_title,
                                       QWidget* parent)
    : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);

  type_ = new QComboBox(this);
  type_->addItems(types);
  type_->setCurrentText(QString::fromStdString(rel.type));
  auto* row = new QHBoxLayout;
  row->addWidget(new QLabel(from_title));
  row->addWidget(type_);
  row->addWidget(new QLabel(to_title));
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(4);
  layout->addLayout(row);

  description_ = new QLineEdit(this);
  description_->setText(QString::fromStdString(rel.description));
  layout->addWidget(description_);
  description_->setPlaceholderText(tr("Описание (необязательно)"));

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString OpenRelationDialog::type() const {
  return type_->currentText();
}

QString OpenRelationDialog::description() const {
  return description_->text();
}

}  // namespace ravel::ui
