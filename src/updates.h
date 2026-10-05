#pragma once
#include <QDialog>
#include <QJsonObject>
#include <QUrl>
#include <QStringList>
#include <functional>
#include <memory>

struct PotatoRelease {
    QString version;
    QUrl download;
    QByteArray sha256;
    qint64 size=0;
    bool newer=false;
};
bool parsePotatoRelease(const QJsonObject &json,const QString &current,PotatoRelease &release,QString &error);
bool validatePotatoPayload(const QString &directory,const QString &version,QString &error);
bool validPotatoVersion(const QString &version);
QString potatoInstallRoot();
QString resolvePotatoInstallRoot(const QString &applicationDirectory,const QStringList &arguments);
class QNetworkAccessManager;

class UpdateDialog : public QDialog {
public:
    explicit UpdateDialog(QWidget *parent,std::function<bool()> prepareRestart,std::function<QString()> projectPath,QNetworkAccessManager *network=nullptr);
    ~UpdateDialog() override;
private:
    struct State;
    std::unique_ptr<State> state;
    void check();
    void download();
    void extracted();
    void fail(const QString &message);
    void stop();
};
