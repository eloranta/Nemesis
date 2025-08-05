#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QApplication>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this)), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->spotView->setModel(&spotModel);

    QSqlQuery query;

    query.exec(QString("create table if not exists spots ("
                       "Id integer primary key autoincrement,"
                       "Call text,"
                       "Country text,"
                       "Mode text,"
                       "Band text,"
                       "Message text)"));

    connect(socket, &QTcpSocket::connected, this, &MainWindow::connected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::socketError);
    connectToServer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToServer()
{
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

    re.setPattern(R"(DX de (\w+):\s+(\d+\.\d+)\s+(\w+)\s+(.+?)\s+(\d{4})Z\x07\x07\r\n)");
    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        QString spotter = match.captured(1);
        QString frequency = match.captured(2);
        QString dxCall = match.captured(3);
        QString comment = match.captured(4);
        QString time = match.captured(5);
        QString dxcc = findDxccCountry(dxCall, qApp->applicationDirPath() + "/cty.dat");
        BandInfo info = frequencyToBandAndMode(frequency);
        qDebug() << time << dxCall << dxcc << frequency << info.band << info.mode << spotter << comment;
    }
}

void MainWindow::socketError(QAbstractSocket::SocketError)
{
    qDebug() << "Socket error: " << socket->errorString();
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


BandInfo MainWindow::frequencyToBandAndMode(const QString &freqkHz)
{

    double freq = freqkHz.toDouble();

    if (freq >= 1810 && freq <= 2000) {
        if (freq < 1830) return {"160m", "CW"};
        if (freq >= 1838 && freq <= 1843) return {"160m", "Digital"};
        if (freq >= 1840 && freq <= 2000) return {"160m", "SSB"};
        return {"160m", "Mixed"};
    }

    if (freq >= 3500 && freq <= 4000) {
        if (freq < 3560) return {"80m", "CW"};
        if (freq >= 3570 && freq <= 3600) return {"80m", "Digital"};
        if (freq >= 3600 && freq <= 4000) return {"80m", "SSB"};
        return {"80m", "Mixed"};
    }

    if (freq >= 7000 && freq <= 7300) {
        if (freq < 7040) return {"40m", "CW"};
        if (freq >= 7040 && freq <= 7074) return {"40m", "Digital"};
        if (freq >= 7074 && freq <= 7150) return {"40m", "SSB"};
        return {"40m", "Mixed"};
    }

    if (freq >= 10000 && freq <= 10150) {
        if (freq < 10130) return {"30m", "CW"};
        if (freq >= 10130 && freq <= 10150) return {"30m", "Digital"};
        return {"30m", "Mixed"};
    }

    if (freq >= 14000 && freq <= 14350) {
        if (freq < 14070) return {"20m", "CW"};
        if (freq >= 14070 && freq <= 14090) return {"20m", "Digital"};
        if (freq >= 14100 && freq <= 14300) return {"20m", "SSB"};
        return {"20m", "Mixed"};
    }

    if (freq >= 18068 && freq <= 18100) {
        if (freq < 14070) return {"17m", "CW"};
        if (freq >= 18100 && freq <= 18110) return {"17m", "Digital"};
        if (freq >= 18110 && freq <= 18168) return {"17m", "SSB"};
        return {"17m", "Mixed"};
    }

    if (freq >= 21000 && freq <= 21450) {
        if (freq < 21070) return {"15m", "CW"};
        if (freq >= 21070 && freq <= 21100) return {"15m", "Digital"};
        if (freq >= 21200) return {"15m", "SSB"};
        return {"15m", "Mixed"};
    }

    if (freq >= 24890 && freq <= 24990) {
        if (freq < 24915) return {"12m", "CW"};
        if (freq >= 24915 && freq <= 24925) return {"12m", "Digital"};
        if (freq >= 24940 && freq <= 24990) return {"12m", "SSB"};
        return {"12m", "Mixed"};
    }

    if (freq >= 28000 && freq <= 29700) {
        if (freq < 28070) return {"10m", "CW"};
        if (freq >= 28070 && freq <= 28120) return {"10m", "Digital"};
        if (freq >= 28120) return {"10m", "SSB"};
        return {"10m", "Mixed"};
    }

    if (freq >= 50000 && freq <= 52000) {
        if (freq < 50100) return {"6m", "CW"};
        if (freq >= 50313 && freq <= 50313) return {"6m", "Digital"};
        return {"6m", "Mixed"};
    }

    if (freq >= 144000 && freq <= 146000) {
        if (freq < 144100) return {"2m", "CW"};
        if (freq >= 144100 && freq <= 144150) return {"2m", "Digital"};
        if (freq >= 144150) return {"2m", "SSB"};
        return {"2m", "Mixed"};
    }

//     // Add other bands similarly...

    return {"Unknown", "Unknown"};
}


