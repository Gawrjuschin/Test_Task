#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "treewidget.h"
#include "textdelegate.h"
#include "salarydelegate.h"
#include "command.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QMouseEvent>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , p_treeWidget(new TreeWidget)
  , p_file(nullptr)
{
  auto* tree_delegate = new TextDelegate;
  auto* salary_delegate = new SalaryDelegate;

  p_treeWidget->setItemDelegateForColumn(0, tree_delegate);
  p_treeWidget->setItemDelegateForColumn(1, salary_delegate);

  ui->setupUi(this);
  auto* headerItem = p_treeWidget->headerItem();
  headerItem->setData(0, Qt::DisplayRole, QString(tr("Demartment")));
  headerItem->setData(1, Qt::DisplayRole, QString(tr("Salary")));

  auto* header = p_treeWidget->header();
  header->setStretchLastSection(false);
  header->setSectionResizeMode(0, QHeaderView::Stretch);
  header->setSectionResizeMode(1, QHeaderView::ResizeToContents);

  // Сброс выделения
  p_treeWidget->setCurrentItem(nullptr);

  setCentralWidget(p_treeWidget);

  // Делегаты
  connect(tree_delegate, &TextDelegate::dataChanged, this, &MainWindow::slot_dataChanged);
  connect(salary_delegate,&SalaryDelegate::dataChanged, this, &MainWindow::slot_dataChanged);
  // Меню -> Файл
  connect(ui->actOpen,    &QAction::triggered, this, &MainWindow::slot_open);
  connect(ui->actSave,    &QAction::triggered, this, &MainWindow::slot_save);
  connect(ui->actSaveAs,  &QAction::triggered, this, &MainWindow::slot_saveAs);
  connect(ui->actExit,    &QAction::triggered, qApp, &QApplication::quit);
  // Меню -> Правка
  connect(ui->actAdd,     &QAction::triggered, this, &MainWindow::slot_add);
  connect(ui->actDelete,  &QAction::triggered, this, &MainWindow::slot_delete);
  connect(ui->actUndo,    &QAction::triggered, this, &MainWindow::slot_undo);
  connect(ui->actRedo,    &QAction::triggered, this, &MainWindow::slot_redo);
  connect(ui->menuFile,   &QMenu::aboutToShow, this, &MainWindow::slot_updateFileMenu);
  connect(ui->menuEdit,   &QMenu::aboutToShow, this, &MainWindow::slot_updateEditMenu);
}

MainWindow::~MainWindow()
{
  delete ui;
  qDeleteAll(m_undoStack);
  qDeleteAll(m_redoStack);
  delete p_file;
}

void MainWindow::addDepartment()
{
  // Добавление отдела (заглушка
  auto* department = new QTreeWidgetItem;

  department->setFlags(department->flags() | Qt::ItemIsEditable);

  department->setText(0, QString(tr("Department name")));
  department->setData(1, Qt::DisplayRole, 0);

  p_treeWidget->addTopLevelItem(department);

  auto index = p_treeWidget->indexOfTopLevelItem(department);
  auto* addCommand = new AddDepartmentCommand(department, p_treeWidget, index);
  m_undoStack.push(addCommand);
}

void MainWindow::addEmployee()
{
  // Добавляем к отделу (проверка уже сделана в вызывающей функции)
  auto* department = p_treeWidget->currentItem();
  // Должность
  auto* function = new QTreeWidgetItem;
  function->setFlags(function->flags() | Qt::ItemIsEditable);
  function->setText(0, QString(tr("Position")));
  function->setData(1, Qt::DisplayRole, 0);
  department->addChild(function);

  // Данные работника
  auto* surname = new QTreeWidgetItem(function);
  auto* name    = new QTreeWidgetItem(function);
  auto* middlename = new QTreeWidgetItem(function);

  surname->setFlags(surname->flags() | Qt::ItemIsEditable);
  name->setFlags(surname->flags() | Qt::ItemIsEditable);
  middlename->setFlags(surname->flags() | Qt::ItemIsEditable);

  surname->setText(0, QString(tr("Surname")));
  name->setText(0, QString(tr("Name")));
  middlename->setText(0, QString(tr("Middlename")));

  // Обновлеие средней з/п
  department->setData(1,Qt::DisplayRole,
                      (department->data(1, Qt::DisplayRole).toDouble() * (department->childCount() - 1))
                      / department->childCount());

  auto index = department->indexOfChild(function);
  auto* addCommand = new AddEmployeeCommand(function, department, index);
  m_undoStack.push(addCommand);
}

void MainWindow::slot_add()
{
  int depth = 0;
  auto* currentItem = p_treeWidget->currentItem();

  while( currentItem )
    { // Определяю вложенность
      currentItem = currentItem->parent();
      ++depth;
    }
  switch(depth)
    {
    case 0:
      {
        addDepartment();
        break;
      }
    case 1:
      {
        addEmployee();
        break;
      }
    default:
      {
        break;
      }
    }
}

void MainWindow::slot_delete()
{
  int level = 0;
  auto* currentItem = p_treeWidget->currentItem();

  while( currentItem )
    { // Определяю вложенность
      currentItem = currentItem->parent();
      ++level;
    }
  switch(level)
    {
    case 1:
      {
        auto index = p_treeWidget->indexOfTopLevelItem(p_treeWidget->currentItem());
        auto* item = p_treeWidget->takeTopLevelItem(index);
        m_undoStack.append(new RemoveDepartmentCommand(item, p_treeWidget, index));
        break;
      }
    case 2:
      {
        auto* currentItem = p_treeWidget->currentItem();
        auto* parent = currentItem->parent();
        auto index = parent->indexOfChild(currentItem);
        parent->removeChild(currentItem);
        m_undoStack.append(new RemoveEmployeeCommand(currentItem, parent, index));
        break;
      }
    default:
      {
        break;
      }
    }
}

void MainWindow::slot_updateFileMenu()
{
  ui->actSave->setDisabled(m_undoStack.isEmpty());
  ui->actSaveAs->setDisabled(m_undoStack.isEmpty());
}

void MainWindow::slot_updateEditMenu()
{
  int depth = 0;
  auto* currentItem = p_treeWidget->currentItem();

  while( currentItem )
    { // Определяю вложенность
      currentItem = currentItem->parent();
      ++depth;
    }

  switch(depth)
    { // Обновление add/delete
    case 0:
      {
        ui->actAdd->setDisabled(false);
        ui->actAdd->setText(tr("Add department"));
        ui->actDelete->setDisabled(true);
        ui->actDelete->setText(tr("Delete"));
        break;
      }
    case 1:
      {
        ui->actAdd->setDisabled(false);
        ui->actAdd->setText(tr("Add employee"));
        ui->actDelete->setDisabled(false);
        ui->actDelete->setText(tr("Delete department"));
        break;
      }
    case 2:
      {
        ui->actAdd->setDisabled(true);
        ui->actAdd->setText(tr("Add"));
        ui->actDelete->setDisabled(false);
        ui->actDelete->setText(tr("Delete employee"));
        break;
      }
    default:
      {
        ui->actAdd->setDisabled(true);
        ui->actAdd->setText(tr("Add"));
        ui->actDelete->setDisabled(true);
        ui->actDelete->setText(tr("Delete"));
      }
    }
  ui->actRedo->setDisabled(m_redoStack.isEmpty());
  ui->actUndo->setDisabled(m_undoStack.isEmpty());
}

void MainWindow::slot_undo()
{
  auto* command = m_undoStack.pop();
  command->cancel();
  m_redoStack.push(command);
}

void MainWindow::slot_redo()
{
  auto* command = m_redoStack.pop();
  command->execute();
  m_undoStack.push(command);
}

void MainWindow::slot_dataChanged(QVariant oldData, int colNum)
{
  auto* item = p_treeWidget->currentItem();
  switch( colNum )
    {
    case 0:
      {
        m_undoStack.append(new EditTextCommand(item, oldData));
        break;
      }
    case 1:
      {
        m_undoStack.append(new EditSalaryCommand(item, oldData));
        break;
      }
    default:
      {
        break;
      }
    }
}


void MainWindow::readDepartments(QXmlStreamReader& reader)
{ // Считывает все отделы
  reader.readNext();

  while( !(reader.isEndElement() && reader.name() == QString::fromLatin1("departments"))  )
    { // Считываем все отделы из перечня
      if (reader.isStartElement() && reader.name() == QString::fromLatin1("department"))
        { // Найден отдел
          readDepartment(reader);
        }
      reader.readNext();
    }
}

void MainWindow::readDepartment(QXmlStreamReader& reader)
{ // Добавляет считанный из readEmployments отдел в список

  auto* department = new QTreeWidgetItem;
  department->setFlags(department->flags() | Qt::ItemIsEditable);

  auto data = reader.attributes().front().value().toUtf8();
  department->setText(0, data);
  department->setData(1, Qt::DisplayRole, 0);

  reader.readNext();

  while( !(reader.isEndElement() && reader.name() == QString::fromLatin1("department")) )
    { // Считываем всё содержимое отдела
      if (reader.isStartElement() && reader.name() == QString::fromLatin1("employments"))
        { // Найден перечень сотрудников
          readEmployments(reader, department);
          p_treeWidget->addTopLevelItem(department);
        }
      reader.readNext();
    } // Считаны все перечни сотрудников отдела
}

QTreeWidgetItem* MainWindow::readEmployments(QXmlStreamReader& reader, QTreeWidgetItem* department)
{  // Вернёт отдел с сотрудниками
  reader.readNext();

  while( !(reader.isEndElement() && reader.name() == QString::fromLatin1("employments"))  )
    { // Считываем всех сотрудников из перчня
      if (reader.isStartElement() && reader.name() == QString::fromLatin1("employment"))
        { // Найден сотрудник
          auto* employment = readEmployment(reader);
          auto old_summ = department->data(1, Qt::DisplayRole).toDouble()
              * department->childCount();
          department->addChild(employment);

          // Обновление средней з/п
          auto new_summ = (old_summ + employment->data(1, Qt::DisplayRole).toInt())
              / department->childCount();
          department->setData(1, Qt::DisplayRole, new_summ);
        }
      reader.readNext();

    }
  // Считан перечень сотрудников отдела
  return department;
}

QTreeWidgetItem* MainWindow::readEmployment(QXmlStreamReader& reader)
{ // Вернёт сотрудника с данными

  auto* employment = new QTreeWidgetItem;
  auto* surname = new QTreeWidgetItem(employment);
  auto* name = new QTreeWidgetItem(employment);
  auto* middlename = new QTreeWidgetItem(employment);

  employment->setFlags(employment->flags() | Qt::ItemIsEditable);
  surname->setFlags(employment->flags() | Qt::ItemIsEditable);
  name->setFlags(employment->flags() | Qt::ItemIsEditable);
  middlename->setFlags(employment->flags() | Qt::ItemIsEditable);

  surname->setText(0, QString::fromLatin1("Surname"));
  name->setText(0, QString::fromLatin1("Name"));
  middlename->setText(0, QString::fromLatin1("Middlename"));

  reader.readNext();

  while( !(reader.isEndElement() && reader.name() == QString::fromLatin1("employment")) )
    { // Считываем все данные сотрудника

      if(reader.isStartElement() && reader.name() == QString::fromLatin1("surname"))
        {
          reader.readNext();
          auto data = reader.text().toUtf8();
          surname->setText(0, data);
        }

      if(reader.isStartElement() && reader.name() == QString::fromLatin1("name"))
        {
          reader.readNext();
          auto data = reader.text().toUtf8();
          name->setText(0, data);
        }

      if(reader.isStartElement() && reader.name() == QString::fromLatin1("middleName"))
        {
          reader.readNext();
          auto data = reader.text().toUtf8();
          middlename->setText(0, data);
        }
      if(reader.isStartElement() && reader.name() == QString::fromLatin1("function"))
        {
          reader.readNext();
          auto data = reader.text().toUtf8();
          employment->setText(0, data);
        }
      if(reader.isStartElement() && reader.name() == QString::fromLatin1("salary"))
        {
          reader.readNext();
          auto data = reader.text().toInt();
          employment->setData(1, Qt::DisplayRole, data);
        }
      reader.readNext();
    }
  // Считан сотрудник из перечня
  return employment;
}

void MainWindow::slot_open()
{
  // Резервная копия указателя на файл
  auto* old_file = p_file;

  while( p_treeWidget->topLevelItemCount())
    {
      auto* item = p_treeWidget->takeTopLevelItem(0);
      delete item;
    }

  // Важно очистить стеки команд
  qDeleteAll(m_redoStack.begin(), m_redoStack.end());
  qDeleteAll(m_undoStack.begin(), m_undoStack.end());
  m_redoStack.clear();
  m_undoStack.clear();

  auto file = QFileDialog::getOpenFileName(this, tr("Open XML"), QDir::homePath(), tr("XML File (*.xml)"));

  p_file = new QFile(file);

  if (!p_file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
      // Восстановил старый указатель
      delete p_file;
      p_file = old_file;

      QMessageBox msgBox;
      msgBox.setText(tr("Could not open document!"));
      msgBox.exec();
    }
  else
    {
      ui->actSave->setDisabled(false);
      ui->actSaveAs->setDisabled(false);

      QXmlStreamReader reader(p_file);
      reader.readNext(); // Считываю первый элемент

      while (!reader.atEnd() && !reader.hasError())
        { // Считываю весь файл
          if (reader.isStartElement() && reader.name() == QString::fromLatin1("departments"))
            { // Найден перечень отделов
              readDepartments(reader);
            }
          reader.readNext();
        } // Считан документ
      p_file->close();
    }
}

void MainWindow::saveDocument()
{
  QXmlStreamWriter writer(p_file);

  auto N = p_treeWidget->topLevelItemCount();

  writer.setAutoFormatting(true);

  writer.writeStartDocument();

  for(auto i = 0; i < N; ++i)
    {
      writer.writeStartElement(QString::fromLatin1("departments"));
      writeDepartment(writer, p_treeWidget->topLevelItem(i));
      writer.writeEndElement();
    }
  writer.writeEndDocument();

  p_file->close();
}

void MainWindow::writeEmployee(QXmlStreamWriter& writer, QTreeWidgetItem* employee)
{
  writer.writeStartElement(QString::fromLatin1("employment"));

  writer.writeStartElement(QString::fromLatin1("name"));
  auto* name = employee->child(1);
  writer.writeCharacters(name->text(0));
  writer.writeEndElement();

  writer.writeStartElement(QString::fromLatin1("surname"));
  auto* surname = employee->child(0);
  writer.writeCharacters(surname->text(0));
  writer.writeEndElement();

  writer.writeStartElement(QString::fromLatin1("middleName"));
  auto* middlename = employee->child(2);
  writer.writeCharacters(middlename->text(0));
  writer.writeEndElement();

  writer.writeStartElement(QString::fromLatin1("function"));
  writer.writeCharacters(employee->text(0));
  writer.writeEndElement();

  writer.writeStartElement(QString::fromLatin1("salary"));
  writer.writeCharacters(employee->text(1));
  writer.writeEndElement();

  writer.writeEndElement();
}

void MainWindow::writeEmployees(QXmlStreamWriter& writer, QTreeWidgetItem* department)
{
  auto N = department->childCount();
  writer.writeStartElement(QString::fromLatin1("employments"));
  for(auto i = 0; i < N; ++i)
    {
      writeEmployee(writer, department->child(i));
    }

  writer.writeEndElement();
}

void MainWindow::writeDepartment(QXmlStreamWriter& writer, QTreeWidgetItem* department)
{
  writer.writeStartElement(QString::fromLatin1("department"));
  writer.writeAttribute(QString::fromLatin1("id"), department->text(0));
  writeEmployees(writer, department);
  writer.writeEndElement();
}

void MainWindow::slot_save()
{
  if(p_file == nullptr)
    {
      slot_saveAs();
      return;
    }
  if (!p_file->open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QMessageBox msgBox;
      msgBox.setText(tr("Could not open document!"));
      msgBox.exec();
    }
  else
    {
      saveDocument();
    }
}

void MainWindow::slot_saveAs()
{
  auto old_file = p_file;

  auto file = QFileDialog::getSaveFileName(this, tr("Save XML"), QDir::homePath(), tr("XML File (*.xml)"));

  p_file = new QFile(file);

  if (!p_file->open(QIODevice::WriteOnly | QIODevice::Text))
    {
      // Восстановил старый указатель
      delete p_file;
      p_file = old_file;

      QMessageBox msgBox;
      msgBox.setText(tr("Could not open document!"));
      msgBox.exec();
    }
  else
    {
      saveDocument();
    }
}
