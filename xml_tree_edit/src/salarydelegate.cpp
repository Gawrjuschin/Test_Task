#include "salarydelegate.h"

#include <QTreeWidgetItem>
#include <QLineEdit>

SalaryDelegate::SalaryDelegate() = default;

QWidget* SalaryDelegate::createEditor(QWidget* parent,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
  if(index.parent().isValid())
    {
      if( !index.parent().parent().isValid() )
        {
          auto* editor = new QLineEdit(parent);
          return editor;
        }
    }
  return nullptr;
}

void 	SalaryDelegate::setEditorData(QWidget* editor,
                                      const QModelIndex& index) const
{
  auto* lineEdit = static_cast<QLineEdit*>(editor);
  lineEdit->setText(index.data(Qt::DisplayRole).toString());
}

void SalaryDelegate::setModelData(QWidget *editor,
                                  QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
  if(editor)
    {
      auto* department = static_cast<QTreeWidgetItem*>(index.parent().internalPointer());
      auto* lineEdit = static_cast<QLineEdit*>(editor);

      auto old_avg = department->data(1, Qt::DisplayRole).toDouble();
      auto old_data = index.data();
      auto old_summ = (old_avg * department->childCount());
      auto new_avg = (old_summ - old_data.toInt() + lineEdit->text().toInt())
          / department->childCount();

      department->setData(1 ,Qt::DisplayRole, new_avg);
      model->setData(index, lineEdit->text(), Qt::EditRole);
      emit dataChanged( old_data, 1);
    }
}

void SalaryDelegate::updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}
