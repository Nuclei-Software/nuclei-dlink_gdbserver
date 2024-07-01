#ifndef APPLICATION_H
#define APPLICATION_H

#include <QFile>
#include <QObject>
#include "logout.h"
#include "transmit.h"
#include "target.h"
#include "server.h"

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);

    void Init(int argc, char *argv[]);

private:
    Logout* logout;
    Transmit* transmit;

public slots:
    void Connect(QString cfg_path, unsigned int port);
    void Disconnect();
};

#endif // APPLICATION_H
