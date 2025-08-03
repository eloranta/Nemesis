#pragma once

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegularExpression>

class QLineEdit;
class QTextEdit;
class QPushButton;

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

    QLineEdit* ipEdit;
    QLineEdit* portEdit;
    QPushButton* connectButton;
    QTextEdit* logEdit;
    QString findDxccCountry(const QString& dxCall, const QString& ctyFilePath);
    QString band(const QString& frequency);

    QRegularExpression re;
};
