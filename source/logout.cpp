#include "../include/logout.h"

extern QQueue<QString> log_queue;

Logout::Logout(QObject *parent)
    : QThread{parent}
{

}

void Logout::run()
{
    QString msg;
    close_flag = false;
    while (1) {
        if (close_flag) {
            break;
        }
        if (!log_queue.empty()) {
            msg = log_queue.dequeue();
            if (msg.isEmpty() == false) {
                emit Toui(msg);
            }
        }
    }
}

void Logout::Close()
{
    close_flag = true;
}
