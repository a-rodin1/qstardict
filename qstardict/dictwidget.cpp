/*****************************************************************************
 * dictwidget.cpp - QStarDict, a StarDict clone written with using Qt        *
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

#include "dictwidget.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include "dictbrowser.h"

namespace QStarDict
{

DictWidget::DictWidget(QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
{
    m_translationView = new DictBrowser(this);
    setFrameStyle(m_translationView->frameStyle());
    m_translationView->setFrameStyle(QFrame::NoFrame);
    m_translationView->verticalScrollBar()->setCursor(Qt::ArrowCursor);
    m_translationView->horizontalScrollBar()->setCursor(Qt::ArrowCursor);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_translationView);
    setLayout(layout);
}

void DictWidget::translate(const QString &str)
{
    m_translationView->setSource(str);
    emit wordTranslated(str);
}

QString DictWidget::translatedWord() const
{
    return m_translationView->source().toString();
}

void DictWidget::clear()
{
    m_translationView->clear();
}

QString DictWidget::cssStyle()
{
    return DictBrowser::cssStyle();
}

void DictWidget::setDict(DictCore *dict)
{
    m_translationView->setDict(dict);
}

const DictCore* DictWidget::dict() const
{
    return m_translationView->dict();
}

}

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab cindent textwidth=120 formatoptions=tc

