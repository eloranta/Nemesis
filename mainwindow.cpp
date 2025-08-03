#include "MainWindow.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QMessageBox>

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
    QString data = socket->readAll();
    if (data.startsWith("DX de "))
        logEdit->append(data);
}

void MainWindow::socketError(QAbstractSocket::SocketError)
{
    logEdit->append("Socket error: " + socket->errorString());
}
