// Copyright (c) 2023-2025 Manuel Schneider

#include "items.h"
#include <QApplication>
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <QWidget>
#include <albert/albert.h>
#include <albert/query.h>
#include <albert/standarditem.h>
using namespace albert;
using namespace std;

QStringList DateTimeItemBase::icon_urls = {":datetime"};
extern QString tr_copy;
extern QString tr_copy_with_placeholder;


DateTimeItemBase::DateTimeItemBase(const QString &id, const QString &text, const QString &subtext):
    id_(id),
    text_(text),
    subtext_(subtext)
{ startTimer(1s); }

void DateTimeItemBase::addObserver(Observer *observer) { observers.insert(observer); }

void DateTimeItemBase::removeObserver(Observer *observer) { observers.erase(observer); }

QString DateTimeItemBase::id() const { return id_; }

QString DateTimeItemBase::text() const { return text_; }

QString DateTimeItemBase::subtext() const { return subtext_; }

QStringList DateTimeItemBase::iconUrls() const { return icon_urls; }

QString DateTimeItemBase::inputActionText() const { return subtext_; }

vector<Action> DateTimeItemBase::actions() const
{
    return {{ QStringLiteral("c"), tr_copy, [this]{ setClipboardText(text()); } }};
}

void DateTimeItemBase::timerEvent(QTimerEvent*)
{
    if (auto t = makeText();
        text_ != t)
    {
        text_ = t;
        for (auto observer : observers)
            observer->notify(this);
    }
}

// ------------------------------------------------------------------------------------------------

DateItem::DateItem() : DateTimeItemBase("d", makeText(), DateTimeItemBase::tr("Date")) {}

QString DateItem::makeText()
{
    // return QLocale().toString(QDateTime::currentDateTime().date(), QLocale::ShortFormat);
    return QLocale().toString(QDateTime::currentDateTime().date(), QLocale::LongFormat);
}

// ------------------------------------------------------------------------------------------------

EpochItem::EpochItem() : DateTimeItemBase("e", makeText(), DateTimeItemBase::tr("Unix time")) {}

QString EpochItem::makeText() { return QString::number(QDateTime::currentSecsSinceEpoch()); }

// ------------------------------------------------------------------------------------------------

TimeItem::TimeItem() : DateTimeItemBase("t", makeText(), DateTimeItemBase::tr("Time")) {}

QString TimeItem::makeText()
{
    // return QLocale().toString(QDateTime::currentDateTime().time(), QLocale::ShortFormat);
    return QDateTime::currentDateTime().time().toString("hh:mm:ss");
}

// ------------------------------------------------------------------------------------------------

UtcItem::UtcItem() : DateTimeItemBase("u", makeText(), DateTimeItemBase::tr("UTC date and time")) {}

QString UtcItem::makeText()
{
    return QLocale().toString(QDateTime::currentDateTimeUtc(), QLocale::ShortFormat);
}

// ------------------------------------------------------------------------------------------------

DateTimeItem::DateTimeItem() : DateTimeItemBase("dt", makeText(), DateTimeItemBase::tr("Date and time")) {}

QString DateTimeItem::makeText()
{
    return QLocale().toString(QDateTime::currentDateTime(), QLocale::LongFormat);
}
