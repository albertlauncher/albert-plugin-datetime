// Copyright (c) 2023-2025 Manuel Schneider

#include "items.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <albert/matcher.h>
#include <albert/widgetsutil.h>
#include <QSettings>
using namespace albert;
using namespace std;

static const auto ck_show_date_on_empty_query = "show_date_on_empty_query";

Plugin::Plugin():
    show_date_on_empty_query_(settings()->value(ck_show_date_on_empty_query, false).value<bool>())
{}

QString Plugin::synopsis(const QString &q) const
{
    return q.isEmpty() ? QStringLiteral(u"<\u2009filter\u2009|\u2009epoch\u2009>") : QString{};
}

template<typename T>
static void addItem(vector<RankItem>& items, const Matcher &matcher)
{
    if (const auto m = matcher.match(T::trName()); m)
    {
        auto item = make_shared<T>();
        item->moveToThread(qApp->thread());
        items.emplace_back(::move(item), m);
    }
}

vector<RankItem> Plugin::rankItems(QueryContext &ctx)
{
    vector<RankItem> r;
    Matcher matcher(ctx);

    addItem<DateItem>(r, matcher);
    addItem<TimeItem>(r, matcher);
    addItem<DateTimeItem>(r, matcher);
    addItem<UtcItem>(r, matcher);

    bool isNumber;
    const ulong unixtime = ctx.query().toULong(&isNumber);
    if (isNumber)
        r.emplace_back(makeFromEpochItem(unixtime), 0.);

    return r;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery()
{
    if (showDateOnEmptyQuery())
        return { make_shared<DateTimeItem>() };
    return {};
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    bindWidget(ui.checkBox_emptyQuery, this, &Plugin::showDateOnEmptyQuery, &Plugin::setShowDateOnEmptyQuery);

    return w;
}

bool Plugin::showDateOnEmptyQuery() const { return show_date_on_empty_query_; }

void Plugin::setShowDateOnEmptyQuery(bool v)
{
    if (show_date_on_empty_query_ != v)
    {
        settings()->setValue(ck_show_date_on_empty_query, v);
        show_date_on_empty_query_ = v;
    }
}
