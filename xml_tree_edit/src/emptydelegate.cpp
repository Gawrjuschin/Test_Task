#include "emptydelegate.h"

EmptyDelegate::EmptyDelegate(QObject *parent) : QItemDelegate(parent)
{

}

QWidget* EmptyDelegate::createEditor(QWidget* parent,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
  return nullptr;
}
