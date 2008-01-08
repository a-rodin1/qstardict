/*****************************************************************************
 * mainwindow.h - QStarDict, a StarDict clone written with using Qt          *
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QSystemTrayIcon>

class QMenu;
class QCloseEvent;

class DictCore;
class PopupWindow;
class SettingsDialog;

/**
 * The main window of QStarDict.
 */
class MainWindow: public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

    public:
        /**
         * Create new MainWindow.
         */
        MainWindow(QWidget *parent = 0);
        /**
         * Destructor.
         */
        ~MainWindow();

        /**
         * Return popup window.
         */
        PopupWindow* popupWindow()
        { return popup; }

        /**
         * Return true if instant search is on, otherwise false.
         */
        bool isInstantSearch() const
        { return m_instantSearch; }
        /**
         * Set instant search mode. If instantSearch is true
         * translation will be shown when typing, otherwise only when
         * "Search" button clicked.
         */
        void setInstantSearch(bool instantSearch);

    public slots:
        /**
         * Show translation of text.
         */
        void showTranslation(const QString &text);

        /**
         * Return translation of text in plain text format.
         */
        QString translate(const QString &text) const;
        /**
         * Return translation of text in HTML format.
         */
        QString translateHtml(const QString &text) const;

    protected:
        void closeEvent(QCloseEvent *event);

    private slots:
        void on_actionAbout_triggered();
        void on_actionSettings_triggered();
        void on_actionSaveToFile_triggered();
        void on_queryButton_clicked();

        void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
        void wordsListItemActivated(QListWidgetItem *item);
        void wordTranslated(const QString &word);

    private:
        friend class SettingsDialog;

        void createTrayIcon();
        void createConnections();
        void loadSettings();
        void saveSettings();

        DictCore *m_dict;
        QSystemTrayIcon *trayIcon;
        QMenu *trayMenu;
        PopupWindow *popup;
        bool m_instantSearch;
};

#endif // MAINWINDOW_H

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab cindent textwidth=120 formatoptions=tc

