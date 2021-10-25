#include "treewidget.h"

#include <QMouseEvent>


TreeWidget::TreeWidget(QWidget* parent)
  : QTreeWidget(parent)
{

}

void TreeWidget::mousePressEvent(QMouseEvent* event)
{
  auto index = indexAt(event->pos());

  if(index.isValid())
    {
      QTreeView::mousePressEvent(event);
    }
  else
    {
      clearSelection();
      selectionModel()->setCurrentIndex({},QItemSelectionModel::Select);
    }
}
