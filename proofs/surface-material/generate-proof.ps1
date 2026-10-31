param([Parameter(Mandatory)][string]$Destination)
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
[IO.Directory]::CreateDirectory($Destination)|Out-Null
function Read-Tagged([string]$path){
    $lines=& git -C $repo show "v0.8.8:$path"
    if($LASTEXITCODE -ne 0){throw "Missing immutable baseline $path"}
    $lines -join "`n"
}
$main=Read-Tagged 'src/main.cpp'
function Replace-Once([string]$old,[string]$new){
    if(!$script:main.Contains($old)){throw "Missing baseline anchor: $old"}
    $script:main=$script:main.Replace($old,$new)
}
Replace-Once '#include "model.h"' "#include `"model.h`"`n#include `"frost-menu.h`"`n#include `"proof-check.h`""
$main=$main.Replace('#include "../tests/','#include "'+$repo.Replace('\','/')+'/tests/')
Replace-Once 'app.setApplicationName("Potato Mapper");app.setOrganizationName("PotatoMapper");' 'app.setApplicationName("Potato Mapper Surface Proof");app.setOrganizationName("PotatoMapperProof");'
$diagnostic='const bool diagnostic=std::any_of(args.begin(),args.end(),[](const QString &a){return a.startsWith("--smoke")||a.startsWith("--test-")||a=="--profile-media"||a=="--profile-fx"||a=="--check-project";});'
Replace-Once $diagnostic 'const bool diagnostic=true; // Proof never performs installed-app maintenance.'
Replace-Once 'MainWindow window;window.show();' 'MainWindow window;window.smoke=true;window.show();'
$main=$main.Replace('setWindowTitle("Potato Mapper")','setWindowTitle("Potato Mapper — Surface material PROOF")')
$main=$main.Replace('setWindowTitle("Potato Mapper"+','setWindowTitle("Potato Mapper — Surface material PROOF"+')
$oldDelete='if(scene.current()){scene.checkpoint();scene.surfaces.removeAt(scene.selected);scene.selected=std::min(scene.selected,int(scene.surfaces.size())-1);scene.touch(true);}'
Replace-Once $oldDelete 'deleteSurfaceForProof();'
$start=$main.IndexOf('    void surfaceMenu(int index,const QPoint &pos){')
$end=$main.IndexOf('    bool prepareRestart(){',$start)
if($start -lt 0 -or $end -lt $start){throw 'Missing menu boundaries'}
$menu=@'
    void deleteSurfaceForProof(){
        if(!scene.current())return;
        scene.checkpoint();scene.surfaces.removeAt(scene.selected);
        scene.selected=std::min(scene.selected,int(scene.surfaces.size())-1);scene.touch(true);
    }
    void surfaceMenu(int index,const QPoint &pos){
        if(index<0||index>=scene.surfaces.size())return;
        scene.select(index);MaterialSurfaceMenu menu(this);
        auto *title=new QWidgetAction(&menu);title->setText(scene.current()->name);title->setObjectName("proofSurfaceTarget");
        auto *targetLabel=new QLabel(scene.current()->name);targetLabel->setAccessibleName("Surface: "+scene.current()->name);
        targetLabel->setContentsMargins(17,6,17,7);targetLabel->setStyleSheet("background:transparent;color:#e4e7ec;font:600 13px 'Segoe UI';");
        title->setDefaultWidget(targetLabel);title->setEnabled(false);menu.addAction(title);
        menu.addAction("Load media…",this,[this]{loadMedia();});
        const auto *s=scene.current();auto *clear=menu.addAction("Clear media — white grid",this,[this]{scene.clearMedia(scene.selected);});clear->setEnabled(s&&(!s->media.isEmpty()||s->pattern));
        menu.addSeparator();menu.addAction("Rename surface…",this,[this]{renameSelected();});
        menu.addSeparator();auto *remove=menu.addAction("Delete surface",this,[this]{deleteSurfaceForProof();});
        remove->setObjectName("proofDeleteSurface");
        menu.exec(pos);
    }
'@
$main=$main.Substring(0,$start)+$menu+"`n"+$main.Substring($end)
$main=[regex]::Replace($main,'(?m)^#include "[^"\r\n]*/tests/[^"\r\n]+"\r?\n','')
$entry=$main.IndexOf('int main(int argc,char **argv){')
if($entry -lt 0){throw 'Missing main entry'}
$proofMain=@'
int main(int argc,char **argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapInterval(1);format.setDepthBufferSize(0);format.setStencilBufferSize(8);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);app.setApplicationName("Potato Mapper Surface Proof");app.setOrganizationName("PotatoMapperProof");
    app.setApplicationVersion(POTATO_VERSION);app.setWindowIcon(QIcon(":/assets/potato-mapper.png"));
    QDir().mkpath(potatoInstallRoot());qInstallMessageHandler(logMessage);
    app.setStyle("Fusion");app.setStyleSheet(potatoStyle());
    MainWindow window;window.smoke=true;window.show();
    runSurfaceMaterialProof(window,app,app.arguments());
    return app.exec();
}
'@
$main=$main.Substring(0,$entry)+$proofMain
[IO.File]::WriteAllText((Join-Path $Destination 'legacy-main.cpp'),$main,[Text.UTF8Encoding]::new($false))
foreach($file in @('style.h','ui.h','dynamic-ui.h')){[IO.File]::WriteAllText((Join-Path $Destination $file),(Read-Tagged "src/$file"),[Text.UTF8Encoding]::new($false))}
