#ifndef TEXTDELEGATE_H
#define TEXTDELEGATE_H

#include <QItemDelegate>

class QTreeWidgetItem;

class TextDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  TextDelegate();
  // Делегат с ограничением на ввод
  // отправляет сигнал для генерации команды

  virtual QWidget *	createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const override;

  virtual void 	setEditorData(QWidget *editor,
                              const QModelIndex &index) const override;

  virtual void 	setModelData(QWidget *editor,
                             QAbstractItemModel *model,
                             const QModelIndex &index) const override;

  virtual void 	updateEditorGeometry(QWidget *editor,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const override;

signals:
  void dataChanged(QVariant oldData, int colNum) const;

private:
  QRegularExpressionValidator* p_validator;

};

#endif // TEXTELEGATE_H
