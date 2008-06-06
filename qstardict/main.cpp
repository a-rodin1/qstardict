/*****************************************************************************
 * main.cpp - QStarDict, a StarDict clone written with using Qt              *
 * Copyright (C) 2007 Alexander Rodin                                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.               *
 *****************************************************************************/

#include "application.h"

#ifdef Q_OS_UNIX
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <unistd.h>
#elif defined(Q_OS_WIN) // Q_OS_UNIX
#include <windows.h>
#include <QMessageBox>
#endif // Q_OS_WIN

#ifdef QSTARDICT_WITH_TRANSLATIONS
#include <QLocale>
#include <QTranslator>
#endif // QSTARDICT_WITH_TRANSLATIONS

int main(int argc, char *argv[])
{
#ifdef Q_OS_UNIX
    QFile lockFile(QDir::homePath() + "/.qstardict/qstardict.pid");
    lockFile.open(QIODevice::ReadOnly);
    QTextStream lockStream(&lockFile);
    QString oldPid = lockStream.readLine();
    QString oldTime = lockStream.readLine();
    if (oldPid.length() && QDir("/proc/" + oldPid).exists() &&
       oldTime == QFileInfo("/proc/" + oldPid).created().toString())
    {
        qWarning("qstardict: Already running");
        return 0;
    }
    lockFile.close();
    lockFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    lockStream << getpid() << endl
               << QFileInfo("/proc/" + QString::number(getpid())).created().toString() << endl;
    lockFile.close();
#elif defined(Q_OS_WIN) // Q_OS_UNIX
    int hMutex = CreateMutex(NULL, true, "qstardict");
    if (GetLastError == ERROR_ALREADY_EXISTS)
    {
        QMessageBox::warning(0, tr("Warning"), tr("Another instance of QStarDict is already running"));
        return 0;
    }
#endif // Q_OS_WIN

    QStarDict::Application app(argc, argv);
    return app.exec();
}

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab cindent textwidth=120 formatoptions=tc

