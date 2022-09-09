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
    void output_log(QString message);
    void flush_ui_config();
    void set_current_speed(quint32 speed);
    void set_gdb_state(QString msg, bool connect);
    void set_dlink_state(QString msg, bool connect);
    void set_cpu_state(QString msg, bool connect);
    quint32 Speed_Khz;
    bool Little_Endian;
    bool Local_Host_Only;
    bool Stay_On_Top;
    bool Generate_Log_File;
    bool Verify_Download;
    bool Init_Regs_On_Start;

signals:
    void ui_connect(QString, QString, QString, QString);
    void ui_close();

private slots:
    void on_pushButton_ClearLog_clicked();
    void on_checkBox_Local_Host_Only_clicked(bool checked);
    void on_checkBox_Stay_On_Top_clicked(bool checked);
    void on_checkBox_Generate_Log_File_clicked(bool checked);
    void on_checkBox_Verify_Download_clicked(bool checked);
    void on_checkBox_Init_Regs_On_Start_clicked(bool checked);

    void on_pushButton_Connect_clicked();

private:
    Ui::MainWindow *ui;
    QFile log_file;

protected:
     void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
