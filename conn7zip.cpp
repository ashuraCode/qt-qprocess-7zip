#include "conn7zip.h"
#include <QFile>
#include <QDebug>


Conn7zip::Conn7zip(): QObject(Q_NULLPTR), mProc(new QProcess)
{
    connect(mProc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(process_finished(int,QProcess::ExitStatus)));
}

Conn7zip::~Conn7zip()
{
    if (mProc!=NULL)
        delete mProc;
}

int Conn7zip::packIt(std::string path, std::string outFile, std::string pass)
{
    if (!mProc->isOpen()) {
        QStringList arguments = QStringList{"-r", "a", outFile.c_str(),
                QString("-p")+pass.c_str(), "-aoa", "-ad", path.c_str()};
        mProc->start("7zg.exe", arguments, QIODevice::ReadWrite);
    }

    if (mProc->waitForStarted() == false) {
        qDebug() << "Error starting 7zg.exe process";
        qDebug() << mProc->errorString();
        return -1;
    }

    return 0;
}

int Conn7zip::packIt(std::list<std::string> path, std::string outFile, std::string pass)
{
    QFile file("listfile.txt");
    if(!file.open(QFile::ReadWrite))
        return -1;

    for (auto &s: path) {
        file.write(s.c_str());
        file.write("\r\n");
    }

    if (!mProc->isOpen()) {
        QStringList arguments = QStringList{"a",// QString("-o")+,
                QString("-p")+pass.c_str(), "-ad", outFile.c_str(), "@listfile.txt"};
        mProc->start("7zg.exe", arguments, QIODevice::ReadWrite);
    }

    if (mProc->waitForStarted() == false) {
        qDebug() << "Error starting 7zg.exe process";
        qDebug() << mProc->errorString();
        return -1;
    }

    return 0;
}

int Conn7zip::unpackIt(std::string inFile, std::string outPath, std::string pass)
{
    if (!mProc->isOpen()) {
        QStringList arguments = QStringList{"-r", "x", QString("-o")+outPath.c_str(),
                QString("-p")+pass.c_str(), "-aoa", "-ad", inFile.c_str()};
        mProc->start("7zg.exe", arguments, QIODevice::ReadWrite);
    }

    if (mProc->waitForStarted() == false) {
        qDebug() << "Error starting 7zg.exe process";
        qDebug() << mProc->errorString();
        return -1;
    }

    return 0;
}

bool Conn7zip::supportedExt(std::list<std::string> &out)
{
    QProcess proc;

    proc.start("7z.exe", QStringList {"i"}, QIODevice::ReadWrite);

    if (proc.waitForStarted() == false) {
        qDebug() << "Error starting 7z.exe process";
        qDebug() << mProc->errorString();
        return false;
    }

    proc.waitForFinished();

    QStringList all = QString(proc.readAllStandardOutput()).split('\n');

    while (all.size()>0) {
        if (all.front().mid(0,8).compare("Formats:", Qt::CaseInsensitive)==0) break;
        all.pop_front();
    }

    if (all.size()>0) {
        all.pop_front();
        for (auto &s: all) {
            QString extp = s.mid(26);
            extp = extp.mid(0, extp.indexOf("  "));
            extp = extp.mid(0, extp.indexOf(QRegExp(" [a-zA-Z0-9] ")));
            extp = extp.mid(0, extp.indexOf(QRegExp(" [a-zA-Z0-9][0-9a-fA-F] ")));
            if (extp.isEmpty()) break;

            for (auto &o: extp.split(' ')) {
                if (o.size() < 5)
                    out.push_back(o.toStdString());
            }
        }
    }

    out.sort();

    return true;
}

bool Conn7zip::filelist(std::string inFile, std::list<std::string> &out)
{
    QProcess proc;

    proc.start("7z.exe", QStringList {"l", inFile.c_str()}, QIODevice::ReadWrite);

    if (proc.waitForStarted() == false) {
        qDebug() << "Error starting 7z.exe process";
        qDebug() << mProc->errorString();
        return false;
    }

    proc.waitForFinished();

    QStringList all = QString(proc.readAllStandardOutput()).split('\n');

    while (all.size()>0) {
        if (all.front().mid(0,3).compare("---", Qt::CaseInsensitive)==0) break;
        all.pop_front();
    } if (all.size()>0) all.pop_front();

    while (all.size()>0) {
        if (all.back().mid(0,3).compare("---", Qt::CaseInsensitive)==0) break;
        all.pop_back();
    } if (all.size()>0) all.pop_back();

    if (all.size()>0) {
        for (auto &s: all) {
            QString extp = s.mid(52);
            extp = extp.trimmed();
            out.push_back(extp.toStdString());
        }
    }

    return true;
}

void Conn7zip::process_finished(int, QProcess::ExitStatus)
{
    mProc->close();
    if (QFile("listfile.txt").exists())
        QFile::remove("listfile.txt");
}
