#include "MainWindow.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFile>
#include <QApplication>

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
        QString dxcc = findDxccCountry(dxCall, qApp->applicationDirPath() + "/cty.dat");

        qDebug() << time << dxCall << dxcc<< frequency << spotter << comment;
    }
 }

void MainWindow::socketError(QAbstractSocket::SocketError)
{
    logEdit->append("Socket error: " + socket->errorString());
}

QString MainWindow::findDxccCountry(const QString& dxCall, const QString& ctyFilePath)
{
    QFile file(ctyFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Unknown";

    QTextStream in(&file);
    QString currentCountry;
    bool expectingPrefixes = false;

    while (!in.atEnd()) {
        QString line = in.readLine();

        // Skip empty lines or comments
        if (line.isEmpty() || line.startsWith("#")) continue;

        if (!line.startsWith(' ') && !line.startsWith('\t')) {
            // Country header line
            QStringList fields = line.split(':', Qt::SkipEmptyParts);
            if (fields.size() >= 1) {
                currentCountry = fields[0].trimmed();
                expectingPrefixes = true;
            }
        } else if (expectingPrefixes) {
            // Prefixes line (starts with whitespace)
            line = line.trimmed();
            QStringList prefixes = line.split(',', Qt::SkipEmptyParts);
            //qDebug() << currentCountry << prefixes;

            for (QString prefix : prefixes) {
                prefix = prefix.trimmed();
                if (prefix.isEmpty()) continue;

                if (prefix.endsWith("*"))
                {
                    QString base = prefix.left(prefix.length() - 1);
                    if (dxCall.toUpper().startsWith(base.toUpper()))
                        return currentCountry;
                }
                else
                {
                    //qDebug() << dxCall << prefix;
                    if (dxCall.toUpper().startsWith(prefix.toUpper()))
                        return currentCountry;
                }
            }
            expectingPrefixes = false; // Move to next block
        }
    }

    return "Unknown";
}
