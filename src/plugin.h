// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();

    QWidget *buildConfigWidget() override;
    QString synopsis(const QString &) const override;
    std::vector<albert::RankItem> rankItems(albert::QueryContext &) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

    bool showDateOnEmptyQuery() const;
    void setShowDateOnEmptyQuery(bool);

private:

    bool show_date_on_empty_query_;

};
