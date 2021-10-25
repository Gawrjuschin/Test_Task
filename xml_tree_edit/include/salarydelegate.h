#ifndef SALARYDELEGATE_H
#define SALARYDELEGATE_H

#include <QItemDelegate>

class QTreeWidgetItem;

class SalaryDelegate : public QItemDelegate
{
    Q_OBJECT

    // Делегат с подсчётом средней з/п
public:
    SalaryDelegate();

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

};

#endif // SALARYDELEGATE_H
