// Copyright (c) 2023-2025 Manuel Schneider

#include "items.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <QWidget>
#include <albert/albert.h>
#include <albert/standarditem.h>
#include <albert/widgetsutil.h>
using namespace albert;
using namespace std;
using namespace util;

QString tr_copy = Plugin::tr("Copy to clipboard");
QString tr_copy_with_placeholder = Plugin::tr("Copy '%1' to clipboard");

Plugin::Plugin():
    items({make_shared<DateTimeItem>(),
           make_shared<DateItem>(),
           make_shared<EpochItem>(),
           make_shared<TimeItem>(),
           make_shared<UtcItem>()})

{
    restore_show_date_on_empty_query(settings());
}

QString Plugin::synopsis(const QString &query) const
{
    if (query.isEmpty())
        return QStringLiteral("%1 | %2 | unix | utc | <number>")
            .arg(tr("Date").toLower(), tr("Time").toLower());
    return {};
}


vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> r;

    for (const auto &i : items)
        if (i->subtext().startsWith(query, Qt::CaseInsensitive))
            r.emplace_back(i, (double)query.string().size() / i->subtext().size());


    bool isNumber;
    const ulong unixtime = query.string().toULong(&isNumber);
    if (isNumber)
    {
        const auto s = QLocale().toString(QDateTime::fromSecsSinceEpoch(unixtime), QLocale::LongFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("u2dt"),
                s,
                tr("Date and time from unix time"),
                DateTimeItem::icon_urls,
                {{ QStringLiteral("c"), tr_copy,
                  [=]{ setClipboardText(s); }}}
                ),
            0.
        );
    }

    return r;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery()
{
    if (show_date_on_empty_query())
        return { items[0] };
    return {};
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    bind(ui.checkBox_emptyQuery,
         this,
         &Plugin::show_date_on_empty_query,
         &Plugin::set_show_date_on_empty_query,
         &Plugin::show_date_on_empty_query_changed);

    return w;
}
