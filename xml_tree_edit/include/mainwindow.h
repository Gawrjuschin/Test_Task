#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QVariant>

class QTreeWidget;
class QTreeWidgetItem;
class QMenu;
class QXmlStreamReader;
class QXmlStreamWriter;
class QFile;

class Command;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void slot_open();
  void slot_save();
  void slot_saveAs();
  void slot_add();
  void slot_delete();
  void slot_updateFileMenu();
  void slot_updateEditMenu();
  void slot_undo();
  void slot_redo();
  // Слот для генерации команд изменения данных
  void slot_dataChanged(QVariant oldData, int colNum);

private:
  Ui::MainWindow*  ui;
  QTreeWidget*     p_treeWidget;
  QStack<Command*> m_redoStack;
  QStack<Command*> m_undoStack;
  QFile*           p_file;

private:
  void addDepartment();
  void addEmployee();

  void readDepartments(QXmlStreamReader&); // Считывает все отделы
  void readDepartment(QXmlStreamReader&); // Добавляет считанный из readEmployments отдел в список
  QTreeWidgetItem* readEmployments(QXmlStreamReader& reader, QTreeWidgetItem* department); // Вернёт отдел с сотрудниками
  QTreeWidgetItem* readEmployment(QXmlStreamReader& reader); // Вернёт сотрудника с данными

  void writeEmployee(QXmlStreamWriter& writer, QTreeWidgetItem* employee);
  void writeEmployees(QXmlStreamWriter& writer, QTreeWidgetItem* department);
  void writeDepartment(QXmlStreamWriter& writer, QTreeWidgetItem* department);
  void saveDocument();

};

#endif // MAINWINDOW_H
