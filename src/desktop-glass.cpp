#include "desktop-glass.h"
#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
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
// Clear any legacy accent policy before selecting the documented DWM material.
// Resolve this compatibility API at runtime; it is not an SDK contract.
struct AccentPolicy { int state; DWORD flags; DWORD tint; DWORD animation; };
struct CompositionAttributeData { int attribute; void *data; SIZE_T size; };
using SetCompositionAttribute=BOOL(WINAPI*)(HWND,CompositionAttributeData*);
constexpr int accentDisabled=0,accentPolicyAttribute=19;
HRESULT setAccent(HWND hwnd,int state=accentDisabled,DWORD tint=0){
    static const auto function=reinterpret_cast<SetCompositionAttribute>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetWindowCompositionAttribute"));
    if(!function)return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
    const DWORD flags=state==4&&qEnvironmentVariable("POTATO_GLASS_ACCENT_FLAGS_CONTROL")!="0"?DWORD(2):DWORD(0);
    AccentPolicy policy{state,flags,tint,0};
    CompositionAttributeData data{accentPolicyAttribute,&policy,sizeof(policy)};
    SetLastError(ERROR_SUCCESS);
    if(function(hwnd,&data))return S_OK;
    const DWORD error=GetLastError();return error?HRESULT_FROM_WIN32(error):E_FAIL;
}
#endif
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
#ifdef Q_OS_WIN
            restoreLayeredWindow();
#endif
            if(m_editor)m_editor->removeEventFilter(this);
        });
    }
    ~Backdrop()override{
#ifdef Q_OS_WIN
        restoreLayeredWindow();
#endif
        if(qApp)qApp->removeNativeEventFilter(this);
    }
    void sync(){
        if(!m_editor||!supported())return;
        connectDiagnostics();
#ifdef Q_OS_WIN
        HIGHCONTRASTW contrast{sizeof(HIGHCONTRASTW),0,nullptr};
        const bool highContrast=SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(contrast),&contrast,0)&&(contrast.dwFlags&HCF_HIGHCONTRASTON);
        QSettings system("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
        const bool systemTransparency=system.value("EnableTransparency",1).toInt()!=0;
        const bool requested=!m_stopping&&qApp->property("potatoInterfaceTheme").toString()=="lumenatelier1"&&
            !qApp->property("potatoReducedTransparency").toBool()&&!qApp->property("potatoIncreasedContrast").toBool()&&!highContrast&&systemTransparency;
        const bool accentControl=requested&&(qEnvironmentVariableIsSet("POTATO_GLASS_ACCENT_CONTROL")||qEnvironmentVariableIsSet("POTATO_GLASS_BLUR_CONTROL"));
        const int accentState=qEnvironmentVariableIntValue("POTATO_GLASS_BLUR_CONTROL")==3?3:4;
        const bool clearControl=requested&&!accentControl&&qEnvironmentVariableIsSet("POTATO_GLASS_CLEAR_CONTROL");
        const bool fullRegionControl=requested&&qEnvironmentVariableIsSet("POTATO_GLASS_FULL_REGION_CONTROL");
        const bool redirectionAlpha=requested&&qEnvironmentVariableIntValue("POTATO_GLASS_REDIRECTION_ALPHA_CONTROL")==1;
        m_editor->setProperty("potatoDesktopGlassSystemAllowed",!highContrast&&systemTransparency);
        // Do not force an HWND while constructing the editor. The Show/WinId
        // events schedule this once Qt has composed the OpenGL hierarchy.
        if(!m_editor->testAttribute(Qt::WA_WState_Created))return;
        const auto hwnd=m_stopping?m_hwnd:reinterpret_cast<HWND>(m_editor->winId());
        if(!hwnd||!IsWindow(hwnd))return;
        if(!m_stopping)prepareLayeredWindow(hwnd);
        if(m_hwnd==hwnd&&m_applied==requested&&!m_force)return;
        m_force=false;m_hwnd=hwnd;m_applied=requested;
        const HRESULT accentReset=setAccent(hwnd);
        if(requested){
            if(!m_captionOverridden){DwmGetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark));m_captionOverridden=true;}
            const BOOL dark=TRUE;DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&dark,sizeof(dark));
        }else if(m_captionOverridden){DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark));m_captionOverridden=false;}
        // Prepare Qt's client alpha before selecting the system material.
        DWM_BLURBEHIND client{};client.dwFlags=DWM_BB_ENABLE;client.fEnable=requested;
        if(requested&&!fullRegionControl){client.dwFlags|=DWM_BB_BLURREGION;client.hRgnBlur=CreateRectRgn(0,0,-1,-1);}
        const HRESULT composition=requested&&!fullRegionControl&&!client.hRgnBlur?E_OUTOFMEMORY:DwmEnableBlurBehindWindow(hwnd,&client);
        if(client.hRgnBlur)DeleteObject(client.hRgnBlur);
        // Keep the default redirection bitmap policy. Qt's OpenGL client
        // uses the alpha composition path prepared above.
        HRESULT alphaResult=S_OK;
        if(QOperatingSystemVersion::current().microVersion()>=26100){
            const BOOL alpha=redirectionAlpha?TRUE:FALSE;
            alphaResult=DwmSetWindowAttribute(hwnd,DWMWA_REDIRECTIONBITMAP_ALPHA,&alpha,sizeof(alpha));
        }
        // QWindowKit's maintained Windows implementation extends only the
        // left frame across the client before selecting Acrylic. Zero margins
        // can leave the client black; negative top margins disturb captions.
        const MARGINS margins=requested&&!clearControl&&!accentControl?MARGINS{65536,0,0,0}:MARGINS{0,0,0,0};
        const HRESULT frame=DwmExtendFrameIntoClientArea(hwnd,&margins);
        const DWM_SYSTEMBACKDROP_TYPE type=requested?((clearControl||accentControl)?DWMSBT_NONE:DWMSBT_TRANSIENTWINDOW):DWMSBT_AUTO;
        const HRESULT backdrop=DwmSetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&type,sizeof(type));
        // Diagnostic Acrylic candidate on the optically verified clear path.
        // Accent tint uses ABGR; alpha24 leaves most light from the desktop.
        const DWORD accentTint=(DWORD(24)<<24)|(DWORD(47)<<16)|(DWORD(36)<<8)|DWORD(28);
        const HRESULT accentResult=accentControl?setAccent(hwnd,accentState,accentState==4?accentTint:0):accentReset;
        DWM_SYSTEMBACKDROP_TYPE readback=DWMSBT_AUTO;
        HRESULT readbackResult=DwmGetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&readback,sizeof(readback));
        const bool nativeAccepted=requested&&SUCCEEDED(frame)&&SUCCEEDED(backdrop)&&SUCCEEDED(composition)&&SUCCEEDED(alphaResult)&&
            SUCCEEDED(readbackResult)&&readback==type&&(!accentControl||SUCCEEDED(accentResult));
        const bool blurAccepted=nativeAccepted&&!clearControl;
        bool enabled=nativeAccepted;
        HRESULT fallbackResult=S_OK;
        if(requested&&!nativeAccepted){
            const MARGINS reset{0,0,0,0};const HRESULT clearFrame=DwmExtendFrameIntoClientArea(hwnd,&reset);
            const DWM_SYSTEMBACKDROP_TYPE resetType=DWMSBT_NONE;const HRESULT clearBackdrop=DwmSetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&resetType,sizeof(resetType));
            DWM_BLURBEHIND clearClient{};clearClient.dwFlags=DWM_BB_ENABLE|DWM_BB_BLURREGION;clearClient.fEnable=TRUE;clearClient.hRgnBlur=CreateRectRgn(0,0,-1,-1);
            const HRESULT clearComposition=clearClient.hRgnBlur?DwmEnableBlurBehindWindow(hwnd,&clearClient):E_OUTOFMEMORY;
            if(clearClient.hRgnBlur)DeleteObject(clearClient.hRgnBlur);
            fallbackResult=FAILED(clearFrame)?clearFrame:FAILED(clearBackdrop)?clearBackdrop:FAILED(clearComposition)?clearComposition:alphaResult;
            enabled=SUCCEEDED(fallbackResult);
            readbackResult=DwmGetWindowAttribute(hwnd,DWMWA_SYSTEMBACKDROP_TYPE,&readback,sizeof(readback));
        }
        const COLORREF caption=enabled?RGB(28,36,47):DWMWA_COLOR_DEFAULT;
        const COLORREF text=enabled?RGB(229,238,247):DWMWA_COLOR_DEFAULT;
        if(!enabled&&m_captionOverridden){DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&m_previousDark,sizeof(m_previousDark));m_captionOverridden=false;}
        DwmSetWindowAttribute(hwnd,DWMWA_CAPTION_COLOR,&caption,sizeof(caption));
        DwmSetWindowAttribute(hwnd,DWMWA_TEXT_COLOR,&text,sizeof(text));
        // Request acceptance/readback are API evidence, not visual proof of
        // desktop blur (window capture can omit the external backdrop).
        m_editor->setProperty("potatoDesktopGlassBackdropRequested",requested);
        m_editor->setProperty("potatoDesktopGlassBackdropAccepted",blurAccepted);
        m_editor->setProperty("potatoDesktopGlassBackdropReadback",SUCCEEDED(readbackResult)?int(readback):-1);
        m_editor->setProperty("potatoDesktopGlassBackdropReadbackHresult",qint64(readbackResult));
        m_editor->setProperty("potatoDesktopGlassMode",clearControl&&nativeAccepted?"clear-control":accentControl&&blurAccepted?(accentState==4?"accent-acrylic":"accent-blur"):blurAccepted?"system-acrylic":enabled?"clear-fallback":"opaque");
        m_editor->setProperty("potatoDesktopGlassAccentState",accentControl?accentState:accentDisabled);
        m_editor->setProperty("potatoDesktopGlassAccentHresult",qint64(accentResult));
        m_editor->setProperty("potatoDesktopGlassTintAlpha",accentControl&&accentState==4?24:0);
        m_editor->setProperty("potatoDesktopGlassRedirectionAlphaRequested",redirectionAlpha);
        m_editor->setProperty("potatoDesktopGlassFallbackUsed",requested&&!nativeAccepted);
        m_editor->setProperty("potatoDesktopGlassFallbackHresult",qint64(fallbackResult));
        m_editor->setProperty("potatoDesktopGlassActive",enabled);
        m_editor->setProperty("potatoDesktopGlassHresult",qint64(FAILED(frame)?frame:(FAILED(backdrop)?backdrop:(FAILED(composition)?composition:alphaResult))));
        m_editor->update();for(auto *child:m_editor->findChildren<QWidget*>())if(!child->inherits("QOpenGLWidget"))child->update();
#endif
    }
    bool eventFilter(QObject*,QEvent *event)override{
#ifdef Q_OS_WIN
        // A GPU driver can select its presentation mode on the first swap.
        // Enable the native per-pixel presentation path synchronously, before
        // the deferred material setup or any OpenGL presentation takes place.
        if(event->type()==QEvent::WinIdChange&&!m_stopping&&m_editor&&supported()){
            const auto hwnd=reinterpret_cast<HWND>(m_editor->internalWinId());
            if(hwnd&&IsWindow(hwnd))prepareLayeredWindow(hwnd);
        }
#endif
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
#ifdef Q_OS_WIN
    void prepareLayeredWindow(HWND hwnd){
        if(qEnvironmentVariableIsSet("POTATO_GLASS_NO_LAYER_CONTROL"))return;
        if(m_layeredHwnd!=hwnd){
            restoreLayeredWindow();m_layeredHwnd=hwnd;
            m_layeredOriginally=(GetWindowLongPtrW(hwnd,GWL_EXSTYLE)&WS_EX_LAYERED)!=0;
            m_savedLayeredAttributes=m_layeredOriginally&&GetLayeredWindowAttributes(hwnd,&m_previousLayeredKey,&m_previousLayeredAlpha,&m_previousLayeredFlags);
        }
        const auto style=GetWindowLongPtrW(hwnd,GWL_EXSTYLE);
        if((style&WS_EX_LAYERED)&&m_layeredPrepared)return;
        SetLastError(ERROR_SUCCESS);
        const auto previous=SetWindowLongPtrW(hwnd,GWL_EXSTYLE,style|WS_EX_LAYERED);
        const DWORD styleError=GetLastError();
        HRESULT result=(!previous&&styleError)?HRESULT_FROM_WIN32(styleError):S_OK;
        if(SUCCEEDED(result)&&!SetLayeredWindowAttributes(hwnd,0,255,LWA_ALPHA)){
            const DWORD error=GetLastError();result=error?HRESULT_FROM_WIN32(error):E_FAIL;
            if(!m_layeredOriginally)SetWindowLongPtrW(hwnd,GWL_EXSTYLE,GetWindowLongPtrW(hwnd,GWL_EXSTYLE)&~LONG_PTR(WS_EX_LAYERED));
        }
        m_layeredPrepared=SUCCEEDED(result);
        if(m_editor){m_editor->setProperty("potatoDesktopGlassLayeredPrepared",m_layeredPrepared);m_editor->setProperty("potatoDesktopGlassLayeredHresult",qint64(result));}
        // Constant opacity remains 255: text and the projector canvas retain
        // their own alpha. Keep this mode for the editor HWND lifetime so a
        // later switch from an opaque theme can still reveal desktop glass.
    }
    void restoreLayeredWindow(){
        if(m_layeredHwnd&&IsWindow(m_layeredHwnd)){
            if(!m_layeredOriginally)SetWindowLongPtrW(m_layeredHwnd,GWL_EXSTYLE,GetWindowLongPtrW(m_layeredHwnd,GWL_EXSTYLE)&~LONG_PTR(WS_EX_LAYERED));
            else if(m_savedLayeredAttributes)SetLayeredWindowAttributes(m_layeredHwnd,m_previousLayeredKey,m_previousLayeredAlpha,m_previousLayeredFlags);
        }
        m_layeredHwnd=nullptr;m_layeredPrepared=false;m_savedLayeredAttributes=false;
    }
#endif
    // Opt-in, one-shot boundary measurement. QWidget::grab() does not show
    // whether the final native OpenGL presentation retained its alpha channel.
    void connectDiagnostics(){
        if(m_diagnosticConnected||qEnvironmentVariableIsEmpty("POTATO_GLASS_DIAGNOSTICS"))return;
        if(auto *canvas=m_editor->findChild<QOpenGLWidget*>()){
            m_diagnosticConnected=true;
            connect(canvas,&QOpenGLWidget::frameSwapped,this,[this,canvas]{
                if(m_diagnosticDone||!m_editor||!canvas->isValid())return;
                auto *context=QOpenGLContext::currentContext();
                if(!context||!context->isValid())return;
                // The compositor can skip the initial flush before the native
                // expose event. Wait for a real top-level presentation.
                if(++m_diagnosticFrames<2){QTimer::singleShot(80,m_editor,[this]{if(m_editor)m_editor->update();});return;}
                m_diagnosticDone=true;recordDiagnostics(canvas,context);
            },Qt::DirectConnection);
        }
    }
    void recordDiagnostics(QOpenGLWidget *canvas,QOpenGLContext *context){
        auto describeFormat=[](const QSurfaceFormat &f){return QJsonObject{
            {"alpha",f.alphaBufferSize()},{"red",f.redBufferSize()},{"green",f.greenBufferSize()},
            {"blue",f.blueBufferSize()},{"depth",f.depthBufferSize()},{"stencil",f.stencilBufferSize()},
            {"major",f.majorVersion()},{"minor",f.minorVersion()},{"profile",int(f.profile())},
            {"swapBehavior",int(f.swapBehavior())}};};
        auto describeWindow=[&](QWindow *window){
            QJsonObject result;if(!window)return result;
            result.insert("winId",QString::number(quintptr(window->winId()),16));
            result.insert("surfaceType",int(window->surfaceType()));
            result.insert("requestedFormat",describeFormat(window->requestedFormat()));
            result.insert("format",describeFormat(window->format()));return result;
        };
        auto *surface=context->surface();
        auto *surfaceWindow=surface&&surface->surfaceClass()==QSurface::Window?static_cast<QWindow*>(surface):nullptr;
        QJsonObject report{{"editorWinId",QString::number(quintptr(m_editor->winId()),16)},
            {"editorWindow",describeWindow(m_editor->windowHandle())},{"currentSurface",describeWindow(surfaceWindow)},
            {"currentContextFormat",describeFormat(context->format())},
            {"canvasContextFormat",describeFormat(canvas->context()->format())},
            {"editorExposed",m_editor->windowHandle()&&m_editor->windowHandle()->isExposed()},
            {"waTranslucent",m_editor->testAttribute(Qt::WA_TranslucentBackground)},
            {"nativeMode",m_editor->property("potatoDesktopGlassMode").toString()},
            {"theme",qApp->property("potatoInterfaceTheme").toString()}};
#ifdef Q_OS_WIN
        auto describeDc=[](HDC dc){
            QJsonObject result;result.insert("dc",QString::number(quintptr(dc),16));
            const int index=dc?GetPixelFormat(dc):0;result.insert("pixelFormatIndex",index);
            PIXELFORMATDESCRIPTOR pfd{};
            const int count=index?DescribePixelFormat(dc,index,sizeof(pfd),&pfd):0;
            result.insert("describePixelFormatCount",count);
            if(count){result.insert("alphaBits",pfd.cAlphaBits);result.insert("colorBits",pfd.cColorBits);result.insert("flags",qint64(pfd.dwFlags));}
            return result;
        };
        const auto hwnd=reinterpret_cast<HWND>(surfaceWindow?surfaceWindow->winId():m_editor->winId());
        HDC dc=GetDC(hwnd);report.insert("nativeWindowDc",describeDc(dc));if(dc)ReleaseDC(hwnd,dc);
        report.insert("nativeWindowExStyle",QString::number(quintptr(GetWindowLongPtrW(hwnd,GWL_EXSTYLE)),16));
        report.insert("nativeClassStyle",QString::number(quintptr(GetClassLongPtrW(hwnd,GCL_STYLE)),16));
        using CurrentDc=HDC(WINAPI*)();
        const auto currentDc=reinterpret_cast<CurrentDc>(GetProcAddress(GetModuleHandleW(L"opengl32.dll"),"wglGetCurrentDC"));
        if(currentDc){const HDC renderingDc=currentDc();report.insert("currentWglDc",describeDc(renderingDc));report.insert("currentWglHwnd",QString::number(quintptr(WindowFromDC(renderingDc)),16));}
#endif
        // Qt may leave a texture-upload context current by frameSwapped. Its
        // canvas share context is the top-level compositor context. Temporarily
        // select the existing editor surface for readback, then restore both.
        auto *previousContext=context;auto *previousSurface=surface;
        bool switched=false;
        if(surfaceWindow!=m_editor->windowHandle()){
            auto *compositor=canvas->context()->shareContext();
            if(!compositor)compositor=context;
            if(compositor->makeCurrent(m_editor->windowHandle())){context=compositor;surfaceWindow=m_editor->windowHandle();switched=true;}
        }
        report.insert("diagnosticSurfaceSwitch",switched);
        if(surfaceWindow==m_editor->windowHandle()){
            auto *gl=context->extraFunctions();QJsonObject gpu;
            gpu.insert("compositorContextFormat",describeFormat(context->format()));
            gpu.insert("initialError",int(gl->glGetError()));
            auto glString=[&](GLenum name){const auto *value=gl->glGetString(name);return value?QString::fromLatin1(reinterpret_cast<const char*>(value)):QString();};
            gpu.insert("vendor",glString(GL_VENDOR));gpu.insert("renderer",glString(GL_RENDERER));gpu.insert("version",glString(GL_VERSION));
            GLint oldReadFbo=0,oldPackBuffer=0,oldPackAlignment=0,oldPackRowLength=0,oldPackSkipRows=0,oldPackSkipPixels=0;
            gl->glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&oldReadFbo);gl->glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&oldPackBuffer);
            gl->glGetIntegerv(GL_PACK_ALIGNMENT,&oldPackAlignment);gl->glGetIntegerv(GL_PACK_ROW_LENGTH,&oldPackRowLength);
            gl->glGetIntegerv(GL_PACK_SKIP_ROWS,&oldPackSkipRows);gl->glGetIntegerv(GL_PACK_SKIP_PIXELS,&oldPackSkipPixels);
            gl->glBindFramebuffer(GL_READ_FRAMEBUFFER,0);GLint defaultReadBuffer=0;gl->glGetIntegerv(GL_READ_BUFFER,&defaultReadBuffer);
            GLint alphaBits=-1;gl->glGetIntegerv(0x0D55 /* GL_ALPHA_BITS */,&alphaBits);
            gpu.insert("legacyAlphaBits",alphaBits);gpu.insert("legacyAlphaQueryError",int(gl->glGetError()));
            GLint attachmentAlpha=-1;gl->glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER,GL_FRONT_LEFT,GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,&attachmentAlpha);
            gpu.insert("frontAttachmentAlphaBits",attachmentAlpha);gpu.insert("attachmentQueryError",int(gl->glGetError()));
            gl->glBindBuffer(GL_PIXEL_PACK_BUFFER,0);gl->glPixelStorei(GL_PACK_ALIGNMENT,1);gl->glPixelStorei(GL_PACK_ROW_LENGTH,0);
            gl->glPixelStorei(GL_PACK_SKIP_ROWS,0);gl->glPixelStorei(GL_PACK_SKIP_PIXELS,0);
            QPoint point(m_editor->width()-24,32);
            if(auto *toolbar=m_editor->findChild<QToolBar*>("projectToolbar"))point=toolbar->mapTo(m_editor,QPoint(qMax(0,toolbar->width()-24),toolbar->height()/2));
            const qreal scale=surfaceWindow->devicePixelRatio();const int x=qRound(point.x()*scale),y=qRound((surfaceWindow->height()-point.y())*scale)-1;
            gpu.insert("sampleClientLogical",QJsonArray{point.x(),point.y()});gpu.insert("sampleFramebuffer",QJsonArray{x,y});
            gpu.insert("previousReadFramebuffer",oldReadFbo);
            for(const GLenum buffer:{GLenum(GL_FRONT),GLenum(GL_BACK)}){
                GLubyte rgba[4]={0,0,0,0};gl->glReadBuffer(buffer);gl->glReadPixels(x,y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,rgba);
                gpu.insert(buffer==GL_FRONT?"frontRgba":"backRgba",QJsonArray{rgba[0],rgba[1],rgba[2],rgba[3]});
                gpu.insert(buffer==GL_FRONT?"frontReadError":"backReadError",int(gl->glGetError()));
            }
            const QPoint canvasPoint=canvas->mapTo(m_editor,canvas->rect().center());
            GLubyte canvasRgba[4]={0,0,0,0};gl->glReadBuffer(GL_FRONT);
            gl->glReadPixels(qRound(canvasPoint.x()*scale),qRound((surfaceWindow->height()-canvasPoint.y())*scale)-1,1,1,GL_RGBA,GL_UNSIGNED_BYTE,canvasRgba);
            gpu.insert("opaqueCanvasControlFrontRgba",QJsonArray{canvasRgba[0],canvasRgba[1],canvasRgba[2],canvasRgba[3]});
            gpu.insert("canvasControlReadError",int(gl->glGetError()));
            gl->glReadBuffer(GLenum(defaultReadBuffer));gl->glBindFramebuffer(GL_READ_FRAMEBUFFER,GLuint(oldReadFbo));
            gl->glBindBuffer(GL_PIXEL_PACK_BUFFER,GLuint(oldPackBuffer));gl->glPixelStorei(GL_PACK_ALIGNMENT,oldPackAlignment);
            gl->glPixelStorei(GL_PACK_ROW_LENGTH,oldPackRowLength);gl->glPixelStorei(GL_PACK_SKIP_ROWS,oldPackSkipRows);gl->glPixelStorei(GL_PACK_SKIP_PIXELS,oldPackSkipPixels);
            gpu.insert("restoreError",int(gl->glGetError()));report.insert("gpu",gpu);
        }else report.insert("gpuSkipped","Unable to select the existing editor surface for readback.");
        if(switched)report.insert("previousContextRestored",previousSurface&&previousContext->makeCurrent(previousSurface));
        const auto json=QJsonDocument(report).toJson(QJsonDocument::Indented);
        m_editor->setProperty("potatoDesktopGlassDiagnostics",QString::fromUtf8(json));
        QFile file(qEnvironmentVariable("POTATO_GLASS_DIAGNOSTICS"));
        if(file.open(QIODevice::WriteOnly|QIODevice::Truncate))file.write(json);
        qInfo().noquote()<<"Desktop glass in-process diagnostics:"<<QString::fromUtf8(json);
    }
    void schedule(){if(m_pending||m_stopping)return;m_pending=true;QTimer::singleShot(0,this,[this]{m_pending=false;if(!m_stopping){m_force=true;sync();}});}
    QPointer<QWidget> m_editor;bool m_pending=false,m_force=true,m_applied=false,m_stopping=false;
    bool m_diagnosticConnected=false,m_diagnosticDone=false;
    int m_diagnosticFrames=0;
#ifdef Q_OS_WIN
    HWND m_hwnd=nullptr;
    HWND m_layeredHwnd=nullptr;bool m_layeredOriginally=false,m_layeredPrepared=false,m_savedLayeredAttributes=false;
    COLORREF m_previousLayeredKey=0;BYTE m_previousLayeredAlpha=255;DWORD m_previousLayeredFlags=LWA_ALPHA;
    BOOL m_previousDark=FALSE;bool m_captionOverridden=false;
#endif
};
}
void prepare(QWidget *editor){if(editor&&supported()){editor->setAttribute(Qt::WA_TranslucentBackground);new Backdrop(editor);}}
void sync(QWidget *editor){if(!editor)return;for(auto *child:editor->children())if(child->objectName()=="potatoDesktopGlassAdapter"){static_cast<Backdrop*>(child)->sync();return;}}
bool active(const QWidget *window){return window&&window->property("potatoDesktopGlassActive").toBool();}
}
