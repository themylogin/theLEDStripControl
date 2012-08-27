#include <QApplication>
#include "Dialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w("192.168.0.4");
    w.show();
    
    return a.exec();
}
