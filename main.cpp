#include "mainwindow.h"
#include <QApplication>
#include <QtSql>

extern bool openDatabase(const QString& name)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(name);
    return db.open();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (!openDatabase("Alarm.sqlite"))
    {
        qDebug() << "Cannot open database";
        return -1;
    }
    MainWindow window;
    window.show();
    return app.exec();
}
