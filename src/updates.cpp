#include "updates.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVersionNumber>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPointer>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

bool validPotatoVersion(const QString &v){return QRegularExpression("^[0-9]{1,5}\\.[0-9]{1,5}\\.[0-9]{1,5}$").match(v).hasMatch();}
QString potatoInstallRoot(){
    const auto args=QCoreApplication::arguments();const int i=args.indexOf("--install-root");
    return QDir::cleanPath(i>=0&&i+1<args.size()?QFileInfo(args[i+1]).absoluteFilePath():QCoreApplication::applicationDirPath());
}
bool parsePotatoRelease(const QJsonObject &j,const QString &current,PotatoRelease &r,QString &error){
    r={};error.clear();r.version=j["tag_name"].toString();if(r.version.startsWith('v'))r.version.remove(0,1);
    if(j["draft"].toBool()||j["prerelease"].toBool()||!validPotatoVersion(current)||!validPotatoVersion(r.version)){error="GitHub did not return a supported stable version.";return false;}
    r.newer=QVersionNumber::compare(QVersionNumber::fromString(r.version),QVersionNumber::fromString(current))>0;
    if(!r.newer)return true;
    const auto expected="PotatoMapper-"+r.version+"-windows-x64.zip";
    for(const auto &entry:j["assets"].toArray()){
        const auto a=entry.toObject();if(a["name"].toString()!=expected||a["state"].toString()!="uploaded")continue;
        r.download=QUrl(a["browser_download_url"].toString());const auto digest=a["digest"].toString();r.size=qint64(a["size"].toDouble());
        const auto expectedPath="/TheGamingDonKey/potato-mapper/releases/download/v"+r.version+"/"+expected;
        if(r.download.scheme()!="https"||r.download.host()!="github.com"||r.download.path()!=expectedPath||!r.download.query().isEmpty()||!r.download.fragment().isEmpty()||!r.download.userInfo().isEmpty()
            ||!QRegularExpression("^sha256:[a-fA-F0-9]{64}$").match(digest).hasMatch()||r.size<=0||r.size>512*1024*1024){error="The release download is incomplete or unsupported. Try again later or use the GitHub Releases page.";return false;}
        r.sha256=QByteArray::fromHex(digest.mid(7).toLatin1());return true;
    }
    error="This release has no Windows x64 app download yet. Try again later.";return false;
}
bool validatePotatoPayload(const QString &directory,const QString &version,QString &error){
    error.clear();if(!validPotatoVersion(version)){error="Invalid application version.";return false;}
    QFile manifest(QDir(directory).filePath("update-manifest.json"));
    if(!manifest.open(QIODevice::ReadOnly)||manifest.size()>4*1024*1024){error="This download does not contain a supported update manifest.";return false;}
    QJsonParseError parse;const auto j=QJsonDocument::fromJson(manifest.readAll(),&parse).object();
    if(parse.error!=QJsonParseError::NoError||j["format"].toString()!="PotatoMapperUpdate"||j["protocol"].toInt()!=1||j["version"].toString()!=version){error="This release needs a different launcher. Download it manually from GitHub; your installed version has not changed.";return false;}
    const auto prefix="versions/"+version+"/";const auto root=QDir(directory).canonicalPath();QSet<QString> listed;
    const auto files=j["files"].toArray();if(files.isEmpty()||files.size()>20000){error="The update file list is invalid.";return false;}
    for(const auto &entry:files){
        const auto item=entry.toObject();const auto name=item["path"].toString();const auto digest=item["sha256"].toString();
        if(!name.startsWith(prefix)||name.contains('\\')||name.contains(':')||name.split('/').contains("..")||name.split('/').contains(".")||name.contains("//")||listed.contains(name.toCaseFolded())||!QRegularExpression("^[a-fA-F0-9]{64}$").match(digest).hasMatch()){error="Invalid file list in update.";return false;}
        const QFileInfo info(QDir(directory).filePath(name));
        if(!info.isFile()||info.isSymLink()||!info.canonicalFilePath().startsWith(root+"/",Qt::CaseInsensitive)){error="A required update file is missing.";return false;}
        QFile file(info.filePath());QCryptographicHash hash(QCryptographicHash::Sha256);
        if(!file.open(QIODevice::ReadOnly)||!hash.addData(&file)||hash.result().toHex()!=digest.toLatin1().toLower()){error="An update file failed verification. Download it again.";return false;}
        listed.insert(name.toCaseFolded());
    }
    if(!listed.contains((prefix+"PotatoMapperApp.exe").toCaseFolded())||!listed.contains((prefix+"runtime-version.txt").toCaseFolded())){error="The editor is missing from the update.";return false;}
    QDirIterator it(QDir(directory).filePath("versions"),QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);int actual=0;
    while(it.hasNext()){it.next();if(it.fileInfo().isSymLink()){error="Links are not allowed in an update.";return false;}if(it.fileInfo().isFile()){++actual;if(!listed.contains(QDir(directory).relativeFilePath(it.filePath()).toCaseFolded())){error="Unexpected file in update runtime.";return false;}}}
    if(actual!=listed.size()){error="The update file count is inconsistent.";return false;}
    QFile marker(QDir(directory).filePath(prefix+"runtime-version.txt"));if(!marker.open(QIODevice::ReadOnly)||marker.readAll()!=version.toUtf8()+"\n"){error="The application version does not match its release.";return false;}
    return true;
}
struct UpdateDialog::State {
    QNetworkAccessManager *network=nullptr;QPointer<QNetworkReply> reply;QProcess *extract=nullptr;
    QLabel *status=nullptr;QProgressBar *progress=nullptr;QPushButton *install=nullptr,*retry=nullptr,*close=nullptr;
    PotatoRelease release;QString root;QFile archive;QCryptographicHash hash{QCryptographicHash::Sha256};
    std::unique_ptr<QTemporaryDir> temp;bool stopped=false,handedOff=false;qint64 received=0;
    std::function<bool()> prepareRestart;std::function<QString()> projectPath;
};
UpdateDialog::UpdateDialog(QWidget *parent,std::function<bool()> prepare,std::function<QString()> project,QNetworkAccessManager *network):QDialog(parent),state(new State){
    setWindowTitle("Potato Mapper — Updates");setAttribute(Qt::WA_DeleteOnClose);setMinimumWidth(460);setWindowModality(Qt::WindowModal);
    auto &s=*state;s.root=potatoInstallRoot();s.prepareRestart=std::move(prepare);s.projectPath=std::move(project);
    auto *layout=new QVBoxLayout(this);layout->setContentsMargins(24,24,24,24);layout->setSpacing(16);
    auto *title=new QLabel("Potato Mapper updates");title->setStyleSheet("font-size:20px;font-weight:700;color:#f6d396");layout->addWidget(title);
    layout->addWidget(new QLabel("Installed version: "+QCoreApplication::applicationVersion()));
    s.status=new QLabel("Checking GitHub for a new release…");s.status->setWordWrap(true);s.status->setTextFormat(Qt::PlainText);layout->addWidget(s.status);
    s.progress=new QProgressBar;s.progress->setRange(0,0);layout->addWidget(s.progress);
    auto *buttons=new QHBoxLayout;s.retry=new QPushButton("Check again");s.install=new QPushButton("Download update");s.close=new QPushButton("Close");s.install->setEnabled(false);s.retry->setEnabled(false);
    s.status->setObjectName("updateStatus");s.install->setObjectName("downloadUpdateButton");
    buttons->addWidget(s.retry);buttons->addStretch();buttons->addWidget(s.install);buttons->addWidget(s.close);layout->addLayout(buttons);
    s.network=network?network:new QNetworkAccessManager(this);s.extract=new QProcess(this);
    connect(s.retry,&QPushButton::clicked,this,[this]{check();});connect(s.install,&QPushButton::clicked,this,[this]{download();});connect(s.close,&QPushButton::clicked,this,&QDialog::reject);
    connect(s.extract,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){if(state->stopped)return;if(code!=0||status!=QProcess::NormalExit){fail("Could not prepare the downloaded ZIP. Your installed app has not changed. Check free space and try again.");return;}extracted();});
    connect(s.extract,&QProcess::errorOccurred,this,[this](QProcess::ProcessError e){if(!state->stopped&&e==QProcess::FailedToStart)fail("Windows could not start its ZIP extraction tool. Download and extract the release manually.");});
    check();
}
UpdateDialog::~UpdateDialog(){stop();}
void UpdateDialog::stop(){auto &s=*state;s.stopped=true;if(s.reply){s.reply->disconnect(this);s.reply->abort();}if(s.extract->state()!=QProcess::NotRunning){s.extract->disconnect(this);s.extract->kill();s.extract->waitForFinished(3000);}s.archive.close();}
void UpdateDialog::fail(const QString &message){auto &s=*state;s.status->setText(message);s.progress->hide();s.install->setEnabled(false);s.retry->setEnabled(true);s.close->setText("Close");s.archive.close();}
static QNetworkRequest request(const QUrl &url){QNetworkRequest r(url);r.setRawHeader("User-Agent","PotatoMapper/"+QCoreApplication::applicationVersion().toUtf8());r.setRawHeader("Accept","application/vnd.github+json");r.setTransferTimeout(30000);r.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);return r;}
void UpdateDialog::check(){
    auto &s=*state;s.stopped=false;s.temp.reset();s.install->setEnabled(false);s.retry->setEnabled(false);s.progress->show();s.progress->setRange(0,0);s.status->setText("Checking GitHub for a new release…");
    auto *reply=s.network->get(request(QUrl("https://api.github.com/repos/TheGamingDonKey/potato-mapper/releases/latest")));s.reply=reply;
    connect(reply,&QNetworkReply::readyRead,this,[reply]{if(reply->bytesAvailable()>2*1024*1024)reply->abort();});
    connect(reply,&QNetworkReply::finished,this,[this,reply]{
        auto &s=*state;s.reply=nullptr;reply->deleteLater();if(s.stopped)return;
        if(reply->error()!=QNetworkReply::NoError){fail("Could not check GitHub. Check your connection and try again. GitHub may also temporarily limit update checks.");return;}
        QJsonParseError parse;const auto doc=QJsonDocument::fromJson(reply->readAll(),&parse);QString error;
        if(parse.error!=QJsonParseError::NoError||!parsePotatoRelease(doc.object(),QCoreApplication::applicationVersion(),s.release,error)){fail(error.isEmpty()?"GitHub returned an invalid release response.":error);return;}
        s.progress->hide();s.retry->setEnabled(true);
        if(!s.release.newer){s.status->setText("You're up to date. No newer stable release is available.");return;}
        s.status->setText("Version "+s.release.version+" is available ("+QString::number(s.release.size/1048576.0,'f',1)+" MB). Your saved mappings and media stay in place.");
        QFile marker(QDir(s.root).filePath("installation.txt"));
        if(!marker.open(QIODevice::ReadOnly)||marker.readAll()!="PotatoMapper/1\n"||!QFileInfo::exists(QDir(s.root).filePath("PotatoMapper.exe"))){s.status->setText("A newer release is available. This copy was opened outside the portable launcher layout; download the full ZIP from GitHub to enable installation.");return;}
        s.install->setEnabled(true);
    });
}
void UpdateDialog::download(){
    auto &s=*state;s.install->setEnabled(false);s.retry->setEnabled(false);s.progress->show();s.progress->setRange(0,100);s.close->setText("Cancel download");
    const auto updates=QDir(s.root).filePath(".updates");if(!QDir().mkpath(updates)){fail("This folder is not writable. Move the complete app folder to a location you can write to, then try again.");return;}
    s.temp=std::make_unique<QTemporaryDir>(updates+"/download-XXXXXX");if(!s.temp->isValid()){fail("Could not create the update folder. Check permissions and free space.");return;}
    s.archive.setFileName(s.temp->filePath("download.zip"));if(!s.archive.open(QIODevice::WriteOnly)){fail("Could not save the update download. Check free space.");return;}
    s.hash.reset();s.received=0;s.status->setText("Downloading Potato Mapper "+s.release.version+"…");
    auto r=request(s.release.download);r.setRawHeader("Accept","application/octet-stream");auto *reply=s.network->get(r);s.reply=reply;
    connect(reply,&QNetworkReply::readyRead,this,[this,reply]{auto &s=*state;const auto bytes=reply->readAll();s.received+=bytes.size();if(s.received>s.release.size||s.archive.write(bytes)!=bytes.size()){reply->abort();return;}s.hash.addData(bytes);});
    connect(reply,&QNetworkReply::downloadProgress,this,[this](qint64 current,qint64 total){if(total>0)state->progress->setValue(int(current*100/total));});
    connect(reply,&QNetworkReply::finished,this,[this,reply]{
        auto &s=*state;s.reply=nullptr;s.archive.close();reply->deleteLater();if(s.stopped)return;
        if(reply->error()!=QNetworkReply::NoError||s.received!=s.release.size||s.hash.result()!=s.release.sha256){fail("The download was interrupted or failed verification. Your installed version has not changed. Try again.");return;}
        s.status->setText("Download verified. Preparing the update…");s.progress->setRange(0,0);s.close->setText("Cancel");
        QFile script(":/packaging/extract-update.ps1");if(!script.open(QIODevice::ReadOnly)){fail("The ZIP extraction instructions are unavailable.");return;}
        auto env=QProcessEnvironment::systemEnvironment();env.insert("POTATO_UPDATE_ZIP",s.temp->filePath("download.zip"));env.insert("POTATO_UPDATE_STAGE",s.temp->filePath("payload"));s.extract->setProcessEnvironment(env);
        s.extract->start(env.value("SystemRoot","C:/Windows")+"/System32/WindowsPowerShell/v1.0/powershell.exe",{"-NoProfile","-NonInteractive","-Command",QString::fromUtf8(script.readAll())});
    });
}
void UpdateDialog::extracted(){
    auto &s=*state;QString error;if(!validatePotatoPayload(s.temp->filePath("payload"),s.release.version,error)){fail(error);return;}
    s.progress->hide();s.close->setText("Close");s.status->setText("The update is ready. Save your work before restarting.");
    if(!s.prepareRestart()){s.status->setText("Update cancelled. Your current app and project are still open.");s.retry->setEnabled(true);return;}
    QStringList args{"--apply-update",s.temp->path(),"--version",s.release.version,"--wait-pid",QString::number(QCoreApplication::applicationPid())};
    const auto project=s.projectPath();if(!project.isEmpty())args<<"--project"<<project;
    if(!QProcess::startDetached(QDir(s.root).filePath("PotatoMapper.exe"),args,s.root)){fail("Could not start the update helper. Your app is still open.");return;}
    s.handedOff=true;s.temp->setAutoRemove(false);if(parentWidget())parentWidget()->setProperty("potatoUpdateExit",true);QCoreApplication::quit();
}
