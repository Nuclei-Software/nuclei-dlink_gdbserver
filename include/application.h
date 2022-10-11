#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "../include/mainwindow.h"
#include "../include/logout.h"
#include "../include/transmit.h"
#include "../include/target.h"
#include "../include/server.h"

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    void ApplicationInit(int argc, char *argv[]);

signals:

private:
    MainWindow* mainwindow;
    Logout* logout;
    Transmit* transmit;
    Target* target;
    Server* server;

public slots:
    void ApplicationConnect(QString cfg_path);
};

#endif // APPLICATION_H
