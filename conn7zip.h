#ifndef CONN7ZIP_H
#define CONN7ZIP_H

#include <QProcess>


class Conn7zip : QObject
{
    Q_OBJECT

public:
    Conn7zip();
    ~Conn7zip();

    // Uruchamiają dodatkowe gui
    int packIt(std::string path, std::string outFile = std::string(), std::string pass = std::string());
    int packIt(std::list<std::string> path, std::string outFile = std::string(), std::string pass = std::string());
    int unpackIt(std::string inFile, std::string outPath = std::string(), std::string pass = std::string());

    bool supportedExt(std::list<std::string> &out); // 7z.dll

    bool filelist(std::string inFile, std::list<std::string> &out); // from package

protected slots:
    void process_finished(int,QProcess::ExitStatus);

private:
    QProcess *mProc;

};

#endif // CONN7ZIP_H
