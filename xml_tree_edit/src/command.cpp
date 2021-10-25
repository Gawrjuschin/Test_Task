#include "command.h"
#include <QVariant>
#include <QTreeWidgetItem>

Command::~Command() = default;


// Добавление отдела
AddDepartmentCommand::AddDepartmentCommand(QTreeWidgetItem* item, QTreeWidget* parent, int index)
  : p_item(item), p_parent(parent), m_index(index)
{

}
AddDepartmentCommand::~AddDepartmentCommand() = default;

void AddDepartmentCommand::execute()
{ // redo
  p_parent->insertTopLevelItem(m_index, p_item);
}
void AddDepartmentCommand::cancel()
{ // undo
  p_parent->takeTopLevelItem(m_index);
}

// Добавление работника
AddEmployeeCommand::AddEmployeeCommand(QTreeWidgetItem* item, QTreeWidgetItem* parent, int index)
  : p_item(item), p_parent(parent), m_index(index)
{

}

AddEmployeeCommand::~AddEmployeeCommand() = default;

void AddEmployeeCommand::execute()
{ // redo
  auto old_sum = p_parent->data(1,Qt::DisplayRole).toDouble() * p_parent->childCount();
  p_parent->insertChild(m_index, p_item);
  p_parent->setData(0, Qt::DisplayRole, p_parent->childCount());
  p_parent->setData(1, Qt::DisplayRole, (old_sum + p_item->data(1, Qt::DisplayRole).toInt()) / p_parent->childCount());
}
void AddEmployeeCommand::cancel()
{ // undo
  auto old_sum = p_parent->data(1,Qt::DisplayRole).toDouble() * p_parent->childCount();
  p_parent->removeChild(p_item);
  p_parent->setData(0, Qt::DisplayRole, p_parent->childCount());
  if(p_parent->childCount())
    { // Деление на ноль
      p_parent->setData(1, Qt::DisplayRole, (old_sum - p_item->data(1, Qt::DisplayRole).toInt()) / p_parent->childCount());
    }
  else
    {
      p_parent->setData(1, Qt::DisplayRole, QString::fromLatin1("N/A"));
    }
}

// Удаление отдела
RemoveDepartmentCommand::RemoveDepartmentCommand(QTreeWidgetItem* item, QTreeWidget* parent, int index)
  : p_item(item), p_parent(parent), m_index(index)
{

}

RemoveDepartmentCommand::~RemoveDepartmentCommand() = default;

void RemoveDepartmentCommand::execute()
{ // redo
  p_parent->takeTopLevelItem(m_index);
}
void RemoveDepartmentCommand::cancel()
{ // undo
  p_parent->insertTopLevelItem(m_index, p_item);
}

// Удаление работника
RemoveEmployeeCommand::RemoveEmployeeCommand(QTreeWidgetItem* item, QTreeWidgetItem* parent, int index)
  : p_item(item), p_parent(parent), m_index(index)
{

}
RemoveEmployeeCommand::~RemoveEmployeeCommand() = default;

void RemoveEmployeeCommand::execute()
{ // redo
  auto old_sum = p_parent->data(1,Qt::DisplayRole).toDouble() * p_parent->childCount();
  p_parent->removeChild(p_item);
  p_parent->setData(0, Qt::DisplayRole, p_parent->childCount());
  if(p_parent->childCount())
    { // Деление на ноль
      p_parent->setData(1, Qt::DisplayRole, (old_sum - p_item->data(1, Qt::DisplayRole).toInt()) / p_parent->childCount());
    }
  else
    {
      p_parent->setData(1, Qt::DisplayRole, QString::fromLatin1("N/A"));
    }
}
void RemoveEmployeeCommand::cancel()
{ // undo
  auto old_sum = p_parent->data(1,Qt::DisplayRole).toDouble() * p_parent->childCount();
  p_parent->insertChild(m_index, p_item);
  p_parent->setData(0, Qt::DisplayRole, p_parent->childCount());
  p_parent->setData(1, Qt::DisplayRole, (old_sum + p_item->data(1, Qt::DisplayRole).toInt()) / p_parent->childCount());
}

// Изменение данных

EditTextCommand::EditTextCommand(QTreeWidgetItem* item, QVariant oldData)
  : p_item(item), p_oldData(new QVariant(oldData))
{

}

EditTextCommand::~EditTextCommand()
{
  delete p_oldData;
}
void EditTextCommand::execute()
{ // redo
  auto newData = p_item->data(2, Qt::DisplayRole);
  p_item->setData(2, Qt::EditRole, *p_oldData);
  p_oldData->swap(newData);
}
void EditTextCommand::cancel()
{ // undo
  auto newData = p_item->data(2, Qt::DisplayRole);
  p_item->setData(2, Qt::EditRole, *p_oldData);
  p_oldData->swap(newData);
}

// Изменение з/п требует пересчёта средней по отделу

EditSalaryCommand::EditSalaryCommand(QTreeWidgetItem* item, QVariant oldData)
  : p_item(item), p_oldData(new QVariant(oldData))
{

}

EditSalaryCommand::~EditSalaryCommand()
{
  delete p_oldData;
}

void EditSalaryCommand::execute()
{ // redo
  auto* department = p_item->parent();
  auto new_data = p_item->data(1, Qt::DisplayRole).toInt();
  auto new_summ = department->data(1, Qt::DisplayRole).toDouble() * department->childCount();
  auto old_avg = (new_summ - new_data + p_oldData->toInt()) / department->childCount();

  department->setData(1, Qt::DisplayRole, old_avg);
  p_item->setData(1, Qt::DisplayRole, p_oldData->toInt());
  p_oldData->setValue(new_data);
}
void EditSalaryCommand::cancel()
{ // undo
  execute();
}
