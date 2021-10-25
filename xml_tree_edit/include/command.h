#ifndef COMMAND_H
#define COMMAND_H

class QTreeWidget;
class QTreeWidgetItem;
class QVariant;
class Command

{ // Интерфейс команды
public:
    virtual ~Command();
    virtual void execute() = 0;
    virtual void cancel() = 0;
};


class AddDepartmentCommand : public Command
{ // У такого элемента нет родителя, поэтому указатель на виджет
public:
    AddDepartmentCommand(QTreeWidgetItem* item, QTreeWidget* parent, int index);
    virtual ~AddDepartmentCommand();
    virtual void execute()override;
    virtual void cancel() override;

private:
    QTreeWidgetItem* p_item;
    QTreeWidget*     p_parent;
    int              m_index;
};

class AddEmployeeCommand : public Command
{
public:
    AddEmployeeCommand(QTreeWidgetItem* item, QTreeWidgetItem* parent, int index);
    virtual ~AddEmployeeCommand();
    virtual void execute()override;
    virtual void cancel() override;

private:
    QTreeWidgetItem* p_item;
    QTreeWidgetItem* p_parent;
    int              m_index;
};





class RemoveDepartmentCommand : public Command
{ // У такого элемента нет родителя, поэтому указатель на виджет
public:
    RemoveDepartmentCommand(QTreeWidgetItem* item, QTreeWidget* parent, int index);
    virtual ~RemoveDepartmentCommand();
    virtual void execute()override;
    virtual void cancel() override;

private:
    QTreeWidgetItem* p_item;
    QTreeWidget*     p_parent;
    int              m_index;
};

class RemoveEmployeeCommand : public Command
{
public:
    RemoveEmployeeCommand(QTreeWidgetItem* item, QTreeWidgetItem* parent, int index);
    virtual ~RemoveEmployeeCommand();
    virtual void execute()override;
    virtual void cancel() override;

private:
    QTreeWidgetItem* p_item;
    QTreeWidgetItem* p_parent;
    int              m_index;
};

class EditTextCommand : public Command
{
public:
    EditTextCommand(QTreeWidgetItem* item, QVariant oldData);
    virtual ~EditTextCommand();
    virtual void execute() override;
    virtual void cancel() override;

private:
    QTreeWidgetItem* p_item;
    QVariant*        p_oldData;
};


class EditSalaryCommand : public Command
{
public:
    EditSalaryCommand(QTreeWidgetItem* item, QVariant oldData);
    virtual ~EditSalaryCommand();
    virtual void execute() override;
    virtual void cancel() override;

private:
    QTreeWidgetItem* p_item;
    QVariant*        p_oldData;
};

#endif // COMMAND_H
