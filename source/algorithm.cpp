#include "../include/algorithm.h"

Algorithm::Algorithm(QObject *parent)
    : QObject{parent}
{

}

void Algorithm::AddFlash(flash_t flash)
{
    flashs.append(flash);
}

void Algorithm::AddWorkarea(workarea_t workarea)
{
    workareas.append(workarea);
}
