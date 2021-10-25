#ifndef EMPTYDELEGATE_H
#define EMPTYDELEGATE_H

#include <QItemDelegate>

class EmptyDelegate : public QItemDelegate
{
public:
  explicit EmptyDelegate(QObject *parent = nullptr);

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;
};

#endif // EMPTYDELEGATE_H
