#ifndef TEXTDELEGATE_H
#define TEXTDELEGATE_H

#include <QItemDelegate>

class QTreeWidgetItem;

class TextDelegate : public QItemDelegate
{ // Наследование делегата для обновления средней з/п
    Q_OBJECT

public:
    TextDelegate();

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
