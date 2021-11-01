#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "treewidget.h"
#include "textdelegate.h"
#include "salarydelegate.h"
#include "emptydelegate.h"
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
  auto* empty_delegate = new EmptyDelegate;
  auto* text_delegate = new TextDelegate;
  auto* salary_delegate = new SalaryDelegate;

  // Делегаты для столбцов
  p_treeWidget->setItemDelegateForColumn(0, empty_delegate);
  p_treeWidget->setItemDelegateForColumn(1, salary_delegate);
  p_treeWidget->setItemDelegateForColumn(2, text_delegate);

  ui->setupUi(this);

  // Настройка заголовка
  auto* headerItem = p_treeWidget->headerItem();
  headerItem->setData(0, Qt::DisplayRole, tr("Count"));
  headerItem->setData(1, Qt::DisplayRole, tr("Salary"));
  headerItem->setData(2, Qt::DisplayRole, tr("Demartment"));

  auto* header = p_treeWidget->header();
  header->setStretchLastSection(false);
  header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(2, QHeaderView::Stretch);

  // Сброс выделения
  p_treeWidget->setCurrentItem(nullptr);

  setCentralWidget(p_treeWidget);

  // Делегаты
  connect(text_delegate, &TextDelegate::dataChanged, this, &MainWindow::slot_dataChanged);
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
  // Добавление отдела
  auto* department = new QTreeWidgetItem;

  department->setFlags(department->flags() | Qt::ItemIsEditable);

  department->setData(0, Qt::DisplayRole, 0);
  department->setData(1, Qt::DisplayRole, tr("N/A"));
  department->setText(2, tr("Department name"));

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
  function->setData(0, Qt::DisplayRole, QString());
  function->setData(1, Qt::DisplayRole, 0);
  function->setText(2, tr("Employment"));
  department->addChild(function);

  // Данные работника
  auto* surname = new QTreeWidgetItem(function);
  auto* name    = new QTreeWidgetItem(function);
  auto* middlename = new QTreeWidgetItem(function);

  surname->setFlags(surname->flags() | Qt::ItemIsEditable);
  name->setFlags(surname->flags() | Qt::ItemIsEditable);
  middlename->setFlags(surname->flags() | Qt::ItemIsEditable);

  surname->setText(2, tr("Surname"));
  name->setText(2, tr("Name"));
  middlename->setText(2, tr("Middlename"));

  // Обновлеие средней з/п и числа сотрудников
  department->setData(1,Qt::DisplayRole,
                      (department->data(1, Qt::DisplayRole).toDouble() * (department->childCount() - 1))
                      / department->childCount());
  department->setData(0,Qt::DisplayRole, department->childCount());

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
    { // Код из команд
    case 1:
      {
        auto index = p_treeWidget->indexOfTopLevelItem(p_treeWidget->currentItem());
        auto* item = p_treeWidget->takeTopLevelItem(index);
        m_undoStack.append(new RemoveDepartmentCommand(item, p_treeWidget, index));
        break;
      }
    case 2:
      { // Нужно обновить з/п и количество сотрудников
        auto* currentItem = p_treeWidget->currentItem();
        auto* parent = currentItem->parent();
        auto old_sum = parent->data(1, Qt::DisplayRole).toDouble() * parent->childCount();
        auto index = parent->indexOfChild(currentItem);
        parent->removeChild(currentItem);
        parent->setData(0, Qt::DisplayRole, parent->childCount());
        if(parent->childCount())
          { // Деление на ноль
            parent->setData(1, Qt::DisplayRole, (old_sum - currentItem->data(1, Qt::DisplayRole).toInt()) / parent->childCount());
          }
        else
          {
            parent->setData(1, Qt::DisplayRole, tr("N/A"));
          }
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
    { // Определяем, что за столбец изменился
    case ColNum::SALARY:
      {
        m_undoStack.append(new EditSalaryCommand(item, oldData));
        break;
      }
    case ColNum::TEXT:
      {
        m_undoStack.append(new EditTextCommand(item, oldData));
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
  do
    { // Считываем все отделы из перечня
      reader.readNext();
      if (reader.isStartElement() && reader.name() == QString("department"))
        { // Найден отдел
          readDepartment(reader);
        }
    }
  while( !(reader.isEndElement() && reader.name() == QString("departments")) && !reader.atEnd() );

}

void MainWindow::readDepartment(QXmlStreamReader& reader)
{ // Добавляет считанный из readEmployments отдел в список
  auto* department = new QTreeWidgetItem; // Новый отдел
  department->setFlags(department->flags() | Qt::ItemIsEditable);

  department->setData(0, Qt::DisplayRole, 0);
  department->setData(1, Qt::DisplayRole, 0);

  QByteArray data{};
  if(!reader.attributes().isEmpty())
    { // Если есть аттрибут
      data = reader.attributes().front().value().toUtf8();
    }
  else
    {
      data = tr("Department name").toUtf8();
    }
  department->setText(2, data);

  do
    { // Считываем всё содержимое отдела
      reader.readNext();
      if (reader.isStartElement() && reader.name() == QString("employments"))
        { // Найден перечень сотрудников
          readEmployments(reader, department);
          if( department->childCount() == 0 )
            { // Пустой отдел
              department->setData(1, Qt::DisplayRole, tr("N/A"));
            }
          p_treeWidget->addTopLevelItem(department);
        }
    } // Считаны все перечни сотрудников отдела
  while( !(reader.isEndElement() && reader.name() == QString("department")) && !reader.atEnd() );

}

QTreeWidgetItem* MainWindow::readEmployments(QXmlStreamReader& reader, QTreeWidgetItem* department)
{  // Вернёт отдел с сотрудниками

  do
    { // Считываем всех сотрудников из перчня
      reader.readNext();

      if (reader.isStartElement() && reader.name() == QString("employment"))
        { // Найден сотрудник
          auto* employment = readEmployment(reader); // записываю нового

          // Обновление средней з/п и количества сотрудников
          auto old_summ = department->data(1, Qt::DisplayRole).toDouble()
              * department->childCount();
          department->addChild(employment);

          auto new_salary = (old_summ + employment->data(1, Qt::DisplayRole).toInt())
              / department->childCount();
          department->setData(1, Qt::DisplayRole, new_salary);

          department->setData(0, Qt::DisplayRole, department->childCount());

        }

    } while( !(reader.isEndElement() && reader.name() == QString("employments")) && !reader.atEnd() );

  return department;

}

QTreeWidgetItem* MainWindow::readEmployment(QXmlStreamReader& reader)
{ // Вернёт сотрудника с данными

  auto* employment = new QTreeWidgetItem;
  auto* surname = new QTreeWidgetItem(employment);
  auto* name = new QTreeWidgetItem(employment);
  auto* middlename = new QTreeWidgetItem(employment);

  employment->setText(2, tr("Employment"));
  employment->setData(1, Qt::DisplayRole, 0);

  surname->setText(2, tr("Surname"));
  name->setText(2, tr("Name"));
  middlename->setText(2, tr("Middlename"));

  employment->setFlags(employment->flags() | Qt::ItemIsEditable);
  surname->setFlags(employment->flags() | Qt::ItemIsEditable);
  name->setFlags(employment->flags() | Qt::ItemIsEditable);
  middlename->setFlags(employment->flags() | Qt::ItemIsEditable);

  do
    { // Считываем все данные сотрудника (отсутствующие будут запослнены значением по умолчанию)
      reader.readNext();

      if(reader.isStartElement())
        {
          auto data = reader.readElementText();

          if(reader.name() == QString("surname"))
            {
              surname->setText(2, data);
            }

          if(reader.name() == QString("name"))
            {
              name->setText(2, data);
            }

          if(reader.name() == QString("middleName"))
            {
              middlename->setText(2, data);
            }
          if(reader.name() == QString("function"))
            {
              employment->setText(2, data);
            }
          if(reader.name() == QString("salary"))
            {
              employment->setData(1, Qt::DisplayRole, data.toInt());
            }
        }


    } while( !(reader.isEndElement() && reader.name() == QString("employment")) && !reader.atEnd() );

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
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setWindowTitle(tr("Error!"));
      msgBox.setText(tr("Could not open document!"));
      msgBox.exec();
    }
  else
    {
      ui->actSave->setDisabled(false);
      ui->actSaveAs->setDisabled(false);

      QXmlStreamReader reader(p_file);

      do
        { // Считываю весь файл
          reader.readNext();
          if (reader.isStartElement() && reader.name() == QString("departments"))
            { // Найден перечень отделов
              readDepartments(reader);
            }
        } while( !reader.atEnd() && !reader.hasError());

      if(reader.hasError())
        {
          QMessageBox msgBox;
          msgBox.setIcon(QMessageBox::Critical);
          msgBox.setWindowTitle(tr("Error!"));
          msgBox.setText(reader.errorString());
          msgBox.exec();
        }

      p_file->close();
    }
}

void MainWindow::saveDocument()
{
  QXmlStreamWriter writer(p_file);
  writer.setAutoFormatting(true);

  writer.writeStartDocument();

  writer.writeStartElement(QString("departments"));

  auto N = p_treeWidget->topLevelItemCount();
  for(auto i = 0; i < N; ++i)
    {
      writeDepartment(writer, p_treeWidget->topLevelItem(i));
    }
  writer.writeEndElement();

  writer.writeEndDocument();

  p_file->close();
}

void MainWindow::writeEmployee(QXmlStreamWriter& writer, QTreeWidgetItem* employee)
{
  writer.writeStartElement(QString("employment"));

  writer.writeStartElement(QString("name"));
  auto* name = employee->child(1);
  writer.writeCharacters(name->text(2));
  writer.writeEndElement();

  writer.writeStartElement(QString("surname"));
  auto* surname = employee->child(0);
  writer.writeCharacters(surname->text(2));
  writer.writeEndElement();

  writer.writeStartElement(QString("middleName"));
  auto* middlename = employee->child(2);
  writer.writeCharacters(middlename->text(2));
  writer.writeEndElement();

  writer.writeStartElement(QString("function"));
  writer.writeCharacters(employee->text(2));
  writer.writeEndElement();

  writer.writeStartElement(QString("salary"));
  writer.writeCharacters(employee->text(1));
  writer.writeEndElement();

  writer.writeEndElement();
}

void MainWindow::writeEmployees(QXmlStreamWriter& writer, QTreeWidgetItem* department)
{
  auto N = department->childCount();
  writer.writeStartElement(QString("employments"));
  for(auto i = 0; i < N; ++i)
    {
      writeEmployee(writer, department->child(i));
    }

  writer.writeEndElement();
}

void MainWindow::writeDepartment(QXmlStreamWriter& writer, QTreeWidgetItem* department)
{
  writer.writeStartElement(QString("department"));
  writer.writeAttribute(QString("id"), department->text(2));
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
