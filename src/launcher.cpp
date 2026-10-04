// The portable launcher deliberately uses only Windows and the C++ runtime.
// Each editor version carries its own Qt libraries in versions/<version>.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>

namespace fs=std::filesystem;
static HWND progressWindow=nullptr,progressLabel=nullptr,progressBar=nullptr;
static void pump(){MSG m;while(PeekMessageW(&m,nullptr,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}}
static void progress(const wchar_t *text,int percent=0){
    if(progressLabel)SetWindowTextW(progressLabel,text);
    if(progressBar)SendMessageW(progressBar,PBM_SETPOS,percent,0);pump();
}
static void showProgress(){
    INITCOMMONCONTROLSEX controls{sizeof(controls),ICC_PROGRESS_CLASS};InitCommonControlsEx(&controls);
    const int x=(GetSystemMetrics(SM_CXSCREEN)-480)/2,y=(GetSystemMetrics(SM_CYSCREEN)-160)/2;
    progressWindow=CreateWindowExW(WS_EX_TOPMOST,L"STATIC",L"Potato Mapper — Updating",WS_OVERLAPPED|WS_CAPTION|WS_VISIBLE,x,y,480,160,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    progressLabel=CreateWindowW(L"STATIC",L"Preparing your update…",WS_CHILD|WS_VISIBLE,24,25,420,35,progressWindow,nullptr,nullptr,nullptr);
    progressBar=CreateWindowW(PROGRESS_CLASSW,nullptr,WS_CHILD|WS_VISIBLE,24,76,420,20,progressWindow,nullptr,nullptr,nullptr);
    SendMessageW(progressBar,PBM_SETRANGE,0,MAKELPARAM(0,100));pump();
}
static std::string read(const fs::path &path){std::ifstream in(path,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
static bool versionValid(const std::string &s){return std::regex_match(s,std::regex("[0-9]{1,5}\\.[0-9]{1,5}\\.[0-9]{1,5}"));}
static std::string selected(const fs::path &root,const wchar_t *name){auto s=read(root/name);while(!s.empty()&&(s.back()=='\n'||s.back()=='\r'))s.pop_back();if(!versionValid(s))throw std::runtime_error("The installed version pointer is missing or invalid. Extract the complete Potato Mapper ZIP again.");return s;}
static void writeAtomic(const fs::path &root,const wchar_t *name,const std::string &bytes){
    const auto temp=root/(std::wstring(name)+L".new-"+std::to_wstring(GetCurrentProcessId()));
    {std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<bytes;out.flush();if(!out)throw std::runtime_error("Could not write the version pointer. Check folder permissions and free space.");}
    if(!MoveFileExW(temp.c_str(),(root/name).c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Could not select the installed version. The current version was not changed.");
}
static void writePointer(const fs::path &root,const wchar_t *name,const std::string &value){writeAtomic(root,name,value+"\n");}
static bool inside(const fs::path &child,const fs::path &parent){
    auto c=fs::weakly_canonical(child),p=fs::weakly_canonical(parent);auto ci=c.begin();
    for(auto pi=p.begin();pi!=p.end();++pi,++ci){if(ci==c.end()||_wcsicmp(ci->c_str(),pi->c_str())!=0)return false;}
    return ci!=c.end();
}
static std::wstring quote(const std::wstring &s){
    std::wstring out=L"\"";size_t slash=0;
    for(auto c:s){if(c==L'\\'){++slash;continue;}if(c==L'\"'){out.append(slash*2+1,L'\\');out+=c;}else{out.append(slash,L'\\');out+=c;}slash=0;}
    out.append(slash*2,L'\\');out+=L'\"';return out;
}
static DWORD launch(const fs::path &root,const std::vector<std::wstring> &args,bool wait){
    const auto ver=selected(root,L"current.txt");const auto exe=root/L"versions"/ver/L"PotatoMapperApp.exe";
    if(!fs::is_regular_file(exe))throw std::runtime_error("The selected application version is incomplete. Use a complete release ZIP or recover the previous version.");
    std::wstring command=quote(exe.wstring())+L" --install-root "+quote(root.wstring());
    for(const auto &a:args)command+=L" "+quote(a);
    STARTUPINFOW start{};start.cb=sizeof(start);PROCESS_INFORMATION child{};
    if(!CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,root.c_str(),&start,&child))throw std::runtime_error("Windows could not start Potato Mapper. Check that the complete ZIP was extracted.");
    CloseHandle(child.hThread);DWORD code=0;
    const auto status=WaitForSingleObject(child.hProcess,wait?INFINITE:1500);
    if(status==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
    CloseHandle(child.hProcess);
    if(!wait&&status==WAIT_OBJECT_0&&code!=0)throw std::runtime_error("The editor could not start. A runtime library may be missing or damaged.");
    return code;
}
static void waitForEditor(DWORD pid){
    if(!pid)return;HANDLE editor=OpenProcess(SYNCHRONIZE,FALSE,pid);
    if(!editor){if(GetLastError()==ERROR_INVALID_PARAMETER)return;throw std::runtime_error("Could not wait for the editor to close. Close Potato Mapper and try again.");}
    const auto start=GetTickCount64();while(WaitForSingleObject(editor,100)==WAIT_TIMEOUT){pump();if(GetTickCount64()-start>30000){CloseHandle(editor);throw std::runtime_error("The editor is still running. No update was applied.");}}
    CloseHandle(editor);
}
static std::wstring wide(const std::string &s){int n=MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),nullptr,0);std::wstring out(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),out.data(),n);return out;}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){
    std::vector<wchar_t> path(32768);GetModuleFileNameW(nullptr,path.data(),DWORD(path.size()));const auto root=fs::path(path.data()).parent_path();
    int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);std::vector<std::wstring> args;for(int i=1;i<argc;i++)args.emplace_back(argv[i]);LocalFree(argv);
    auto value=[&](const wchar_t *flag){for(size_t i=0;i+1<args.size();++i)if(args[i]==flag)return args[i+1];return std::wstring();};
    auto has=[&](const wchar_t *flag){for(const auto &a:args)if(a==flag)return true;return false;};
    const bool apply=has(L"--apply-update"),rollback=has(L"--rollback"),noLaunch=has(L"--no-launch");bool editorClosed=false;
    HANDLE lock=INVALID_HANDLE_VALUE;fs::path pendingOwned,sessionOwned;std::string restoreVersion,priorPrevious;bool previousExisted=false,pointersTouched=false;
    try{
        if(read(root/L"installation.txt")!="PotatoMapper/1\n")throw std::runtime_error("Extract the entire Potato Mapper package before starting it. Keep installation.txt and versions beside this launcher.");
        if(!apply&&!rollback){
            HANDLE busy=CreateFileW((root/L".update-lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_HIDDEN,nullptr);
            if(busy==INVALID_HANDLE_VALUE)throw std::runtime_error("Potato Mapper is updating. Wait for the update to finish and open it again.");lock=busy;
            bool wait=false;for(const auto &a:args)if(a.rfind(L"--smoke",0)==0||a==L"--profile-media"||a.rfind(L"--test-",0)==0)wait=true;
            const int result=int(launch(root,args,wait));CloseHandle(lock);return result;
        }
        lock=CreateFileW((root/L".update-lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_HIDDEN,nullptr);
        if(lock==INVALID_HANDLE_VALUE)throw std::runtime_error("Another update is already in progress.");
        if(apply){
            const auto session=fs::path(value(L"--apply-update"));
            if(!inside(session,root/L".updates")||!fs::is_directory(session)||fs::is_symlink(session))throw std::runtime_error("Update staging folder is outside this installation.");
            sessionOwned=session;
        }
        if(!noLaunch)showProgress();progress(L"Waiting for the editor to close…",5);
        const auto pidText=value(L"--wait-pid");DWORD pid=pidText.empty()?0:DWORD(std::stoul(pidText));waitForEditor(pid);editorClosed=true;
        // Qt's process lock must also be absent; a different editor could have opened.
        if(fs::exists(root/L".potato-runtime.lock"))throw std::runtime_error("An editor session is still open. No update was applied.");
        const auto old=selected(root,L"current.txt");std::string next;
        if(rollback){next=selected(root,L"previous.txt");if(next==old||!fs::is_regular_file(root/L"versions"/next/L"PotatoMapperApp.exe"))throw std::runtime_error("There is no previous application version to restore.");}
        else{
            const auto text=value(L"--version");next.clear();for(auto c:text){if(c>127)throw std::runtime_error("Invalid update version.");next.push_back(char(c));}if(!versionValid(next)||next==old)throw std::runtime_error("Invalid update version.");
            const auto session=fs::path(value(L"--apply-update"));
            if(!inside(session,root/L".updates")||!fs::is_directory(session))throw std::runtime_error("Update staging folder is outside this installation.");
            const auto source=session/L"payload"/L"versions"/next;
            if(!inside(source,session)||!fs::is_regular_file(source/L"PotatoMapperApp.exe")||read(source/L"runtime-version.txt")!=next+"\n")throw std::runtime_error("The prepared update is incomplete.");
            const auto destination=root/L"versions"/next;
            const auto pending=root/L"versions"/(".installing-"+next+"-"+std::to_string(GetCurrentProcessId()));
            std::vector<fs::path> files;
            for(const auto &entry:fs::recursive_directory_iterator(source)){
                if(entry.is_symlink()||!inside(entry.path(),source))throw std::runtime_error("Invalid file in prepared update.");
                if(entry.is_regular_file())files.push_back(entry.path());
            }
            if(fs::exists(destination)){
                // A retained version may be selected again after rollback. Reuse it
                // only when every runtime file matches the verified download.
                if(!inside(destination,root/L"versions")||fs::is_symlink(destination))throw std::runtime_error("Invalid retained version folder.");
                size_t count=0;for(const auto &e:fs::recursive_directory_iterator(destination)){
                    if(e.is_symlink()||!inside(e.path(),destination))throw std::runtime_error("Invalid retained runtime file.");
                    if(e.is_regular_file())++count;
                }
                if(count!=files.size())throw std::runtime_error("The retained version is incomplete. Extract a fresh release folder.");
                for(const auto &file:files){const auto target=destination/fs::relative(file,source);if(!fs::is_regular_file(target)||read(target)!=read(file))throw std::runtime_error("The retained version differs from this release. Extract a fresh release folder.");}
            }else{
                if(fs::exists(pending))throw std::runtime_error("An installation staging folder already exists. Try again.");
                fs::create_directories(pending);pendingOwned=pending;
                size_t copied=0;for(const auto &file:files){const auto target=pending/fs::relative(file,source);fs::create_directories(target.parent_path());fs::copy_file(file,target);progress(L"Installing the new version. Your projects stay in place…",10+int(75*++copied/files.size()));}
                fs::rename(pending,destination);pendingOwned.clear();
            }
        }
        progress(L"Selecting the new application version…",90);
        restoreVersion=old;previousExisted=fs::exists(root/L"previous.txt");priorPrevious=read(root/L"previous.txt");pointersTouched=true;
        writePointer(root,L"previous.txt",old);writePointer(root,L"current.txt",next);
        if(!sessionOwned.empty()&&inside(sessionOwned,root/L".updates")){std::error_code ignored;fs::remove_all(sessionOwned,ignored);sessionOwned.clear();}
        progress(L"Opening Potato Mapper…",100);
        if(progressWindow)DestroyWindow(progressWindow);
        if(!noLaunch){std::vector<std::wstring> reopen;auto project=value(L"--project");if(!project.empty()){reopen={L"--project",project};}launch(root,reopen,false);}
        CloseHandle(lock);lock=INVALID_HANDLE_VALUE;
        return 0;
    }catch(const std::exception &error){
        if(!pendingOwned.empty()&&inside(pendingOwned,root/L"versions")){std::error_code ignored;fs::remove_all(pendingOwned,ignored);}
        if(!sessionOwned.empty()&&inside(sessionOwned,root/L".updates")){std::error_code ignored;fs::remove_all(sessionOwned,ignored);}
        if(pointersTouched){
            try{writePointer(root,L"current.txt",restoreVersion);}catch(...){}
            try{if(previousExisted)writeAtomic(root,L"previous.txt",priorPrevious);else fs::remove(root/L"previous.txt");}catch(...){}
        }
        if(progressWindow)DestroyWindow(progressWindow);
        // Recover a failed update automatically; an ordinary failed launch can
        // offer the retained version without altering any project files.
        if(!noLaunch){
            if(!has(L"--test-recovery"))MessageBoxW(nullptr,wide(error.what()).c_str(),L"Potato Mapper",MB_OK|MB_ICONERROR);
            try{
                if(restoreVersion.empty()&&!editorClosed&&!apply&&!rollback&&lock!=INVALID_HANDLE_VALUE){
                    const auto previous=selected(root,L"previous.txt");
                    if(MessageBoxW(nullptr,L"Open the previous application version? Your saved projects stay in place.",L"Potato Mapper — Recovery",MB_YESNO|MB_ICONQUESTION)==IDYES){writePointer(root,L"current.txt",previous);editorClosed=true;}
                }
                if(editorClosed){auto project=value(L"--project");launch(root,project.empty()?std::vector<std::wstring>{}:std::vector<std::wstring>{L"--project",project},false);}
            }catch(...){}
        }
        if(lock!=INVALID_HANDLE_VALUE)CloseHandle(lock);
        return 2;
    }
}
