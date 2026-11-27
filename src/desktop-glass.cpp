#include "desktop-glass.h"
#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLWidget>
#include <QOperatingSystemVersion>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <QWindow>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#endif

namespace PotatoDesktopGlass {
namespace {
#ifdef Q_OS_WIN
HRESULT setClientAlpha(HWND hwnd,bool enabled){
    // Starting with 24H2 the GDI redirection bitmap is opaque by default.
    // Qt's translucent raster backing store already contains premultiplied BGRA.
    if(QOperatingSystemVersion::current().microVersion()<26100)return S_OK;
    const BOOL alpha=enabled;
    return DwmSetWindowAttribute(hwnd,DWMWA_REDIRECTIONBITMAP_ALPHA,&alpha,sizeof(alpha));
}

HRESULT setMaterial(HWND hwnd,DWM_SYSTEMBACKDROP_TYPE type){
    // Extend the left frame across the raster client while retaining the native caption.
    const MARGINS margins{type==DWMSBT_TRANSIENTWINDOW?65536:0,0,0,0};
    const auto frame=DwmExtendFrameIntoClientArea(hwnd,&margins);
    const auto material=DwmSetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&type,sizeof(type));
    return FAILED(frame)?frame:material;
}

HRESULT setClearClient(HWND hwnd,bool enabled){
    DWM_BLURBEHIND client{};
    client.dwFlags=DWM_BB_ENABLE;
    client.fEnable=enabled;
    if(enabled){
        client.dwFlags|=DWM_BB_BLURREGION;
        client.hRgnBlur=CreateRectRgn(0,0,-1,-1);
        if(!client.hRgnBlur)return E_OUTOFMEMORY;
    }
    const auto result=DwmEnableBlurBehindWindow(hwnd,&client);
    if(client.hRgnBlur)DeleteObject(client.hRgnBlur);
    return result;
}

class Backdrop final:public QObject,public QAbstractNativeEventFilter {
public:
    Backdrop(QWidget *editor,QOpenGLWidget *canvas):QObject(editor),m_editor(editor),m_canvas(canvas){
        setObjectName("potatoDesktopGlassAdapter");
        qApp->installNativeEventFilter(this);
        editor->installEventFilter(this);
    }
    ~Backdrop()override{
        if(qApp)qApp->removeNativeEventFilter(this);
    }

    void sync(){
        HIGHCONTRASTW contrast{sizeof(HIGHCONTRASTW),0,nullptr};
        const bool highContrast=SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(contrast),&contrast,0)&&(contrast.dwFlags&HCF_HIGHCONTRASTON);
        QSettings system("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
        const bool systemAllowed=!highContrast&&system.value("EnableTransparency",1).toInt()!=0;
        const bool requested=systemAllowed&&qApp->property("potatoInterfaceTheme").toString()=="lumenatelier1"&&
            !qApp->property("potatoReducedTransparency").toBool()&&!qApp->property("potatoIncreasedContrast").toBool();
        const bool frost=requested&&qApp->property("potatoLumenWindowsFrost").toBool();
        m_editor->setProperty("potatoDesktopGlassSystemAllowed",systemAllowed);
        const auto hwnd=reinterpret_cast<HWND>(m_editor->internalWinId());
        if(!hwnd||!IsWindow(hwnd))return;
        if(m_hwnd==hwnd&&m_requested==requested&&m_frost==frost&&!m_force)return;
        if(m_hwnd!=hwnd){m_darkOverridden=false;m_clearEnabled=false;}
        m_hwnd=hwnd;m_requested=requested;m_frost=frost;m_force=false;

        if(m_clearEnabled){setClearClient(hwnd,false);m_clearEnabled=false;}
        const HRESULT alpha=setClientAlpha(hwnd,requested);
        const auto type=frost?DWMSBT_TRANSIENTWINDOW:(requested?DWMSBT_NONE:DWMSBT_AUTO);
        const HRESULT material=setMaterial(hwnd,type);
        DWM_SYSTEMBACKDROP_TYPE actual=DWMSBT_AUTO;
        HRESULT query=DwmGetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&actual,sizeof(actual));
        const bool acrylic=frost&&SUCCEEDED(alpha)&&SUCCEEDED(material)&&SUCCEEDED(query)&&actual==DWMSBT_TRANSIENTWINDOW;
        if(requested&&!acrylic){
            const auto clearMaterial=frost?setMaterial(hwnd,DWMSBT_NONE):material;
            const auto clearComposition=setClearClient(hwnd,true);
            m_clearEnabled=SUCCEEDED(alpha)&&SUCCEEDED(clearMaterial)&&SUCCEEDED(clearComposition);
            if(!m_clearEnabled){
                setClearClient(hwnd,false);
                setClientAlpha(hwnd,false);
                setMaterial(hwnd,DWMSBT_AUTO);
            }
            query=DwmGetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&actual,sizeof(actual));
        }

        const bool active=acrylic||m_clearEnabled;
        if(active&&!m_darkOverridden){
            m_darkOverridden=SUCCEEDED(DwmGetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark)));
            if(m_darkOverridden){const BOOL dark=TRUE;DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&dark,sizeof(dark));}
        }else if(!active&&m_darkOverridden){
            DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark));
            m_darkOverridden=false;
        }
        const COLORREF caption=active?RGB(28,36,47):DWMWA_COLOR_DEFAULT;
        const COLORREF text=active?RGB(229,238,247):DWMWA_COLOR_DEFAULT;
        DwmSetWindowAttribute(hwnd,DWMWA_CAPTION_COLOR,&caption,sizeof(caption));
        DwmSetWindowAttribute(hwnd,DWMWA_TEXT_COLOR,&text,sizeof(text));
        m_editor->setProperty("potatoDesktopGlassBackdropRequested",frost);
        m_editor->setProperty("potatoDesktopGlassBackdropAccepted",acrylic);
        m_editor->setProperty("potatoDesktopGlassBackdropReadback",SUCCEEDED(query)?int(actual):-1);
        m_editor->setProperty("potatoDesktopGlassActive",active);
        m_editor->setProperty("potatoDesktopGlassMode",acrylic?"system-acrylic":m_clearEnabled?(frost?"clear-fallback":"clear-desktop"):"opaque");
        m_editor->update();
        for(auto *child:m_editor->findChildren<QWidget*>())if(!child->inherits("QOpenGLWidget"))child->update();
        if(requested&&!m_diagnosticScheduled&&!qEnvironmentVariableIsEmpty("POTATO_GLASS_DIAGNOSTICS")){
            m_diagnosticScheduled=true;
            QTimer::singleShot(400,this,[this]{recordDiagnostics();});
        }
    }
    bool eventFilter(QObject*,QEvent *event)override{
        if(event->type()==QEvent::Show||event->type()==QEvent::WinIdChange)schedule();
        return false;
    }
    bool nativeEventFilter(const QByteArray&,void *message,qintptr*)override{
        const auto *msg=static_cast<MSG*>(message);
        if(msg->hwnd==m_hwnd&&(msg->message==WM_DWMCOMPOSITIONCHANGED||msg->message==WM_SETTINGCHANGE||msg->message==WM_THEMECHANGED))schedule();
        return false;
    }
private:
    void schedule(){
        if(m_pending)return;
        m_pending=true;
        QTimer::singleShot(0,this,[this]{m_pending=false;m_force=true;sync();});
    }
    void recordDiagnostics(){
        if(!m_editor->windowHandle()||!m_canvas||!m_canvas->windowHandle())return;
        // Inspect the app's own paint and canvas. Composited desktop blur requires an optical fixture.
        const auto *bar=m_editor->findChild<QToolBar*>("projectToolbar");
        const QPoint point=bar?bar->mapTo(m_editor,QPoint(bar->width()-24,bar->height()/2)):QPoint(4,4);
        const auto chrome=m_editor->grab(QRect(point,QSize(2,2))).toImage();
        const auto canvas=m_canvas->grabFramebuffer();
        const auto rgba=[](const QImage &image){
            if(image.isNull())return QJsonArray{};
            const QColor color=image.pixelColor(image.rect().center());
            return QJsonArray{color.red(),color.green(),color.blue(),color.alpha()};
        };
        const QJsonObject data{{"theme",qApp->property("potatoInterfaceTheme").toString()},
            {"nativeMode",m_editor->property("potatoDesktopGlassMode").toString()},
            {"nativeClassStyle",QString::number(GetClassLongPtrW(m_hwnd,GCL_STYLE),16)},
            {"nativeWindowExStyle",QString::number(GetWindowLongPtrW(m_hwnd,GWL_EXSTYLE),16)},
            {"editorSurfaceType",int(m_editor->windowHandle()->surfaceType())},
            {"editorAlphaBufferSize",m_editor->windowHandle()->format().alphaBufferSize()},
            {"canvasSurfaceType",int(m_canvas->windowHandle()->surfaceType())},
            {"canvasHasEditorParent",m_canvas->windowHandle()->parent()==m_editor->windowHandle()},
            {"chromePaintRgba",rgba(chrome)},{"canvasFramebufferRgba",rgba(canvas)}};
        QFile file(qEnvironmentVariable("POTATO_GLASS_DIAGNOSTICS"));
        if(file.open(QIODevice::WriteOnly|QIODevice::Truncate))file.write(QJsonDocument(data).toJson());
    }
    QWidget *m_editor;
    QPointer<QOpenGLWidget> m_canvas;
    HWND m_hwnd=nullptr;
    BOOL m_previousDark=FALSE;
    bool m_requested=false,m_frost=false,m_force=true,m_pending=false,m_clearEnabled=false,m_darkOverridden=false,m_diagnosticScheduled=false;
};
#endif
}

void prepare(QWidget *editor,QOpenGLWidget *canvas){
#ifdef Q_OS_WIN
    if(!editor||!canvas||QOperatingSystemVersion::current().microVersion()<22621)return;
    // A native OpenGL child gets its own Qt composition target. The top-level
    // editor can then submit premultiplied raster chrome directly to DWM.
    // Keep the splitter/sidebar as ordinary widgets when designating this one child.
    const bool keepSiblings=QCoreApplication::testAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings,true);
    canvas->setAttribute(Qt::WA_DontCreateNativeAncestors);
    canvas->setAttribute(Qt::WA_NativeWindow);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings,keepSiblings);
    editor->setAttribute(Qt::WA_TranslucentBackground);
    new Backdrop(editor,canvas);
#else
    Q_UNUSED(editor);Q_UNUSED(canvas);
#endif
}
void sync(QWidget *editor){
#ifdef Q_OS_WIN
    if(!editor)return;
    for(auto *child:editor->children())if(child->objectName()=="potatoDesktopGlassAdapter"){
        static_cast<Backdrop*>(child)->sync();return;
    }
#else
    Q_UNUSED(editor);
#endif
}
bool active(const QWidget *window){return window&&window->property("potatoDesktopGlassActive").toBool();}
}
