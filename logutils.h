#ifndef LOGUTILS_H
#define LOGUTILS_H

#define LOGSIZE 1024 * 100 //log size in bytes
#define LOGFILES 5

#include <QObject>
#include <QString>
#include <QDebug>
#include <QDate>
#include <QTime>


namespace LOGUTILS
{
const QString logFolderName = "logs";

bool initLogging();
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString& msg);

}

#endif // LOGUTILS_H
