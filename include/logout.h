#ifndef LOGOUT_H
#define LOGOUT_H

#include <QThread>
#include <QObject>
#include <QQueue>

class Logout : public QThread
{
    Q_OBJECT
public:
    explicit Logout(QObject *parent = nullptr);

protected:
    void run() override;

private:
    bool close_flag;

signals:
    void Toui(QString);

public slots:
    void Close();
};

#endif // LOGOUT_H
