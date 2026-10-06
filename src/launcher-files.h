#pragma once
// Shared native checks: neither the portable launcher nor its refresh helper
// depends on Qt or on DLLs beside the root executable.
#include <windows.h>
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace potatoLauncher {
namespace fs = std::filesystem;
inline std::string read(const fs::path &path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}
inline bool versionValid(const std::string &version) {
    return std::regex_match(version, std::regex("[0-9]{1,5}\\.[0-9]{1,5}\\.[0-9]{1,5}"));
}
inline bool plainPath(const fs::path &path) {
    // Reject junctions as well as symbolic links, including linked ancestors.
    for (auto p=fs::absolute(path); !p.empty(); p=p.parent_path()) {
        const auto attributes=GetFileAttributesW(p.c_str());
        if(attributes!=INVALID_FILE_ATTRIBUTES && (attributes&FILE_ATTRIBUTE_REPARSE_POINT))return false;
        if(p==p.parent_path())break;
    }
    return true;
}
inline std::string sha256(const fs::path &path) {
    std::ifstream in(path,std::ios::binary);
    if(!in)throw std::runtime_error("Could not read a managed program file for verification.");
    BCRYPT_ALG_HANDLE algorithm=nullptr; BCRYPT_HASH_HANDLE hash=nullptr;
    if(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)
        throw std::runtime_error("Windows could not open SHA-256 verification.");
    DWORD size=0,returned=0;
    auto cleanup=[&](){if(hash)BCryptDestroyHash(hash);BCryptCloseAlgorithmProvider(algorithm,0);};
    if(BCryptGetProperty(algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),sizeof(size),&returned,0)<0){cleanup();throw std::runtime_error("Windows could not prepare SHA-256 verification.");}
    std::vector<unsigned char> object(size);
    if(BCryptCreateHash(algorithm,&hash,object.data(),size,nullptr,0,0)<0){cleanup();throw std::runtime_error("Windows could not prepare SHA-256 verification.");}
    std::array<char,65536> chunk{};
    while(in){in.read(chunk.data(),std::streamsize(chunk.size()));const auto n=in.gcount();if(n&&BCryptHashData(hash,reinterpret_cast<PUCHAR>(chunk.data()),ULONG(n),0)<0){cleanup();throw std::runtime_error("Windows could not verify a program file.");}}
    if(!in.eof()){cleanup();throw std::runtime_error("A program file could not be fully read.");}
    std::array<unsigned char,32> digest{};
    if(BCryptFinishHash(hash,digest.data(),ULONG(digest.size()),0)<0){cleanup();throw std::runtime_error("Windows could not finish program verification.");}
    cleanup();std::string result;constexpr char hex[]="0123456789abcdef";
    for(const auto b:digest){result+=hex[b>>4];result+=hex[b&15];}return result;
}
inline bool completeRuntime(const fs::path &runtime,const std::string &version) {
    if(!versionValid(version)||!plainPath(runtime)||!fs::is_regular_file(runtime/L"PotatoMapperApp.exe")
        ||read(runtime/L"runtime-version.txt")!=version+"\n")return false;
    const auto manifest=runtime/L"runtime-files.sha256";
    if(!fs::exists(manifest)) {
        // Earlier protocol-1 releases have no retained manifest. Require their
        // editor, version marker, and central Qt dependency before recovery.
        return fs::is_regular_file(runtime/L"Qt6Core.dll")&&plainPath(runtime/L"PotatoMapperApp.exe")&&plainPath(runtime/L"Qt6Core.dll");
    }
    if(!plainPath(manifest)||!fs::is_regular_file(manifest)||fs::file_size(manifest)>4*1024*1024)return false;
    std::istringstream lines(read(manifest));std::string line;bool editor=false,marker=false;size_t count=0;
    try {
        while(std::getline(lines,line)) {
            if(!line.empty()&&line.back()=='\r')line.pop_back();
            if(line.size()<67||line[64]!='\t'||!std::regex_match(line.substr(0,64),std::regex("[a-f0-9]{64}")))return false;
            const auto name=line.substr(65);const auto relative=fs::u8path(name);
            if(name.empty()||name.find('\\')!=std::string::npos||name.find(':')!=std::string::npos||relative.is_absolute())return false;
            for(const auto &part:relative)if(part==L".."||part==L".")return false;
            const auto file=runtime/relative;
            if(!plainPath(file)||!fs::is_regular_file(file)||sha256(file)!=line.substr(0,64))return false;
            editor=editor||name=="PotatoMapperApp.exe";marker=marker||name=="runtime-version.txt";
            if(++count>20000)return false;
        }
    }catch(...){return false;}
    return editor&&marker&&count>2;
}
}
