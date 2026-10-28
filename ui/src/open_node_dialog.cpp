#include "open_node_dialog.hpp"

#include <QVBoxLayout>

namespace ravel::ui {

OpenNodeDialog::OpenNodeDialog(ravel::app::NodeView node, const QStringList& types, QWidget* parent)
    : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);

  type_ = new QComboBox(this);
  type_->addItems(types);
  type_->setCurrentText(QString::fromStdString(node.type));
  layout->addWidget(type_);

  title_ = new QLineEdit(this);
  title_->setText(QString::fromStdString(node.title));
  title_->setPlaceholderText(tr("Название"));
  layout->addWidget(title_);

  description_ = new QPlainTextEdit(this);
  description_->setPlainText(QString::fromStdString(node.description));
  description_->setPlaceholderText(tr("Описание (необязательно)"));
  layout->addWidget(description_);

  size_ = new QDoubleSpinBox(this);
  size_->setRange(0.5, 5.0);
  size_->setSingleStep(0.1);
  size_->setDecimals(1);
  size_->setValue(node.size);
  layout->addWidget(size_);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString OpenNodeDialog::type() const {
  return type_->currentText();
}

QString OpenNodeDialog::title() const {
  return title_->text();
}

QString OpenNodeDialog::description() const {
  return description_->toPlainText();
}
double OpenNodeDialog::size() const {
  return size_->value();
}

}  // namespace ravel::ui
