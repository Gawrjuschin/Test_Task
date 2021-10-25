#ifndef SALARYDELEGATE_H
#define SALARYDELEGATE_H

#include <QItemDelegate>

class QTreeWidgetItem;

class SalaryDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  SalaryDelegate();
  // Делегат с ограничением на ввод подсчётом средней з/п
  // отправляет сигнал для генерации команды

  virtual QWidget* createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const override;

  virtual void setEditorData(QWidget *editor,
                             const QModelIndex &index) const override;

  virtual void setModelData(QWidget *editor,
                            QAbstractItemModel *model,
                            const QModelIndex &index) const override;

  virtual void updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const override;
signals:
  void dataChanged(QVariant oldData, int colNum) const;

private:
  QIntValidator* p_validator;

};

#endif // SALARYDELEGATE_H
