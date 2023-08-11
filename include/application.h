#ifndef APPLICATION_H
#define APPLICATION_H

#include <QFile>
#include <QObject>
#include "mainwindow.h"
#include "logout.h"
#include "transmit.h"

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);

    void Init(int argc, char *argv[]);

private:
    MainWindow* mainwindow;
    Logout* logout;
    Transmit* transmit;

public slots:
    void Connect(QString cfg_path);
    void Disconnect();
};

#endif // APPLICATION_H
