#include "add_node_dialog.hpp"

#include <QVBoxLayout>

namespace ravel::ui {

AddNodeDialog::AddNodeDialog(const QStringList& types, QWidget* parent) : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);
  type_ = new QComboBox(this);
  type_->addItems(types);
  title_ = new QLineEdit(this);
  description_ = new QPlainTextEdit(this);
  size_ = new QDoubleSpinBox(this);
  size_->setValue(1.0);
  size_->setRange(0.5, 5.0);
  size_->setSingleStep(0.1);
  size_->setDecimals(1);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  title_->setPlaceholderText(tr("Название"));
  description_->setPlaceholderText(tr("Описание (необязательно)"));

  layout->addWidget(type_);
  layout->addWidget(title_);
  layout->addWidget(description_);
  layout->addWidget(size_);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString AddNodeDialog::type() const {
  return type_->currentText();
}

QString AddNodeDialog::title() const {
  return title_->text();
}

QString AddNodeDialog::description() const {
  return description_->toPlainText();
}
double AddNodeDialog::size() const {
  return size_->value();
}

}  // namespace ravel::ui
