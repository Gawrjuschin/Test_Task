#include "salarydelegate.h"

#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QRegularExpressionValidator>

SalaryDelegate::SalaryDelegate()
  : p_validator(new QRegularExpressionValidator(this))
{
  QRegularExpression rx("[0-9]{1,6}"); // В полуинтервале [0; 1'000'000)
  p_validator->setRegularExpression(rx);
}

QWidget* SalaryDelegate::createEditor(QWidget* parent,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
  if(index.parent().isValid())
    {
      if( !index.parent().parent().isValid() )
        {
          auto* editor = new QLineEdit(parent);
          editor->setValidator(p_validator);
          return editor;
        }
    }
  return nullptr;
}

void 	SalaryDelegate::setEditorData(QWidget* editor,
                                      const QModelIndex& index) const
{
  auto* lineEdit = static_cast<QLineEdit*>(editor);
  lineEdit->setValidator(p_validator);
  lineEdit->setText(index.data(Qt::DisplayRole).toString());
}

void SalaryDelegate::setModelData(QWidget *editor,
                                  QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
  if(editor)
    {
      constexpr auto salaryCol = 1;
      auto* lineEdit = static_cast<QLineEdit*>(editor);
      auto newData = lineEdit->text().toInt();

      auto* department = static_cast<QTreeWidgetItem*>(index.parent().internalPointer());

      auto old_avg = department->data(1, Qt::DisplayRole).toDouble();
      auto old_data = index.data();
      auto old_summ = (old_avg * department->childCount());
      auto new_avg = (old_summ - old_data.toInt() + lineEdit->text().toInt())
          / department->childCount();

      department->setData(salaryCol ,Qt::DisplayRole, new_avg);
      model->setData(index, newData, Qt::EditRole);
      emit dataChanged( old_data, salaryCol );
    }
}

void SalaryDelegate::updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}
