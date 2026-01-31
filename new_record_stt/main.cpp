#include "new_record_stt.h"
#include <QtWidgets/QApplication>
#include <QThread>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    new_record_stt w;
    w.show();

    return a.exec();
}
