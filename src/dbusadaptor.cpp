/********************************************************************************
 * dbusadaptor.cpp - QStarDict, a StarDict clone written with using Qt          *
 * Copyright (C) 2007 Alexander Rodin                                           *
 *                                                                              *
 * This program is free software: you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by         *
 * the Free Software Foundation, either version 3 of the License, or            *
 * (at your option) any later version.                                          *
 *                                                                              *
 * This program is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
 * GNU General Public License for more details.                                 *
 *                                                                              *
 * You should have received a copy of the GNU General Public License            *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
 ********************************************************************************/

#include "dbusadaptor.h"

#include <QDBusConnection>
#include "mainwindow.h"
#include "popupwindow.h"

DBusAdaptor::DBusAdaptor(MainWindow *mainWindow)
    : QDBusAbstractAdaptor(mainWindow), m_mainWindow(mainWindow)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.qstardict.qstardict");
    connection.registerObject("/qstardict", mainWindow);
}

bool DBusAdaptor::mainWindowVisible() const
{
    return m_mainWindow->isVisible();
}

void DBusAdaptor::setMainWindowVisible(bool visible)
{
    m_mainWindow->setVisible(visible);
}

void DBusAdaptor::showTranslation(const QString &text)
{
    m_mainWindow->showTranslation(text);
}

void DBusAdaptor::showPopup(const QString &text)
{
    m_mainWindow->popupWindow()->showTranslation(text);
}

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab cindent

