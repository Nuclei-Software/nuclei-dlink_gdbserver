#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>

extern QQueue<QString> log_queue;

void output_message(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString msgType;
    switch(type)
    {
    case QtDebugMsg:
        msgType = QString("Debug");
        break;
    case QtInfoMsg:
        msgType = QString("Info");
        break;
    case QtWarningMsg:
        msgType = QString("Warning");
        break;
    case QtCriticalMsg:
        msgType = QString("Critical");
        break;
    case QtFatalMsg:
        msgType = QString("Fatal");
        break;
    default:
        msgType = QString("Default");
        break;
    }
    #if 0
    QString contextInfo = QString("%1 line[%2]") .arg(context.file) .arg(context.line);
    QString strTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString message = QString("%1 %2 %3: %4\n") .arg(msgType) .arg(strTime) .arg(contextInfo) .arg(msg);
    #endif
    QString message = QString("%1: %2\n") .arg(msgType) .arg(msg);
    log_queue.enqueue(message);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Local_Host_Only = false;
    Stay_On_Top = false;
    Generate_Log_File = false;
    Verify_Download = false;
    Init_Regs_On_Start = false;

    ui->lineEdit_Port->setText("3333");
    ui->lineEdit_Baud->setText("115200");
    QList<QSerialPortInfo> m_list = QSerialPortInfo::availablePorts();
    for (int i = 0;i < m_list.count();i++) {
        ui->comboBox_Serial->addItem(m_list.at(i).systemLocation());
        #if 0
        qDebug() << "Name : " << m_list.at(i).portName();
        qDebug() << "Description : " << m_list.at(i).description();
        qDebug() << "Manufacturer: " << m_list.at(i).manufacturer();
        qDebug() << "Serial Number: " << m_list.at(i).serialNumber();
        qDebug() << "System Location: " << m_list.at(i).systemLocation();
        #endif
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::output_log(QString message)
{
    ui->textBrowser->append(message);
    if (Generate_Log_File) {
        log_file.open(QFile::WriteOnly);
        log_file.write(message.toUtf8());
        log_file.close();
    }
}

void MainWindow::install_message_handler()
{
    qInstallMessageHandler(output_message);
}

void MainWindow::flush_ui_config()
{
    switch (ui->comboBox_Initial_Speed->currentIndex()) {
    case 0:
        Speed_Khz = 100 * 1000;
        break;
    case 1:
        Speed_Khz = 200 * 1000;
        break;
    case 2:
        Speed_Khz = 500 * 1000;
        break;
    case 3:
        Speed_Khz = 1000 * 1000;
        break;
    case 4:
        Speed_Khz = 2000 * 1000;
        break;
    case 5:
        Speed_Khz = 5000 * 1000;
        break;
    case 6:
        Speed_Khz = 10000 * 1000;
        break;
    case 7:
        Speed_Khz = 20000 * 1000;
        break;
    case 8:
        Speed_Khz = 50000 * 1000;
        break;
    default:
        break;
    }

    switch (ui->comboBox_Endian->currentIndex()) {
    case 0:
        Little_Endian = false;
        break;
    case 1:
        Little_Endian = true;
        break;
    default:
        break;
    }
}

void MainWindow::set_current_speed(quint32 speed)
{
    ui->lineEdit_Current_Speed->setText(QString::number(speed) + "Khz");
}

void MainWindow::set_gdb_state(QString msg, bool connect)
{
    ui->lineEdit_GDB->setText(msg);
    if (connect) {
        ui->progressBar_GDB->setValue(100);
    } else {
        ui->progressBar_GDB->setValue(0);
    }
}

void MainWindow::set_dlink_state(QString msg, bool connect)
{
    ui->lineEdit_Dlink->setText(msg);
    if (connect) {
        ui->progressBar_Dlink->setValue(100);
    } else {
        ui->progressBar_Dlink->setValue(0);
    }
}

void MainWindow::set_cpu_state(QString msg, bool connect)
{
    ui->lineEdit_CPU->setText(msg);
    if (connect) {
        ui->progressBar_CPU->setValue(100);
    } else {
        ui->progressBar_CPU->setValue(0);
    }
}

void MainWindow::on_pushButton_ClearLog_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_checkBox_Local_Host_Only_clicked(bool checked)
{
    if (checked) {
        Local_Host_Only = true;
    } else {
        Local_Host_Only = false;
    }
}

void MainWindow::on_checkBox_Stay_On_Top_clicked(bool checked)
{
    if (checked) {
        Stay_On_Top = true;
    } else {
        Stay_On_Top = false;
    }
}

void MainWindow::on_checkBox_Generate_Log_File_clicked(bool checked)
{
    if (checked) {
        log_file.setFileName(QFileDialog::getSaveFileName());
        log_file.open(QFile::WriteOnly);
        log_file.close();
        if (log_file.exists()) {
            Generate_Log_File = true;
        } else {
            Generate_Log_File = false;
            ui->checkBox_Generate_Log_File->setCheckState(Qt::CheckState::Unchecked);
        }
    } else {
        Generate_Log_File = false;
    }
}

void MainWindow::on_checkBox_Verify_Download_clicked(bool checked)
{
    if (checked) {
        Verify_Download = true;
    } else {
        Verify_Download = false;
    }
}

void MainWindow::on_checkBox_Init_Regs_On_Start_clicked(bool checked)
{
    if (checked) {
        Init_Regs_On_Start = true;
    } else {
        Init_Regs_On_Start = false;
    }
}

void MainWindow::on_pushButton_Connect_clicked()
{
    ui->pushButton_Connect->setDisabled(true);
    emit ui_connect(ui->lineEdit_Port->text(),
                    ui->comboBox_Serial->currentText(),
                    ui->lineEdit_Baud->text(),
                    ui->comboBox_Interface->currentText());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << event;
    emit ui_close();
}
