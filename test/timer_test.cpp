#include "plugin.h"
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QThreadPool>
#include <albert/app.h>
#include <albert/pluginloader.h>
#include <albert/pluginmetadata.h>
#include <albert/querycontext.h>
#include <albert/rankitem.h>
#include <albert/usagescoring.h>
#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

// Minimal app, PluginInstance::settings() requires albert::app(). Registers itself as
// albert::app() on construction.
class FakeApp : public App
{
public:

    void show(const QString &) override {}
    void showSettings(QString) override {}
    bool localizationEnabled() const override { return false; }
    const map<QString, Extension*> &extensions() const override { return extensions_; }

    unique_ptr<QSettings> settings() override
    { return make_unique<QSettings>(dir_.filePath(u"config"_s), QSettings::IniFormat); }

    unique_ptr<QSettings> state() override
    { return make_unique<QSettings>(dir_.filePath(u"state"_s), QSettings::IniFormat); }

    const filesystem::path &cacheLocation() override { return path_; }
    const filesystem::path &configLocation() override { return path_; }
    const filesystem::path &dataLocation() override { return path_; }

private:

    QTemporaryDir dir_;
    const filesystem::path path_{dir_.path().toStdString()};
    const map<QString, Extension*> extensions_;
};

// Minimal plugin loader to construct the Plugin outside of Albert
class FakePluginLoader : public PluginLoader
{
public:

    FakePluginLoader() { metadata_.id = u"datetime"_s; }

    QString path() const override { return {}; }
    const PluginMetadata &metadata() const override { return metadata_; }
    void load() override {}
    void unload() override {}
    PluginInstance *instance() override { return nullptr; }

private:

    PluginMetadata metadata_{};
};

// Minimal query context to call Plugin::rankItems outside of Albert
class FakeQueryContext : public QueryContext
{
public:

    FakeQueryContext(const QueryHandler &handler, const QString &query) :
        handler_(handler),
        query_(query)
    {}

    bool isValid() const override { return true; }
    const QueryHandler &handler() const override { return handler_; }
    QString trigger() const override { return {}; }
    QString query() const override { return query_; }
    const UsageScoring &usageScoring() const override { return usage_scoring_; }

private:

    const QueryHandler &handler_;
    const QString query_;
    const UsageScoring usage_scoring_{
        false, 1.0, make_shared<const unordered_map<ItemKey, double>>()};
};

class DateTimeTests : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void items_released_in_foreign_thread_delete_later();

private:

    FakeApp app;
    FakePluginLoader loader;
    unique_ptr<Plugin> plugin;
};

void DateTimeTests::initTestCase()
{
    // PluginInstance takes its loader from current_loader on construction
    PluginLoader::current_loader = &loader;
    plugin = make_unique<Plugin>();
    PluginLoader::current_loader = nullptr;
}

// Deleting an item outside its thread leaves its timer registered. This
// test case checks for regressions of this issue. 
// (See https://github.com/albertlauncher/albert/issues/1633)
void DateTimeTests::items_released_in_foreign_thread_delete_later()
{
    const int max_thread_pool_wait_ms = 5000;
    QTest::failOnWarning("QObject::~QObject: Timers cannot be stopped from another thread");

    // Create the items in a worker thread, like a global query does. The empty query matches all
    // date time items.
    vector<RankItem> items;
    QThreadPool::globalInstance()->start([&]{
        FakeQueryContext ctx(*plugin, u""_s);
        items = plugin->rankItems(ctx);
    });
    QVERIFY(QThreadPool::globalInstance()->waitForDone(max_thread_pool_wait_ms));
    QVERIFY(!items.empty());

    // Copy the pointers so we can check the timers for each pointer after the items have been
    // deleted
    vector<QObject*> item_pointers;
    for (const RankItem &rank_item : items)
    {
        QObject *item = dynamic_cast<QObject*>(rank_item.item.get());
        QVERIFY(item);
        item_pointers.push_back(item);
    }

    // Wait until the main thread starts the timers
    QAbstractEventDispatcher *dispatcher = QAbstractEventDispatcher::instance(qApp->thread());
    QTRY_VERIFY(ranges::all_of(item_pointers, [&](QObject *o){
        return !dispatcher->timersForObject(o).empty();
    }));

    // Release the last references in a pool thread, like a cancelled global query does
    QThreadPool::globalInstance()->start([&]{ items.clear(); });
    QVERIFY(QThreadPool::globalInstance()->waitForDone(max_thread_pool_wait_ms));

    // Ensure timers are unregistered, otherwise they will fire on dangling pointers. Only compares
    // pointers, does not dereference them.
    QTRY_VERIFY(ranges::all_of(item_pointers, [&](QObject *o){
        return dispatcher->timersForObject(o).empty();
    }));
}

QTEST_GUILESS_MAIN(DateTimeTests)
#include "timer_test.moc"
