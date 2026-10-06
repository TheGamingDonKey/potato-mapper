#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include "launcher-files.h"

namespace fs=std::filesystem;
static void clearFailureReport(const fs::path &root){
    const auto report=root/L".maintenance"/L"launcher-refresh-error.txt";
    std::error_code ignored;
    if(potatoLauncher::plainPath(report)&&fs::is_regular_file(report,ignored))fs::remove(report,ignored);
}
static std::wstring wide(const std::string &s){const int n=MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),nullptr,0);std::wstring out(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),out.data(),n);return out;}
static void failureReport(const fs::path &root,const std::string &message) {
    try {
        const auto maintenance=root/L".maintenance";
        if(!potatoLauncher::plainPath(maintenance))return;
        fs::create_directories(maintenance);
        const auto temporary=maintenance/(L"launcher-refresh-error-"+std::to_wstring(GetCurrentProcessId())+L".tmp");
        std::ofstream out(temporary,std::ios::binary|std::ios::trunc);
        out<<"Potato Mapper launcher maintenance failed.\n"<<message<<"\n\nYour projects and retained application versions were not removed.\n"
           <<"Close Potato Mapper and reopen the main-folder launcher to retry, or extract a complete release ZIP into a new folder.\n";
        out.close();if(out)MoveFileExW(temporary.c_str(),(maintenance/L"launcher-refresh-error.txt").c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
    }catch(...){}
}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int) {
    int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);fs::path root;std::string version;
    for(int i=1;i+1<argc;++i){if(std::wstring(argv[i])==L"--install-root")root=argv[++i];else if(std::wstring(argv[i])==L"--version"){for(const auto c:std::wstring(argv[++i])){if(c>127){version.clear();break;}version+=char(c);}}}LocalFree(argv);
    HANDLE lock=INVALID_HANDLE_VALUE;fs::path temporary,backup;bool trustedRoot=false;
    try {
        if(root.empty()||!root.is_absolute()||!potatoLauncher::versionValid(version))throw std::runtime_error("Invalid launcher maintenance arguments.");
        root=fs::weakly_canonical(root);
        if(!potatoLauncher::plainPath(root)||potatoLauncher::read(root/L"installation.txt")!="PotatoMapper/1\n")throw std::runtime_error("This folder is not a supported portable installation.");
        trustedRoot=true;
        const auto runtime=root/L"versions"/version;
        std::vector<wchar_t> ownPath(32768);GetModuleFileNameW(nullptr,ownPath.data(),DWORD(ownPath.size()));
        if(!potatoLauncher::plainPath(runtime)||fs::weakly_canonical(fs::path(ownPath.data()).parent_path())!=fs::weakly_canonical(runtime))throw std::runtime_error("The launcher refresh helper is outside the selected runtime.");
        const auto source=runtime/L"launcher"/L"PotatoMapper.exe",destination=root/L"PotatoMapper.exe";
        if(!potatoLauncher::plainPath(source)||!potatoLauncher::plainPath(destination)||!fs::is_regular_file(source))throw std::runtime_error("A launcher path is linked, missing, or invalid.");
        const auto digest=potatoLauncher::sha256(source);
        auto expected=potatoLauncher::read(runtime/L"launcher"/L"launcher-sha256.txt");while(!expected.empty()&&(expected.back()=='\n'||expected.back()=='\r'))expected.pop_back();
        if(expected!=digest)throw std::runtime_error("The replacement launcher failed SHA-256 verification.");
        if(fs::is_regular_file(destination)&&potatoLauncher::sha256(destination)==digest){clearFailureReport(root);return 0;}
        const auto deadline=GetTickCount64()+30000;
        do {
            lock=CreateFileW((root/L".update-lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_HIDDEN,nullptr);
            if(lock!=INVALID_HANDLE_VALUE)break;
            Sleep(100);
        }while(GetTickCount64()<deadline);
        if(lock==INVALID_HANDLE_VALUE)throw std::runtime_error("Another launcher or update kept the installation busy for 30 seconds. The old launcher remains available.");
        // Check again after acquiring the same lock used by update and rollback.
        if(potatoLauncher::read(root/L"current.txt")!=version+"\n"||potatoLauncher::read(runtime/L"runtime-version.txt")!=version+"\n")throw std::runtime_error("The selected application changed while launcher maintenance was waiting.");
        const auto maintenance=root/L".maintenance";
        if(!potatoLauncher::plainPath(maintenance))throw std::runtime_error("The maintenance folder is linked to another location.");
        fs::create_directories(maintenance);
        const auto session=maintenance/(L"launcher-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
        if(!fs::create_directory(session))throw std::runtime_error("Could not create a fresh launcher maintenance folder.");
        temporary=session/L"replacement.exe";backup=session/L"previous-PotatoMapper.exe";
        fs::copy_file(source,temporary);
        if(potatoLauncher::sha256(temporary)!=digest)throw std::runtime_error("The staged launcher copy failed verification. The old launcher was retained.");
        HANDLE staged=CreateFileW(temporary.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(staged==INVALID_HANDLE_VALUE)throw std::runtime_error("Could not flush the staged launcher to disk.");
        const bool flushed=FlushFileBuffers(staged)!=FALSE;CloseHandle(staged);
        if(!flushed)throw std::runtime_error("Could not finish writing the staged launcher. Check free space.");
        DWORD lastError=0;bool replaced=false;
        do {
            if(fs::exists(destination)) {
                if(!fs::is_regular_file(destination)||!potatoLauncher::plainPath(destination))throw std::runtime_error("The root launcher is not a regular program file.");
                replaced=ReplaceFileW(destination.c_str(),temporary.c_str(),backup.c_str(),0,nullptr,nullptr)!=FALSE;
            }else replaced=MoveFileExW(temporary.c_str(),destination.c_str(),MOVEFILE_WRITE_THROUGH)!=FALSE;
            if(replaced)break;
            lastError=GetLastError();
            // If Windows has already moved the old executable to its backup,
            // stop and recover it instead of overwriting that recovery copy.
            if(fs::exists(backup))break;
            if(lastError!=ERROR_SHARING_VIOLATION&&lastError!=ERROR_ACCESS_DENIED&&lastError!=ERROR_LOCK_VIOLATION)break;
            Sleep(100);
        }while(GetTickCount64()<deadline);
        if(!replaced)throw std::runtime_error("Windows could not replace the root launcher (error "+std::to_string(lastError)+"). Close other copies and retry. Its previous executable is retained.");
        temporary.clear();
        clearFailureReport(root);CloseHandle(lock);lock=INVALID_HANDLE_VALUE;return 0;
    }catch(const std::exception &error) {
        // ReplaceFile normally leaves the destination intact on failure. Some
        // filesystem failures can leave it at the backup path; restore that
        // previous executable without replacing any existing destination.
        std::error_code ignored;
        if(!backup.empty()&&!fs::exists(root/L"PotatoMapper.exe",ignored)&&fs::is_regular_file(backup,ignored))MoveFileExW(backup.c_str(),(root/L"PotatoMapper.exe").c_str(),MOVEFILE_WRITE_THROUGH);
        if(lock!=INVALID_HANDLE_VALUE)CloseHandle(lock);
        if(trustedRoot)failureReport(root,error.what());
        OutputDebugStringW((L"Potato Mapper: "+wide(error.what())).c_str());return 2;
    }
}
