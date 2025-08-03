#include "MainWindow.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QMessageBox>
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

QString MainWindow::band(const QString& freq)
{
    const int freqHz = freq.toDouble() * 1000.0;
    if (freqHz >= 1800000 && freqHz <= 2000000)    return "160m";
    if (freqHz >= 3500000 && freqHz <= 4000000)    return "80m";
    if (freqHz >= 5330500 && freqHz <= 5403500)    return "60m";
    if (freqHz >= 7000000 && freqHz <= 7300000)    return "40m";
    if (freqHz >= 10100000 && freqHz <= 10150000)  return "30m";
    if (freqHz >= 14000000 && freqHz <= 14350000)  return "20m";
    if (freqHz >= 18068000 && freqHz <= 18168000)  return "17m";
    if (freqHz >= 21000000 && freqHz <= 21450000)  return "15m";
    if (freqHz >= 24890000 && freqHz <= 24990000)  return "12m";
    if (freqHz >= 28000000 && freqHz <= 29700000)  return "10m";
    if (freqHz >= 50000000 && freqHz <= 54000000)  return "6m";
    if (freqHz >= 70000000 && freqHz <= 71000000)  return "4m";
    if (freqHz >= 144000000 && freqHz <= 148000000) return "2m";
    if (freqHz >= 222000000 && freqHz <= 225000000) return "1.25m";
    if (freqHz >= 420000000 && freqHz <= 450000000) return "70cm";
    if (freqHz >= 902000000 && freqHz <= 928000000) return "33cm";
    if (freqHz >= 1240000000 && freqHz <= 1300000000) return "23cm";

    return "Unknown";
}

void MainWindow::readData()
{
    QString line = socket->readAll();

    re.setPattern(R"(DX de (\w+):\s+(\d+\.\d+)\s+(\w+)\s+(.+?)\s+(\d{4})Z\x07\x07\r\n)");
    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        QString spotter = match.captured(1);
        QString frequency = match.captured(2);
        QString dxCall = match.captured(3);
        QString comment = match.captured(4);
        QString time = match.captured(5);
        QString dxcc = findDxccCountry(dxCall, qApp->applicationDirPath() + "/cty.dat");

        qDebug() << time << dxCall << dxcc<< band(frequency) << spotter << comment;
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

    while (!in.atEnd()) {
        QString line = in.readLine();

        // Skip empty lines or comments
        if (line.isEmpty() || line.startsWith("#")) continue;

        if (!line.startsWith(' ') && !line.startsWith('\t')) {
            // Country header line
            QStringList fields = line.split(':', Qt::SkipEmptyParts);
            if (fields.size() >= 1) {
                currentCountry = fields[0].trimmed();
            }
        } else {
            // Prefixes line (starts with whitespace)
            line = line.trimmed();
            const QStringList prefixes = line.split(',', Qt::SkipEmptyParts);
            //qDebug() << currentCountry << prefixes;

            for (QString prefix : prefixes) {
                prefix = prefix.trimmed();
                if (prefix.isEmpty()) continue;

                if (prefix.endsWith("*") || prefix.endsWith(";"))
                {
                    QString base = prefix.left(prefix.length() - 1);
                    if (dxCall.startsWith(base))
                        return currentCountry;
                }
                else
                {
                    if (dxCall.startsWith(prefix))
                        return currentCountry;
                }
            }
        }
    }

    return "Unknown";
}
