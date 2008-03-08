/*****************************************************************************
 * stardict.cpp - QStarDict, a StarDict clone written with using Qt          *
 * Copyright (C) 2008 Alexander Rodin                                        *
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

#include "stardict.h"

#include <list>
#include <map>
#include <string>
#include <utility>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include "lib.h"
#include "file.hpp"

namespace
{
std::string xdxf2html(const char *p);
std::string parse_data(const char *data);
QString html2text(QString html);
QString whereDict(const QString &name, const QStringList &dictDirs);
const int MaxFuzzy = 24;

class StdList: public std::list<std::string>
{
    public:
        StdList()
            : std::list<std::string>()
        { }

        StdList(const QList<QString> &list)
            : std::list<std::string>()
        {
            for (QList<QString>::const_iterator i = list.begin(); i != list.end(); ++i)
                push_back(i->toUtf8().data());
        }

        StdList(const std::list<std::string> &list)
            : std::list<std::string>(list)
        { }

        QStringList toStringList() const
        {
            QStringList list;
            for (const_iterator i = begin(); i != end(); ++i)
                list << QString::fromUtf8(i->c_str());
            return list;
        }
};

class IfoListSetter
{
    public:
        IfoListSetter(QStringList *list)
            : m_list(list)
        { }

        void operator ()(const std::string &filename, bool)
        {
            DictInfo info;
            if (info.load_from_ifo_file(filename, false))
                m_list->push_back(QString::fromUtf8(info.bookname.c_str()));
        }

    private:
        QStringList *m_list;
};

class IfoFileFinder
{
    public:
        IfoFileFinder(const QString &name)
            : m_name(name.toUtf8().data())
        { }

        void operator()(const std::string &filename, bool)
        {
            DictInfo info;
            if (info.load_from_ifo_file(filename, false) && info.bookname == m_name)
                m_filename = filename;
        }

        QString filename()
        { return QString::fromUtf8(m_filename.c_str()); }
    private:
        std::string m_name;
        std::string m_filename;
};
}

StarDict::StarDict(QObject *parent)
    : QObject(parent)
{
    m_sdLibs = new Libs;
    QSettings settings(workPath() + "/settings.ini", QSettings::IniFormat);
    m_dictDirs = settings.value("StarDict/dictDirs", m_dictDirs).toStringList();
    if (m_dictDirs.isEmpty())
    {
#ifdef Q_OS_UNIX
        m_dictDirs << "/usr/share/stardict/dic";
        m_dictDirs << QDir::homePath() + "/.stardict/dic";
#else
        m_dictDirs << QCoreApplication::applicationDirPath() + "/dic";
#endif // Q_OS_UNIX
        m_dictDirs << workPath() + "/dicts";
    }
}

StarDict::~StarDict()
{
    QSettings settings(workPath() + "/settings.ini", QSettings::IniFormat);
    settings.setValue("StarDict/dictDirs", m_dictDirs);
    delete m_sdLibs;
}

QStringList StarDict::avialableDicts() const
{
    QStringList result;
    IfoListSetter setter(&result);
    for_each_file(StdList(m_dictDirs), ".ifo", StdList(), StdList(), setter);

    return result;
}

void StarDict::setLoadedDicts(const QStringList &loadedDicts)
{
    m_sdLibs->reload(StdList(m_dictDirs), StdList(loadedDicts), StdList());

    m_loadedDicts.clear();
    for (int i = 0; i < m_sdLibs->ndicts(); ++i)
        m_loadedDicts[QString::fromUtf8(m_sdLibs->dict_name(i).c_str())] = i;
}

StarDict::DictInfo StarDict::dictInfo(const QString &dict)
{
    ::DictInfo nativeInfo;
    if (! nativeInfo.load_from_ifo_file(whereDict(dict, StarDict::m_dictDirs).toUtf8().data(), false))
        return DictInfo();
    DictInfo result(name(), dict);
    result.setFullName(QString::fromUtf8(nativeInfo.bookname.c_str()));
    result.setAuthor(QString::fromUtf8(nativeInfo.author.c_str()));
    result.setEmail(QString::fromUtf8(nativeInfo.email.c_str()));
    result.setWebSite(QString::fromUtf8(nativeInfo.website.c_str()));
    result.setDescription(QString::fromUtf8(nativeInfo.description.c_str()));
    result.setWordsCount(static_cast<long>(nativeInfo.wordcount));
    return result;
}

bool StarDict::isTranslatable(const QString &dict, const QString &word)
{
    if (! m_loadedDicts.contains(dict))
        return false;
    long ind;
    return m_sdLibs->SimpleLookupWord(word.toUtf8().data(), ind, m_loadedDicts[dict]);
}

StarDict::Translation StarDict::translate(const QString &dict, const QString &word)
{
    if (! m_loadedDicts.contains(dict))
        return Translation();
    if (word.isEmpty())
        return Translation();
    int dictIndex = m_loadedDicts[dict];
    long ind;
    if (! m_sdLibs->SimpleLookupWord(word.toUtf8().data(), ind, m_loadedDicts[dict]))
        return Translation();
    return Translation(QString::fromUtf8(m_sdLibs->poGetWord(ind, dictIndex)),
            QString::fromUtf8(m_sdLibs->dict_name(dictIndex).c_str()),
            QString::fromUtf8(parse_data(m_sdLibs->poGetWordData(ind, dictIndex)).c_str()));
}

QStringList StarDict::findSimilarWords(const QString &word)
{
    gchar *fuzzy_res[MaxFuzzy];
    if (! m_sdLibs->LookupWithFuzzy(word.toUtf8().data(), fuzzy_res, MaxFuzzy))
        return QStringList();
    QStringList result;
    for (gchar **p = fuzzy_res, **end = fuzzy_res + MaxFuzzy; p != end && *p; ++p)
    {
        result << QString::fromUtf8(*p);
        g_free(*p);
    }
    return result;
}

void StarDict::execSettingsDialog()
{
    
}

namespace
{
QString whereDict(const QString &name, const QStringList &dictDirs)
{
    IfoFileFinder finder(name);
    for_each_file(StdList(dictDirs), ".ifo", StdList(), StdList(), finder);
    return finder.filename();
}

std::string xdxf2html(const char *p) // taken from sdcv
{
    std::string res;
    for (; *p; ++p)
    {
        if (*p != '<')
        {
            if (*p == '\n')
                res += "<br>";
            else
                res += *p;
            continue;
        }

        const char *next = strchr(p, '>');
        if (!next)
            continue;

        std::string name(p + 1, next - p - 1);

        if (name == "abr")
            res += "<font class=\"abbreviature\">";
        else if (name == "/abr")
            res += "</font>";
        else if (name == "k")
        {
            const char *begin = next;
            if ((next = strstr(begin, "</k>")) != 0)
                next += sizeof("</k>") - 1 - 1;
            else
                next = begin;
        }
        else if (name == "tr")
            res += "<font class=\"transcription\">[";
        else if (name == "/tr")
            res += "]</font>";
        else if (name == "ex")
            res += "<font class=\"example\">";
        else if (name == "/ex")
            res += "</font>";
        else if (!name.empty() && name[0] == 'c' && name != "co")
        {
            std::string::size_type pos = name.find("code");
            if (pos != std::string::size_type( -1))
            {
                pos += sizeof("code=\"") - 1;
                std::string::size_type end_pos = name.find("\"");
                std::string color(name, pos, end_pos - pos);
                res += "";
            }
            else
            {
                res += "";
            }
        }
        else if (name == "/c")
            res += "";

        p = next;
    }
    return res;
}

std::string parse_data(const char *data) // taken from sdcv
{
    using std::string;

    if (!data)
        return "";

    string res;
    guint32 data_size, sec_size = 0;
    gchar *m_str;
    const gchar *p = data;
    data_size = *((guint32 *)p);
    p += sizeof(guint32);
    while (guint32(p - data) < data_size)
    {
        switch (*p++)
        {
        case 'm':
        case 'l':  //need more work...
        case 'g':
            sec_size = strlen(p);
            if (sec_size)
            {
                res += "\n";
                m_str = g_strndup(p, sec_size);
                res += m_str;
                g_free(m_str);
            }
            ++sec_size;
            break;
        case 'x':
            sec_size = strlen(p);
            if (sec_size)
            {
                res += "\n";
                m_str = g_strndup(p, sec_size);
                res += xdxf2html(m_str);
                g_free(m_str);
            }
            ++sec_size;
            break;
        case 't':
            sec_size = strlen(p);
            if (sec_size)
            {
                res += "\n";
                m_str = g_strndup(p, sec_size);
                res += "[" + string(m_str) + "]";
                g_free(m_str);
            }
            ++sec_size;
            break;
        case 'y':
            sec_size = strlen(p);
            ++sec_size;
            break;
        case 'W':
        case 'P':
            sec_size = *((guint32 *)p);
            sec_size += sizeof(guint32);
            break;
        }
        p += sec_size;
    }
    return res;
}

QString html2text(QString html)
{
    return html.replace("<br>", "\n").
                remove(QRegExp("<.*>")).
                replace("&lt;", "<").
                replace("&gt;", ">").
                replace("&amp;", "&").
                replace("&quot;", "\"");
}

}

Q_EXPORT_PLUGIN2(stardict, StarDict)

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab cindent textwidth=120 formatoptions=tc

