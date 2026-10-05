#include "updates.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QTimer>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QPointer>
#include <QLockFile>
#include <iostream>

// Exercise the actual dialog with deterministic responses and the real ZIP
// extractor. No GitHub account, user project, or installed app is touched.
class FixtureReply : public QNetworkReply {
    QByteArray body;
    qint64 position=0;
public:
    FixtureReply(const QNetworkRequest &request,QByteArray data,bool interrupted,QObject *parent):QNetworkReply(parent),body(std::move(data)){
        setRequest(request);setUrl(request.url());setOperation(QNetworkAccessManager::GetOperation);open(QIODevice::ReadOnly);
        QTimer::singleShot(20,this,[this,interrupted]{
            if(isFinished())return;
            if(interrupted){body=body.left(body.size()/2);setError(QNetworkReply::RemoteHostClosedError,"Interrupted fixture download");}
            emit readyRead();emit downloadProgress(body.size(),body.size());setFinished(true);emit finished();
        });
    }
    void abort() override {if(!isFinished()){setError(OperationCanceledError,"Cancelled");setFinished(true);emit finished();}}
    qint64 bytesAvailable() const override {return body.size()-position+QIODevice::bytesAvailable();}
    qint64 readData(char *target,qint64 max) override {const auto n=qMin(max,qint64(body.size())-position);if(n<=0)return -1;memcpy(target,body.constData()+position,size_t(n));position+=n;return n;}
};
class FixtureNetwork : public QNetworkAccessManager {
public:
    QByteArray metadata,zip;
    bool interrupted=false;
    using QNetworkAccessManager::QNetworkAccessManager;
protected:
    QNetworkReply *createRequest(Operation,const QNetworkRequest &r,QIODevice*) override {
        const bool release=r.url().host()=="api.github.com";
        return new FixtureReply(r,release?metadata:zip,!release&&interrupted,this);
    }
};
static bool until(const std::function<bool()> &ready,int timeout=20000){
    QElapsedTimer timer;timer.start();while(timer.elapsed()<timeout){QApplication::processEvents();QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);if(ready())return true;QThread::msleep(10);}return false;
}
static QByteArray bytes(const QString &path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return {};return f.readAll();}
static void write(const QString &path,const QByteArray &data){QDir().mkpath(QFileInfo(path).absolutePath());QFile f(path);if(!f.open(QIODevice::WriteOnly)||f.write(data)!=data.size())throw std::runtime_error("Could not create fixture");}
int main(int argc,char **argv){
    QApplication app(argc,argv);app.setApplicationVersion("0.4.0");
    const auto args=app.arguments();if(args.size()<3){std::cerr<<"Usage: PotatoUpdateChecks.exe <release.zip> <extracted-package> --install-root <disposable-root>\n";return 2;}
    auto check=[](bool pass,const char *label){std::cout<<(pass?"PASS ":"FAIL ")<<label<<std::endl;if(!pass)throw std::runtime_error(label);};
    try{
        const auto zip=bytes(args[1]);check(!zip.isEmpty(),"read packaged ZIP");
        const auto package=QFileInfo(args[2]).absoluteFilePath();
        const auto version=QString::fromUtf8(bytes(QDir(package).filePath("current.txt"))).trimmed();
        const auto current=app.applicationVersion();const auto currentBytes=current.toUtf8()+"\n";
        const auto name="PotatoMapper-"+version+"-windows-x64.zip";
        QJsonObject asset{{"name",name},{"state","uploaded"},{"size",double(zip.size())},{"digest","sha256:"+QString(QCryptographicHash::hash(zip,QCryptographicHash::Sha256).toHex())},{"browser_download_url","https://github.com/TheGamingDonKey/potato-mapper/releases/download/v"+version+"/"+name}};
        QJsonObject release{{"tag_name","v"+version},{"assets",QJsonArray{asset}}};PotatoRelease parsed;QString error;
        check(parsePotatoRelease(release,current,parsed,error)&&parsed.newer,"stable newer release and matching asset");
        check(parsePotatoRelease(release,version,parsed,error)&&!parsed.newer,"current version needs no install");
        auto broken=release;broken["assets"]=QJsonArray{};check(!parsePotatoRelease(broken,current,parsed,error),"missing platform download rejected");
        asset["digest"]="";broken["assets"]=QJsonArray{asset};check(!parsePotatoRelease(broken,current,parsed,error),"missing download digest rejected");
        const bool validPackage=validatePotatoPayload(package,version,error);if(!validPackage)std::cerr<<error.toStdString()<<std::endl;
        check(validPackage,"actual packaged runtime manifest and hashes");
        QTemporaryDir invalid;write(invalid.filePath("update-manifest.json"),bytes(QDir(args[2]).filePath("update-manifest.json")));
        check(!validatePotatoPayload(invalid.path(),version,error),"incomplete installation rejected");
        const auto root=potatoInstallRoot();check(root!=QCoreApplication::applicationDirPath(),"disposable install root supplied");
        write(QDir(root).filePath("installation.txt"),"PotatoMapper/1\n");
        const bool handoff=args.contains("--handoff");
        if(handoff){const int launcher=args.indexOf("--launcher");const auto source=launcher>=0&&launcher+1<args.size()?args[launcher+1]:QDir(args[2]).filePath("PotatoMapper.exe");check(QFile::copy(source,QDir(root).filePath("PotatoMapper.exe")),"copy real launcher");}
        else write(QDir(root).filePath("PotatoMapper.exe"),"fixture launcher");
        write(QDir(root).filePath("current.txt"),currentBytes);write(QDir(root).filePath("My mapping.pmap"),"{\"format\":\"PotatoMapper\",\"version\":1,\"width\":1280,\"height\":720,\"surfaces\":[]}");write(QDir(root).filePath("media/clip.mp4"),"personal media fixture");
        const auto before=bytes(QDir(root).filePath("My mapping.pmap")),media=bytes(QDir(root).filePath("media/clip.mp4"));
        if(handoff){
            QLockFile lock(QDir(root).filePath(".potato-runtime.lock"));check(lock.tryLock(),"editor process lock acquired");
            FixtureNetwork network;network.metadata=QJsonDocument(release).toJson();network.zip=zip;
            QPointer<UpdateDialog> dialog=new UpdateDialog(nullptr,[]{std::cout<<"HANDOFF restart approved"<<std::endl;return true;},[&]{return QDir(root).filePath("My mapping.pmap");},&network);dialog->show();
            QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{if(!dialog){timer.stop();return;}auto *b=dialog->findChild<QPushButton*>("downloadUpdateButton");if(b&&b->isEnabled()){timer.stop();std::cout<<"HANDOFF download begins"<<std::endl;b->click();}});timer.start(20);
            QTimer::singleShot(30000,&app,[]{QCoreApplication::exit(2);});
            const int code=app.exec();if(dialog)delete dialog.data();std::cout<<"HANDOFF editor exits "<<code<<std::endl;return code;
        }
        auto run=[&](bool interrupted){
            FixtureNetwork network;network.metadata=QJsonDocument(release).toJson();network.zip=zip;network.interrupted=interrupted;bool prepared=false;
            auto *dialog=new UpdateDialog(nullptr,[&]{prepared=true;return false;},[]{return QString();},&network);QPointer<UpdateDialog> safe=dialog;
            auto *download=dialog->findChild<QPushButton*>("downloadUpdateButton");auto *status=dialog->findChild<QLabel*>("updateStatus");
            check(until([&]{return download->isEnabled();}),"dialog offers download");download->click();
            check(until([&]{return interrupted?status->text().contains("interrupted"):prepared;}),interrupted?"interrupted download leaves app running":"real extraction reaches restart confirmation");
            if(!interrupted)check(status->text().contains("cancelled"),"cancelled restart leaves current project open");
            dialog->reject();check(until([&]{return safe.isNull();}),"closing dialog cancels and releases staging");
            check(bytes(QDir(root).filePath("current.txt"))==currentBytes&&bytes(QDir(root).filePath("My mapping.pmap"))==before&&bytes(QDir(root).filePath("media/clip.mp4"))==media,"selected version, project and media unchanged");
            check(QDir(QDir(root).filePath(".updates")).entryList(QDir::Dirs|QDir::NoDotAndDotDot).isEmpty(),"temporary download folder removed");
        };
        run(true);run(false);return 0;
    }catch(const std::exception &e){std::cerr<<e.what()<<std::endl;return 2;}
}
