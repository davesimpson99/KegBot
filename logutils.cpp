#include "logutils.h"

#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <iostream>

namespace LOGUTILS
{
  static QString logFileName;

  void initLogFileName()
  {
    logFileName = QString(logFolderName + "/Log_%1__%2.txt")
                  .arg(QDate::currentDate().toString("yyyy_MM_dd"))
                  .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
  }

  /**
   * @brief deletes old log files, only the last ones are kept
   */
  void deleteOldLogs()
  {
    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Time | QDir::Reversed);
    dir.setPath(logFolderName);

    QFileInfoList list = dir.entryInfoList();
    if (list.size() <= LOGFILES)
    {
      return; //no files to delete
    } else
    {
      for (int i = 0; i < (list.size() - LOGFILES); i++)
      {
        QString path = list.at(i).absoluteFilePath();
        QFile file(path);
        file.remove();
      }
    }
  }

  bool initLogging()
  {
      // Create folder for logfiles if not exists
      if(!QDir(logFolderName).exists())
      {
        QDir().mkdir(logFolderName);
      }

      deleteOldLogs(); //delete old log files
      initLogFileName(); //create the logfile name

      QFile outFile(logFileName);
      if(outFile.open(QIODevice::WriteOnly | QIODevice::Append))
      {
        qInstallMessageHandler(LOGUTILS::myMessageHandler);
        return true;
      }
      else
      {
        return false;
      }
  }

  void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString& txt)
  {
    //check file size and if needed create new log!
    {
      QFile outFileCheck(logFileName);
      int size = outFileCheck.size();

      if (size > LOGSIZE) //check current log size
      {
        deleteOldLogs();
        initLogFileName();
      }
    }

    QFile outFile(logFileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
  }
}
