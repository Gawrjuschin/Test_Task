#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  TreeWidget(QWidget* parent = nullptr);
  // Наследование ради перегрузки mousePressEvent
  // чтобы снять выделение при клике по пустому месту

protected:
  virtual void mousePressEvent(QMouseEvent* event) override;
};

#endif // TREEWIDGET_H
