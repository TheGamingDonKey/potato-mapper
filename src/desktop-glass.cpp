#include "desktop-glass.h"
#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QOperatingSystemVersion>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QWidget>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#endif

namespace PotatoDesktopGlass {
namespace {
bool supported(){
#ifdef Q_OS_WIN
    const auto os=QOperatingSystemVersion::current();
    return os.type()==QOperatingSystemVersion::Windows&&os.microVersion()>=22621;
#else
    return false;
#endif
}
class Backdrop final:public QObject,public QAbstractNativeEventFilter {
public:
    explicit Backdrop(QWidget *editor):QObject(editor),m_editor(editor){
        setObjectName("potatoDesktopGlassAdapter");qApp->installNativeEventFilter(this);editor->installEventFilter(this);
        connect(qApp,&QCoreApplication::aboutToQuit,this,[this]{
            m_stopping=true;m_force=true;sync();qApp->removeNativeEventFilter(this);
            if(m_editor)m_editor->removeEventFilter(this);
        });
    }
    ~Backdrop()override{if(qApp)qApp->removeNativeEventFilter(this);}
    void sync(){
        if(!m_editor||!supported())return;
#ifdef Q_OS_WIN
        HIGHCONTRASTW contrast{sizeof(HIGHCONTRASTW),0,nullptr};
        const bool highContrast=SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(contrast),&contrast,0)&&(contrast.dwFlags&HCF_HIGHCONTRASTON);
        QSettings system("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
        const bool systemTransparency=system.value("EnableTransparency",1).toInt()!=0;
        const bool requested=!m_stopping&&qApp->property("potatoInterfaceTheme").toString()=="lumenatelier1"&&
            !qApp->property("potatoReducedTransparency").toBool()&&!qApp->property("potatoIncreasedContrast").toBool()&&!highContrast&&systemTransparency;
        m_editor->setProperty("potatoDesktopGlassSystemAllowed",!highContrast&&systemTransparency);
        // Do not force an HWND while constructing the editor. The Show/WinId
        // events schedule this once Qt has composed the OpenGL hierarchy.
        if(!m_editor->testAttribute(Qt::WA_WState_Created))return;
        const auto hwnd=m_stopping?m_hwnd:reinterpret_cast<HWND>(m_editor->winId());
        if(!hwnd||!IsWindow(hwnd))return;
        if(m_hwnd==hwnd&&m_applied==requested&&!m_force)return;
        m_force=false;m_hwnd=hwnd;m_applied=requested;
        // Keep the native non-client frame opaque. Extending glass over the
        // whole frame makes the caption inherit the window underneath it.
        const MARGINS margins{0,0,0,0};
        const HRESULT frame=DwmExtendFrameIntoClientArea(hwnd,&margins);
        const DWM_SYSTEMBACKDROP_TYPE type=requested?DWMSBT_NONE:DWMSBT_AUTO;
        const HRESULT backdrop=DwmSetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&type,sizeof(type));
        // On modern Windows this documented API enables client alpha, but
        // does not provide the old Aero blur. No blur claim is made here.
        DWM_BLURBEHIND client{};client.dwFlags=DWM_BB_ENABLE;client.fEnable=requested;
        if(requested){client.dwFlags|=DWM_BB_BLURREGION;client.hRgnBlur=CreateRectRgn(0,0,-1,-1);}
        const HRESULT composition=requested&&!client.hRgnBlur?E_OUTOFMEMORY:DwmEnableBlurBehindWindow(hwnd,&client);
        if(client.hRgnBlur)DeleteObject(client.hRgnBlur);
        // Retain DWM's native caption rendering on 24H2 as well.
        HRESULT alphaResult=S_OK;
        if(QOperatingSystemVersion::current().microVersion()>=26100){
            const BOOL alpha=FALSE;
            alphaResult=DwmSetWindowAttribute(hwnd,DWMWA_REDIRECTIONBITMAP_ALPHA,&alpha,sizeof(alpha));
        }
        const bool enabled=requested&&SUCCEEDED(frame)&&SUCCEEDED(backdrop)&&SUCCEEDED(composition)&&SUCCEEDED(alphaResult);
        if(requested&&!enabled){
            DWM_BLURBEHIND resetClient{};resetClient.dwFlags=DWM_BB_ENABLE;resetClient.fEnable=FALSE;DwmEnableBlurBehindWindow(hwnd,&resetClient);
            const MARGINS reset{0,0,0,0};DwmExtendFrameIntoClientArea(hwnd,&reset);
            const DWM_SYSTEMBACKDROP_TYPE resetType=DWMSBT_AUTO;DwmSetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&resetType,sizeof(resetType));
            if(QOperatingSystemVersion::current().microVersion()>=26100){const BOOL alpha=FALSE;DwmSetWindowAttribute(hwnd,DWMWA_REDIRECTIONBITMAP_ALPHA,&alpha,sizeof(alpha));}
        }
        const COLORREF caption=enabled?RGB(36,41,48):DWMWA_COLOR_DEFAULT;
        const COLORREF text=enabled?RGB(238,241,245):DWMWA_COLOR_DEFAULT;
        if(enabled){
            if(!m_captionOverridden){DwmGetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark));m_captionOverridden=true;}
            const BOOL dark=TRUE;DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&dark,sizeof(dark));
        }else if(m_captionOverridden){DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark));m_captionOverridden=false;}
        DwmSetWindowAttribute(hwnd,DWMWA_CAPTION_COLOR,&caption,sizeof(caption));
        DwmSetWindowAttribute(hwnd,DWMWA_TEXT_COLOR,&text,sizeof(text));
        m_editor->setProperty("potatoDesktopGlassActive",enabled);
        m_editor->setProperty("potatoDesktopGlassHresult",qint64(FAILED(frame)?frame:(FAILED(backdrop)?backdrop:(FAILED(composition)?composition:alphaResult))));
        m_editor->update();for(auto *child:m_editor->findChildren<QWidget*>())if(!child->inherits("QOpenGLWidget"))child->update();
#endif
    }
    bool eventFilter(QObject*,QEvent *event)override{
        if(event->type()==QEvent::Show||event->type()==QEvent::WinIdChange)schedule();
        return false;
    }
    bool nativeEventFilter(const QByteArray&,void *message,qintptr*)override{
#ifdef Q_OS_WIN
        const auto *msg=static_cast<MSG*>(message);
        if(msg->hwnd==m_hwnd&&(msg->message==WM_DWMCOMPOSITIONCHANGED||msg->message==WM_SETTINGCHANGE||msg->message==WM_THEMECHANGED))schedule();
#else
        Q_UNUSED(message);
#endif
        return false;
    }
private:
    void schedule(){if(m_pending||m_stopping)return;m_pending=true;QTimer::singleShot(0,this,[this]{m_pending=false;if(!m_stopping){m_force=true;sync();}});}
    QPointer<QWidget> m_editor;bool m_pending=false,m_force=true,m_applied=false,m_stopping=false;
#ifdef Q_OS_WIN
    HWND m_hwnd=nullptr;
    BOOL m_previousDark=FALSE;bool m_captionOverridden=false;
#endif
};
}
void prepare(QWidget *editor){if(editor&&supported()){editor->setAttribute(Qt::WA_TranslucentBackground);new Backdrop(editor);}}
void sync(QWidget *editor){if(!editor)return;for(auto *child:editor->children())if(child->objectName()=="potatoDesktopGlassAdapter"){static_cast<Backdrop*>(child)->sync();return;}}
bool active(const QWidget *window){return window&&window->property("potatoDesktopGlassActive").toBool();}
}
