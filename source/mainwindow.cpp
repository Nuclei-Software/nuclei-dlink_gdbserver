#include "../include/mainwindow.h"
#include "ui_mainwindow.h"

QQueue<QString> log_queue;

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

    ui->lineEdit_LoaderPath->setText("/* Please click right button select dlink gdb server config file. */");
    ui->textBrowser_Logout->document()->setMaximumBlockCount(10000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::output_log(QString message)
{
    ui->textBrowser_Logout->append(message);
    ui->textBrowser_Logout->moveCursor(QTextCursor::End);
}

void MainWindow::install_message_handler()
{
    qInstallMessageHandler(output_message);
}

void MainWindow::on_toolButton_LoaderPath_clicked()
{
    ui->lineEdit_LoaderPath->setText(QFileDialog::getOpenFileName());
}

void MainWindow::on_pushButton_Connect_clicked()
{
    if (QFile::exists(ui->lineEdit_LoaderPath->text())) {
        ui->pushButton_Connect->setDisabled(true);
        emit Connect(ui->lineEdit_LoaderPath->text());
    } else {
        output_log("Not found config file!");
    }
}

void MainWindow::on_pushButton_ClearLog_clicked()
{
    ui->textBrowser_Logout->clear();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << event;
    emit Close();
}
