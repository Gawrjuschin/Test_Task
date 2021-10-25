#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{ // Наследование ради перегрузки mousePressEvent
  // ради снятия выделения при клику по пустому месту
    Q_OBJECT
public:
    TreeWidget(QWidget* parent = nullptr);
protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
};

#endif // TREEWIDGET_H
