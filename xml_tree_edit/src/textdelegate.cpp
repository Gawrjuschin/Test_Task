#include "textdelegate.h"
#include <QLineEdit>

#include <QDebug>

TextDelegate::TextDelegate()
  : QItemDelegate()
{

}

QWidget* TextDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{ // Возвращает элемент ввода
  return new QLineEdit(parent);
}

void TextDelegate::setEditorData(QWidget* editor,
                                 const QModelIndex& index) const
{
  auto* lineEdit = static_cast<QLineEdit*>(editor);
  lineEdit->setText(index.data(Qt::DisplayRole).toString());
}

void TextDelegate::setModelData(QWidget *editor,
                                QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  auto* lineEdit = static_cast<QLineEdit*>(editor);
  QVariant oldData(index.data());
  QVariant newData(lineEdit->text());
  model->setData(index, newData, Qt::EditRole);
  emit dataChanged( oldData, 0 );
}

void TextDelegate::updateEditorGeometry(QWidget *editor,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}
