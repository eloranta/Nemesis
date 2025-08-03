#include "MainWindow.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QMessageBox>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this))
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    logEdit = new QTextEdit();
    logEdit->setReadOnly(true);

    mainLayout->addWidget(logEdit);
    setCentralWidget(central);

    connect(socket, &QTcpSocket::connected, this, &MainWindow::connected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::socketError);
    connectToServer();
}

MainWindow::~MainWindow() {}

void MainWindow::connectToServer()
{
    logEdit->append("Connecting...");
    socket->connectToHost("ham.connect.fi", 7300);
    socket->waitForConnected(5000);
}

void MainWindow::connected()
{
    socket->write("og3z\r\n");
}

void MainWindow::readData()
{
    QString line = socket->readAll();

    QRegularExpression re(R"(DX de (\w+):\s+(\d+\.\d+)\s+(\w+)\s+(.+?)\s+(\d{4})Z\x07\x07\r\n)");
    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        QString spotter = match.captured(1);
        QString frequency = match.captured(2);
        QString dxCall = match.captured(3);
        QString comment = match.captured(4);
        QString time = match.captured(5);

        qDebug() << "Spotter:" << spotter;
        qDebug() << "Frequency:" << frequency;
        qDebug() << "DX Station:" << dxCall;
        qDebug() << "Comment:" << comment;
        qDebug() << "Time:" << time;
    }
 }

void MainWindow::socketError(QAbstractSocket::SocketError)
{
    logEdit->append("Socket error: " + socket->errorString());
}
