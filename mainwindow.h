#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegularExpression>
#include <QSqlRelationalTableModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct BandInfo
{
    QString band;
    QString mode;
};

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
    QTcpSocket* socket;
    Ui::MainWindow *ui;
    QRegularExpression re;
    QSqlRelationalTableModel spotModel;
    QString findDxccCountry(const QString& dxCall, const QString& ctyFilePath);
    BandInfo frequencyToBandAndMode(const QString &freqkHz);
};
#endif // MAINWINDOW_H
