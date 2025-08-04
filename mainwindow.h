#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void connectToServer();
    void connected();
    void readData();
    void socketError(QAbstractSocket::SocketError socketError);
private:
    Ui::MainWindow *ui;
    QTcpSocket* socket;
    QString findDxccCountry(const QString& dxCall, const QString& ctyFilePath);
    QString band(const QString& frequency);
    QRegularExpression re;
};
#endif // MAINWINDOW_H
