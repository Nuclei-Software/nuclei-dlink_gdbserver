#include "../include/logout.h"

extern QQueue<QString> log_queue;

Logout::Logout(QObject *parent)
    : QThread{parent}
{

}

void Logout::run()
{
    close_flag = false;
    while (1) {
        if (close_flag) {
            break;
        }
        if (!log_queue.empty()) {
            emit Toui(log_queue.dequeue());
        }
    }
}

void Logout::Close()
{
    close_flag = true;
}
