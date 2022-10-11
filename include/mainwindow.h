#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtDebug>
#include <QDateTime>
#include <QQueue>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void install_message_handler();

signals:
    void Connect(QString);
    void Close();

private slots:
    void on_toolButton_LoaderPath_clicked();
    void on_pushButton_Connect_clicked();
    void on_pushButton_ClearLog_clicked();

private:
    Ui::MainWindow *ui;

public slots:
    void output_log(QString message);

protected:
     void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
