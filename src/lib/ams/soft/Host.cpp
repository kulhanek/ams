// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include <Host.hpp>
#include <ctype.h>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <ShellProcessor.hpp>
#include <fnmatch.h>
#include <AMSGlobalConfig.hpp>
#include <FileSystem.hpp>
#include <Shell.hpp>
#include <Cuda.hpp>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <ostream>
#include <iomanip>
#include <limits>
#include <User.hpp>
#include <set>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <PrintEngine.hpp>

//------------------------------------------------------------------------------

// how long the cache is valid in seconds
#define CACHE_VALIDITY 86400

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CHost    Host;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHost::CHost(void)
{
    NCPUs = 1;
    NGPUs = 0;
    NNodes = 1;
    CacheLoaded = false;
    CacheTime = 0;
    ClearAll();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHost::InitGlobalSetup(void)
{
    // take hostname from HOSTNAME if it was not already set by SetHostName
    if( Hostname == NULL ){
        Hostname = CShell::GetSystemVariable("HOSTNAME");
    } else {
        if( Hostname == CShell::GetSystemVariable("HOSTNAME") ){
            AlienHost = false;
        }
    }

    // init default data -------------------------
    NCPUs = 1;
    CSmallString sNCPU;
    sNCPU = CShell::GetSystemVariable("AMS_NCPUS");
    if( sNCPU != NULL ) {
        NCPUs = sNCPU.ToInt();
    }

    NGPUs = 0;
    CSmallString sNGPU;
    sNGPU = CShell::GetSystemVariable("AMS_NGPUS");
    if( sNGPU != NULL ) {
        NGPUs = sNGPU.ToInt();
    }

    NNodes = 1;
    CSmallString sNNodes;
    sNNodes = CShell::GetSystemVariable("AMS_NNODES");
    if( sNNodes != NULL ) {
        NNodes = sNNodes.ToInt();
    }
}

//------------------------------------------------------------------------------

void CHost::InitHostFile(void)
{
    CFileName    config_name;

    config_name = AMSGlobalConfig.GetETCDIR() / "default" / "hosts.xml";
    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Hosts);

    if( xml_parser.Parse(config_name) == false ) {
        CSmallString    error;
        error << "unable to load '" << config_name << "'";
        RUNTIME_ERROR(error);
    }

    CXMLElement* p_mele = Hosts.GetFirstChildElement("config");
    if( p_mele ){
        p_mele->GetAttribute("key",ConfigKey);
    }
}

//------------------------------------------------------------------------------

void CHost::ClearAll(void)
{
    Hosts.RemoveAllChildNodes();
    //NCPUs = 1; // global data - do not update here
    //NGPUs = 0;
    //NNodes = 1;
    NumOfHostCPUs = 1;
    NumOfHostGPUs = 0;
    CPUModelName = "";
    GPUModelNames.clear();
    DefaultTokens.clear();
    HostTokens.clear();
    CPUInfoTokens.clear();
    CompatTokens.clear();
    GPUInfoTokens.clear();
    CUDATokens.clear();
    AllTokens.clear();

    CudaFilter = "-none-";
    CudaLib = "-none-";

    NetFilters = "-none-";

    DefaultNumOfHostCPUs = 0;
    HostNumOfHostCPUs = 0;
    CPUInfoNumOfHostCPUs = 0;

    ConfigRealm  = NULL;
    ConfigKey = NULL;

    CacheLoaded = false;
    CacheTime = 0;

    HTDetected = false;
    HTEnabled = false;

    IsDesktop = false;
    DesktopCPUPenalty = 1;

    CPUSpec = 1.0;
}

//------------------------------------------------------------------------------

void CHost::LoadCache(void)
{
    CXMLDocument xml_document;

    // try to load cache
    CFileName file_name = "/tmp/ams_cache." + User.GetName();

    if( CFileSystem::IsFile(file_name) == false ){
        // no cache file
        CSmallString warning;
        warning << "no cache file '" << file_name << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&xml_document);
    if( xml_parser.Parse(file_name) == false ){
        ErrorSystem.RemoveAllErrors(); // avoid global error
        CSmallString warning;
        warning << "unable to parse cache '" << file_name << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLElement* p_ele = xml_document.GetFirstChildElement("cache");
    if( p_ele == NULL ) return;

// master ID -----------
    // this is id from the host file
    CSmallString cache_key;

    p_ele->GetAttribute("key",cache_key);
    if( cache_key != ConfigKey ){
        ES_WARNING("master/cache keys mismatch");
        return;
    }

    // this is current time
    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();
    p_ele->GetAttribute("lid",CacheTime);
    if( current_time.GetSecondsFromBeginning() > CacheTime + CACHE_VALIDITY ){
        ES_WARNING("cache too old");
        return;
    }

// header --------------
    CXMLElement* p_hele = p_ele->GetFirstChildElement("header");
    if( p_hele ){
        p_hele->GetAttribute("nhcpu",NumOfHostCPUs);
        p_hele->GetAttribute("cpum",CPUModelName);
        p_hele->GetAttribute("nhgpu",NumOfHostGPUs);
        string sbuf;
        p_hele->GetAttribute("gpum",sbuf);
        if( ! sbuf.empty() ) split(GPUModelNames,sbuf,is_any_of("|"));
    }

// desktop -------------
    CXMLElement* p_dsele = p_ele->GetFirstChildElement("desktop");
    if( p_dsele ){
        p_dsele->GetAttribute("st",IsDesktop);
        p_dsele->GetAttribute("pn",DesktopCPUPenalty);
    }

// cpuinfo -------------
    CXMLElement* p_cele = p_ele->GetFirstChildElement("cpuinfo");
    if( p_cele ){
        p_cele->GetAttribute("htd",HTDetected);
        p_cele->GetAttribute("hte",HTEnabled);
        string sbuf;
        p_cele->GetAttribute("tks",sbuf);
        if( ! sbuf.empty() ) split(CPUInfoTokens,sbuf,is_any_of("#"));
        p_cele->GetAttribute("ncpus",CPUInfoNumOfHostCPUs);
        string flags;
        p_cele->GetAttribute("fl",flags);
        split(CPUInfoFlags,flags,is_any_of(","));
        p_cele->GetAttribute("sp",CPUSpec);
    }

// cuda ----------------
    CXMLElement* p_nele = p_ele->GetFirstChildElement("cuda");
    if( p_nele ){
        string sbuf;
        p_nele->GetAttribute("tks",sbuf);
        if( ! sbuf.empty() ) split(CUDATokens,sbuf,is_any_of("#"));
        p_nele->GetAttribute("flt",CudaFilter);
        p_nele->GetAttribute("lib",CudaLib);
        string cap;
        p_nele->GetAttribute("cap",cap);
        split(GPUCapabilities,cap,is_any_of(","));
    }

// network --------------
    CXMLElement* p_dele = p_ele->GetFirstChildElement("net");
    if( p_dele ){
        string sbuf;
        p_dele->GetAttribute("dev",sbuf);
        if( ! sbuf.empty() ) split(NetDevs,sbuf,is_any_of("#"));
        p_dele->GetAttribute("flt",NetFilters);
        sbuf.clear();
        p_dele->GetAttribute("tks",sbuf);
        if( ! sbuf.empty() ) split(NetTokens,sbuf,is_any_of("#"));
    }

    CacheLoaded = true;
}

//------------------------------------------------------------------------------

void CHost::SaveCache(void)
{
    CXMLDocument xml_document;
    xml_document.CreateChildDeclaration();
    xml_document.CreateChildComment("AMS host cache file");
    CXMLElement* p_ele = xml_document.CreateChildElement("cache");

// master key ------------
    // this is id from the host file
    p_ele->SetAttribute("key",ConfigKey);

    // this is current time
    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();
    p_ele->SetAttribute("lid",current_time.GetSecondsFromBeginning());

// header --------------
    CXMLElement* p_hele = p_ele->CreateChildElement("header");
    // common
    p_hele->SetAttribute("nhcpu",NumOfHostCPUs);
    p_hele->SetAttribute("cpum",CPUModelName);
    p_hele->SetAttribute("nhgpu",NumOfHostGPUs);
    p_hele->SetAttribute("gpum",join(GPUModelNames,"|"));

// desktop -------------
    CXMLElement* p_dsele = p_ele->CreateChildElement("desktop");
    p_dsele->SetAttribute("st",IsDesktop);
    p_dsele->SetAttribute("pn",DesktopCPUPenalty);

// cpuinfo -------------
    CXMLElement* p_cele = p_ele->CreateChildElement("cpuinfo");
    p_cele->SetAttribute("htd",HTDetected);
    p_cele->SetAttribute("hte",HTEnabled);
    p_cele->SetAttribute("tks",join(CPUInfoTokens,"#"));
    p_cele->SetAttribute("ncpus",CPUInfoNumOfHostCPUs);
    p_cele->SetAttribute("fl",join(CPUInfoFlags,","));
    p_cele->SetAttribute("sp",CPUSpec);

// cuda ----------------
    CXMLElement* p_nele = p_ele->CreateChildElement("cuda");
    p_nele->SetAttribute("tks",join(CUDATokens,"#"));
    p_nele->SetAttribute("flt",CudaFilter);
    p_nele->SetAttribute("lib",CudaLib);
    p_nele->SetAttribute("cap",join(GPUCapabilities,","));

// special -------------
    CXMLElement* p_dele = p_ele->CreateChildElement("net");
    p_dele->SetAttribute("dev",join(NetDevs,"#"));
    p_dele->SetAttribute("flt",NetFilters);
    p_dele->SetAttribute("tks",join(NetTokens,"#"));

    // save cache
    CFileName file_name = "/tmp/ams_cache." + User.GetName();

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&xml_document);
    if( xml_printer.Print(file_name) == false ){
        CSmallString warning;
        warning << "unable to save cache '" << file_name << "'";
        ES_WARNING(warning);
    }
}

//------------------------------------------------------------------------------

void CHost::InitHost(int ncpus,int ngpus)
{
    NCPUs = ncpus;
    NGPUs = ngpus;
    InitHost();
}

//------------------------------------------------------------------------------

CXMLElement* CHost::FindGroup(void)
{
    bool personal = CShell::GetSystemVariable("AMS_PERSONAL") == "ON" ;
    return(FindGroup(Hostname,personal));
}

//------------------------------------------------------------------------------

CXMLElement* CHost::FindGroup(const CSmallString& hostname,bool personal)
{

//    CSmallString info;
//    info << "hostname: " << Hostname;
//    ES_WARNING(info);

    CXMLElement* p_gele = Hosts.GetChildElementByPath("config/groups/group");
    while( p_gele != NULL ){
        CSmallString name;
        p_gele->GetAttribute("name",name);
//        CSmallString info;
//        info << "group: " << name;
//        ES_WARNING(info);
        CXMLElement* p_host = p_gele->GetChildElementByPath("hosts/host");
        while( p_host != NULL ){
            CSmallString name;
            p_host->GetAttribute("name",name);
//            CSmallString info;
//            info << "  host: " << name;
//            ES_WARNING(info);
            if( personal ){
                if( name == "personal" ) return(p_gele);
            } else {
                if( fnmatch(name,hostname,0) == 0){
//                    CSmallString info;
//                    info << "HIT: " << name;
//                    ES_WARNING(info);
                    return(p_gele);
                }
            }
            p_host = p_host->GetNextSiblingElement();
        }

        p_gele = p_gele->GetNextSiblingElement();
    }

    return(p_gele);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetGroupNS(void)
{
    CXMLElement* p_grp = FindGroup();
    if( p_grp == NULL ) return("na");
    CSmallString gns("na");
    p_grp->GetAttribute("groupns",gns);
    return(gns);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetGroupNS(const CSmallString& hostname,bool personal)
{
    CXMLElement* p_grp = FindGroup(hostname,personal);
    if( p_grp == NULL ) return("na");
    CSmallString gns("na");
    p_grp->GetAttribute("groupns",gns);
    return(gns);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetRealm(void)
{
    CXMLElement* p_grp = FindGroup();
    if( p_grp == NULL ) return("NONE");
    CSmallString realm("NONE");
    p_grp->GetAttribute("realm",realm);
    return(realm);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetRealm(const CSmallString& hostname,bool personal)
{
    CXMLElement* p_grp = FindGroup(hostname,personal);
    if( p_grp == NULL ) return("NONE");
    CSmallString realm("NONE");
    p_grp->GetAttribute("realm",realm);
    return(realm);
}

//------------------------------------------------------------------------------

void CHost::InitHost(bool nocache)
{
    CPUModelName="unknown";

    // try to load cache
    if( nocache == false) LoadCache();

    // parse config file -------------------------
    CXMLElement* p_aele = NULL;

    // try to load group specific setup
    CXMLElement* p_gele = FindGroup();
    if( p_gele != NULL ){
        p_gele->GetAttribute("name",ConfigRealm);
        if( ConfigRealm == NULL ) ConfigRealm = "group-specific";
        p_aele = p_gele->GetChildElementByPath("archs");
    }

    // if not exists then use default
    if( p_aele == NULL ){
        // use default
        p_aele = Hosts.GetChildElementByPath("config/archs");
        ConfigRealm = NULL;
    }

    CXMLElement* p_ele = NULL;
    if( p_aele != NULL ){
        p_ele = p_aele->GetFirstChildElement();
    }

    while( p_ele ){
        if( p_ele->GetName() == "default" ){
            InitDefaultTokens(p_ele);
        }
        if( p_ele->GetName() == "hosts" ){
            InitHostsTokens(p_ele);
        }
        if(  (p_ele->GetName() == "cpuinfo") &&
             (AlienHost == false) && (CacheLoaded == false) ){
            InitCPUInfoTokens(p_ele);
        }
        if(  (p_ele->GetName() == "desktop") &&
             (AlienHost == false) && (CacheLoaded == false) ){
            InitDesktopTokens(p_ele);
        }
        if( (p_ele->GetName() == "gpuinfo") && (AlienHost == false) ){
            InitGPUInfoTokens(p_ele);
        }
        if( (p_ele->GetName() == "cuda") &&
             (AlienHost == false) && (CacheLoaded == false) ){
            InitCudaGPUTokens(p_ele);
        }
        if( (p_ele->GetName() == "net") &&
             (AlienHost == false) && (CacheLoaded == false) ){
            InitNetworkTokens(p_ele);
        }
        p_ele = p_ele->GetNextSiblingElement();
    }

    InitCompatibilityTokens(p_aele);

    // merge all tokens
    for(size_t i=0; i < DefaultTokens.size(); i++){
        AllTokens.push_back(DefaultTokens[i]);
    }
    for(size_t i=0; i < HostTokens.size(); i++){
        AllTokens.push_back(HostTokens[i]);
    }
    for(size_t i=0; i < CompatTokens.size(); i++){
        AllTokens.push_back(CompatTokens[i]);
    }
    for(size_t i=0; i < CPUInfoTokens.size(); i++){
        AllTokens.push_back(CPUInfoTokens[i]);
    }
    for(size_t i=0; i < GPUInfoTokens.size(); i++){
        AllTokens.push_back(GPUInfoTokens[i]);
    }
    for(size_t i=0; i < CUDATokens.size(); i++){
        AllTokens.push_back(CUDATokens[i]);
    }
    for(size_t i=0; i < NetTokens.size(); i++){
        AllTokens.push_back(NetTokens[i]);
    }

    // sort tokens
    AllTokens.sort();
    AllTokens.unique();

    // save host cache
    if( (AlienHost == false) &&
        (CacheLoaded == false) &&
        (User.IsUserNameProvided() == false) ){
        SaveCache();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHost::InitDefaultTokens(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    // tokens
    string value;
    std::vector<string> tokens;
    if( p_ele->GetAttribute("tokens",value) ){
        split(tokens,value,is_any_of("#"));
    }

    // max cpus per node
    p_ele->GetAttribute("ncpus",DefaultNumOfHostCPUs);
    if( CacheLoaded == false ) NumOfHostCPUs = DefaultNumOfHostCPUs;

    // copy tokens
    vector<string>::iterator  it = tokens.begin();
    vector<string>::iterator  ie = tokens.end();
    while( it != ie ){
        DefaultTokens.push_back(*it);
        it++;
    }
}

//------------------------------------------------------------------------------

void CHost::InitHostsTokens(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    CXMLElement* p_hele = p_ele->GetFirstChildElement("node");
    while( p_hele != NULL ){
        CSmallString name;
        p_hele->GetAttribute("name",name);
        if( fnmatch(name,Hostname,0) == 0 ){
            // node found exit
            break;
        }
        p_hele = p_hele->GetNextSiblingElement("node");
    }

    if( p_hele == NULL ) return;

    // -------------------------------------------

    // tokens
    string value;
    std::vector<string> tokens;
    if( p_hele->GetAttribute("tokens",value) ){
        split(tokens,value,is_any_of("#"));
    }

    // max cpus per node
    bool cenable = false;
    p_ele->GetAttribute("ecpus",cenable);
    if( cenable ) {
        p_ele->GetAttribute("ncpus",HostNumOfHostCPUs);
        if( CacheLoaded == false ) NumOfHostCPUs = HostNumOfHostCPUs;
    }

    // copy tokens
    bool tenable = false;
    p_ele->GetAttribute("tokens",tenable);
    if( tenable ) {
        vector<string>::iterator  it = tokens.begin();
        vector<string>::iterator  ie = tokens.end();
        while( it != ie ){
            HostTokens.push_back(*it);
            it++;
        }
    }
}

//------------------------------------------------------------------------------

void CHost::InitCPUInfoTokens(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    ifstream cpuinfo;

    // open /proc/cpuinfo file
    cpuinfo.open("/proc/cpuinfo");
    if( ! cpuinfo ){
        RUNTIME_ERROR("unable to open /proc/cpuinfo");
    }

    // parse CPU attributes
    string line;
    bool arch_found = false;

    int count_CPU = 0;
    HTDetected = false;
    int phys_CPUs = 0;
    int cpu_cores = 0;

    list<int> physIds;
    list<int> coreIds;

    while( getline(cpuinfo,line) ){
        vector<string>  key_and_value;
        split(key_and_value,line,is_any_of(":"));
        if( key_and_value.size() != 2 ) continue;

        string key = key_and_value[0];
        string values = key_and_value[1];

        trim(key);
        trim(values);

        if( key == "model name" ){
            vector<string> words;
            split(words,values,is_any_of(" "),token_compress_on);
            CPUModelName =  join(words," ");
            count_CPU++;
            continue;
        }

        if( key == "physical id" ){
            vector<string> words;
            split(words,values,is_any_of(" "),token_compress_on);
            if( words.size() == 1 ){
                int l_physCPU = atoi(words[0].c_str());
                physIds.push_back(l_physCPU);
            }
            continue;
        }

        if( key == "core id" ){
            vector<string> words;
            split(words,values,is_any_of(" "),token_compress_on);
            if( words.size() == 1 ){
                int l_cpu_core = atoi(words[0].c_str());
                coreIds.push_back(l_cpu_core);
            }
            continue;
        }

        if( (key == "flags") && (arch_found == false) ){
            split(CPUInfoFlags,values,is_any_of(" "),token_compress_on);
            arch_found = true;
            continue;
        }
    }

    // close file
    cpuinfo.close();


    // get total mem
    double mem = 0;
    std::string token;
    std::ifstream file("/proc/meminfo");
    while(file >> token) {
        if(token == "MemTotal:") {
            file >> mem;
            break;
        }
        // ignore rest of the line
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    if( mem > 0 ){
        stringstream str;
        mem = mem / 1024 ; // kbytes to MB
        if( mem < 1024 ){
            str << " [Total memory: " << fixed << setprecision(1) << mem << " MB" << "]";
        } else {
            mem = mem / 1024;
            if( mem < 1024 ){
                str << " [Total memory: " << fixed << setprecision(1) << mem << " GB" << "]";
            } else {
                mem = mem / 1024;
                str << " [Total memory: " << fixed << setprecision(1) << mem << " TB" << "]";
            }
        }
        CPUModelName << str.str();
    }

    // on virtualized machines ids are not sequential numbers
    physIds.sort();
    physIds.unique();
    coreIds.sort();
    coreIds.unique();

    phys_CPUs = physIds.size();
    cpu_cores = coreIds.size();

    // is HT supported and enabled?
    if( (cpu_cores*phys_CPUs != 0) && (cpu_cores*phys_CPUs < count_CPU) ){
        HTDetected = true;
    }
    HTEnabled = false;
    p_ele->GetAttribute("ht",HTEnabled);
    HTEnabled &= HTDetected;

    if( (HTEnabled == false) && (cpu_cores*phys_CPUs != 0) ){
        count_CPU = cpu_cores*phys_CPUs;
    }

    // max cpus per node
    bool cenable = false;
    p_ele->GetAttribute("ecpus",cenable);
    if( cenable ) {
        CPUInfoNumOfHostCPUs = count_CPU;
        NumOfHostCPUs = count_CPU;
    }

    // filter CPU tokens
    bool tenable = false;
    p_ele->GetAttribute("tokens",tenable);
    if( tenable ) {
        CXMLElement* p_fele = p_ele->GetFirstChildElement("filter");
        while( p_fele != NULL ){
            CSmallString filter;
            p_fele->GetAttribute("value",filter);
            vector<string>::iterator  it = CPUInfoFlags.begin();
            vector<string>::iterator  ie = CPUInfoFlags.end();
            while( it != ie ){
                if( fnmatch(filter,(*it).c_str(),0) == 0 ){
                    CPUInfoTokens.push_back(*it);
                }
                it++;
            }
            p_fele = p_fele->GetNextSiblingElement("filter");
        }
    }

    // determine spec
    CXMLElement* p_spele = Hosts.GetChildElementByPath("config/cpu_specs");
    if( p_spele != NULL ){
        CXMLElement* p_iele = p_spele->GetFirstChildElement("model");
        while( p_iele != NULL ){
            CSmallString model;
            float        spec = 1.0;
            p_iele->GetAttribute("model",model);
            p_iele->GetAttribute("spec",spec);
            if( model == CPUModelName ){
                CPUSpec = spec;
            }
            p_iele = p_iele->GetNextSiblingElement("model");
        }
    }
}

//------------------------------------------------------------------------------

void CHost::InitCompatibilityTokens(CXMLElement* p_cele)
{
    std::list<string>  all_tokens;

    // merge all tokens
    for(size_t i=0; i < DefaultTokens.size(); i++){
        all_tokens.push_back(string(DefaultTokens[i]));
    }
    for(size_t i=0; i < HostTokens.size(); i++){
        all_tokens.push_back(string(HostTokens[i]));
    }
    for(size_t i=0; i < CPUInfoTokens.size(); i++){
        all_tokens.push_back(string(CPUInfoTokens[i]));
    }

    // sort tokens
    all_tokens.sort();
    all_tokens.unique();

    // add all compatibility tokens to HostTokens
    std::list<string>::iterator it = all_tokens.begin();
    std::list<string>::iterator ie = all_tokens.end();
    while( it != ie ){

        // try to find arch
        CXMLElement* p_arch = NULL;
        if( p_cele != NULL ){
            p_arch = p_cele->GetChildElementByPath("compat/arch");
        }
        while( p_arch ){
            string arch_name;
            p_arch->GetAttribute("name",arch_name);
            if( arch_name == (*it) ){
                // add all compatible roots into the hosts tokens
                CXMLElement* p_nele = p_arch->GetFirstChildElement("nextmatch");
                while( p_nele ){
                    CSmallString narch;
                    p_nele->GetAttribute("name",narch);
                    CompatTokens.push_back(string(narch));
                    p_nele = p_nele->GetNextSiblingElement("nextmatch");
                }
            }

            p_arch = p_arch->GetNextSiblingElement("arch");
        }

        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHost::InitGPUInfoTokens(CXMLElement* p_ele)
{
    GPUInfoTokens.clear();
    if( NGPUs > 0 ){
        // tokens
        string value;
        std::vector<string> tokens;
        if( p_ele->GetAttribute("tokens",value) ){
            split(tokens,value,is_any_of("#"));

            vector<string>::iterator  it = tokens.begin();
            vector<string>::iterator  ie = tokens.end();
            while( it != ie ){
                GPUInfoTokens.push_back(*it);
                it++;
            }
        }
    }
}

//------------------------------------------------------------------------------

void CHost::InitDesktopTokens(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( AlienHost == true ) return;

    p_ele->GetAttribute("penalty",DesktopCPUPenalty);

    CXMLElement* p_fele = p_ele->GetFirstChildElement("host");
    while( p_fele != NULL ){
        CSmallString filter,cmd;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("filter",filter);
        success &= p_fele->GetAttribute("cmd",cmd);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("host");

        if( success == false ){
            ES_WARNING("undefined filter and/or cmd attributes for host element");
            continue;
        }

        // does host match Hostname
        if( fnmatch(filter,Hostname,0) != 0 ){
            CSmallString warning;
            warning << "desktop: host '" << Hostname << "' does not match filter '" << filter << "'";
            ES_WARNING(warning);
            continue;
        }

        // execute command
        success = false;
        FILE* p_file = popen(cmd,"r");
        if( p_file != NULL ){
            success = pclose(p_file) == 0;
        }
        // was is OK?
        if( success ){
            IsDesktop = true;
            break;
        }
    }
}

//------------------------------------------------------------------------------

void CHost::InitCudaGPUTokens(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    CudaFilter = "-none-";
    CudaLib = "-none-";
    CUDATokens.clear();
    if( AlienHost == true ) return;

    bool capastokens = false;
    p_ele->GetAttribute("capastokens",capastokens);

    CXMLElement* p_fele = p_ele->GetFirstChildElement("host");
    while( p_fele != NULL ){
        CSmallString filter,cudalib;
        string stokens;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("filter",filter);
        success &= p_fele->GetAttribute("lib",cudalib);
        success &= p_fele->GetAttribute("tokens",stokens);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("host");

        if( success == false ){
            ES_WARNING("undefined filter, lib, or tokens attributes for host element");
            continue;
        }

        // does host match Hostname
        if( fnmatch(filter,Hostname,0) != 0 ){
            CSmallString warning;
            warning << "cuda: host '" << Hostname << "' does not match filter '" << filter << "'";
            ES_WARNING(warning);
            continue;
        }

        // try to load cuda lib
        CCuda cuda;
        if( cuda.Init(cudalib) == false ){
            CSmallString warning;
            warning << "unable to init cuda lib '" << cudalib << "'";
            ES_WARNING(warning);
            continue;
        }

        CudaFilter = filter;
        CudaLib = cudalib;

        // get number of GPU devices
        NumOfHostGPUs = cuda.GetNumOfGPUs();
        // get list of GPU devices
        cuda.GetGPUInfo(GPUModelNames,GPUCapabilities);

        if( NumOfHostGPUs > 0 ){
            // add gpu tokens if available and ngpus > 0
            std::vector<string> tokens;
            split(tokens,stokens,is_any_of("#"));

            vector<string>::iterator  it = tokens.begin();
            vector<string>::iterator  ie = tokens.end();
            while( it != ie ){
                CUDATokens.push_back(*it);
                it++;
            }
        }
        break;
    }

    if( capastokens ){
        // inject capabilities as cuda tokens
        CUDATokens.insert(CUDATokens.begin(),GPUCapabilities.begin(),GPUCapabilities.end());
    }
}

// -----------------------------------------------------------------------------

void CHost::InitNetworkTokens(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    // list available network interfaces
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        ES_WARNING("unable to get list of network devices");
        return;
    }

    // Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        NetDevs.insert(ifa->ifa_name);
    }

    freeifaddrs(ifaddr);

    NetTokens.clear();
    if( AlienHost == true ) return;

    int nfilt = 0;

    list<string> nettks;

    CXMLElement* p_fele = p_ele->GetFirstChildElement("iface");
    while( p_fele != NULL ){
        CSmallString filter;
        string stokens;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("filter",filter);
        success &= p_fele->GetAttribute("tokens",stokens);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("iface");

        if( success == false ){
            ES_WARNING("undefined filter or tokens attributes for iface element");
            continue;
        }

        // does match any device
        std::set<std::string>::iterator   dit = NetDevs.begin();
        std::set<std::string>::iterator   die = NetDevs.end();

        while( dit != die ){
            string dev = *dit;
            if( fnmatch(filter,dev.c_str(),0) == 0 ){
                if( nfilt == 0 ){
                    NetFilters = filter;
                } else {
                    NetFilters += ",";
                    NetFilters += filter;
                }
                nfilt++;
                // add dev tokens if available
                std::vector<string> tokens;
                split(tokens,stokens,is_any_of("#"));

                vector<string>::iterator  it = tokens.begin();
                vector<string>::iterator  ie = tokens.end();
                while( it != ie ){
                    nettks.push_back(*it);
                    it++;
                }
            } else {
                CSmallString warning;
                warning << "net device " << dev << " does not match filter " << filter;
                ES_WARNING(warning);
            }
            dit++;
        }

    }

    nettks.unique();
    std::copy( nettks.begin(), nettks.end(), std::back_inserter( NetTokens ) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CHost::GetRootParallelModes(void)
{
    // FIX ME - check if site does not have own specification
    CXMLElement* p_mele = Hosts.GetChildElementByPath("config/modes");
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to open hosts config/modes element");
    }
    return(p_mele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CHost::GetCPUModel(void) const
{
    return(CPUModelName);
}

//------------------------------------------------------------------------------

const std::vector<std::string>& CHost::GetGPUModels(void) const
{
    return(GPUModelNames);
}

//------------------------------------------------------------------------------

bool CHost::IsGPUModelSMP(void)
{
    if( NumOfHostGPUs <= 1 ) return(false);
    CSmallString first = GPUModelNames[0];
    for(size_t i=1; i < GPUModelNames.size(); i++){
        if( first != GPUModelNames[i] ) return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetHostName(void)
{
    return(Hostname);
}

//------------------------------------------------------------------------------

void CHost::SetHostName(const CSmallString& hostname)
{
    Hostname = hostname;
    AlienHost = true;
}

//------------------------------------------------------------------------------

int CHost::GetNCPUs(void)
{
    return(NCPUs);
}

//------------------------------------------------------------------------------

int CHost::GetNNodes(void)
{
    return(NNodes);
}

//------------------------------------------------------------------------------

int CHost::GetNGPUs(void)
{
    return(NGPUs);
}

//------------------------------------------------------------------------------

int CHost::GetNumOfHostCPUs(void)
{
    return(NumOfHostCPUs);
}

//------------------------------------------------------------------------------

int CHost::GetNumOfHostGPUs(void)
{
    return(NumOfHostGPUs);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetArchTokens(void)
{
    CSmallString tokens;

    std::list<std::string>::iterator   it = AllTokens.begin();
    std::list<std::string>::iterator   ie = AllTokens.end();
    while(it != ie){
        if( it != AllTokens.begin() )  tokens << ",";
        tokens << (*it);
        it++;
    }
    return(tokens);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetSecTokens(std::vector<std::string>& list)
{
    CSmallString tokens;

    std::vector<std::string>::iterator   it = list.begin();
    std::vector<std::string>::iterator   ie = list.end();
    while(it != ie){
        if( it != list.begin() )  tokens << ",";
        tokens << (*it);
        it++;
    }

    if( tokens == NULL ){
        tokens << "-none-";
    }

    return(tokens);
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetSecTokens(std::set<std::string>& list)
{
    CSmallString tokens;

    std::set<std::string>::iterator   it = list.begin();
    std::set<std::string>::iterator   ie = list.end();
    while(it != ie){
        if( it != list.begin() )  tokens << ",";
        tokens << (*it);
        it++;
    }

    if( tokens == NULL ){
        tokens << "-none-";
    }

    return(tokens);
}

//------------------------------------------------------------------------------

void CHost::PrintHostDetailedInfo(CVerboseStr& vout)
{
    // parse config file -------------------------
    CXMLElement* p_aele = NULL;

    // try to load group specific setup
    CXMLElement* p_gele = FindGroup();
    if( p_gele != NULL ){
        p_gele->GetAttribute("name",ConfigRealm);
        if( ConfigRealm == NULL ) ConfigRealm = "group-specific";
        p_aele = p_gele->GetChildElementByPath("archs");
    }

    // if not exists then use default
    if( p_aele == NULL ){
        // use default
        p_aele = Hosts.GetChildElementByPath("config/archs");
        ConfigRealm = NULL;
    }


    vout << endl;
    vout << "Full host name      : " << GetHostName() << endl;
    vout << "Config key          : " << ConfigKey << endl;
    if( ConfigRealm != NULL ){
    vout << "Configuration realm : " << ConfigRealm << endl;
    } else {
    vout << "Configuration realm : default" << endl;
    }
    if( CacheLoaded ){
    CSmallTime time(CacheValidity());
    vout << "Loaded from cache ... (Cache is still valid for " << time.GetSTimeAndDay() << ")" << endl;
    } else {
    CSmallTime time(CACHE_VALIDITY);
    vout << "No cache loaded ...   (New cache will be walid for " << time.GetSTimeAndDay() << ")" << endl;
    }
    vout << "===================================================================" << endl;

    CXMLElement* p_ele = NULL;
    if( p_aele != NULL ){
        p_ele = p_aele->GetFirstChildElement();
    }

    int pri = 0;
    while( p_ele ){
        if( p_ele->GetName() == "default" ){
            pri++;
    vout << ">>> default ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Enabled       : " << WhatIsEnabled(p_ele) << endl;
    vout << "    Priority      : " << pri << endl;
    vout << "    Num of CPUs   : " << DefaultNumOfHostCPUs << endl;
    CPrintEngine::PrintTokens(vout,"    Default tokens: ",GetSecTokens(DefaultTokens));
        }
        if( p_ele->GetName() == "hosts" ){
            pri++;
    vout << ">>> hosts ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Enabled       : " << WhatIsEnabled(p_ele) << endl;
    vout << "    Priority      : " << pri << endl;
    vout << "    Num of CPUs   : " << HostNumOfHostCPUs << endl;
    CPrintEngine::PrintTokens(vout,"    Host tokens   : ",GetSecTokens(HostTokens));
        }
        if( p_ele->GetName() == "desktop" ){
            pri++;
    vout << ">>> desktop ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    if( IsDesktop ){
    vout << "    Status        : host is a desktop, the CPU penalty will apply for PBS node specification" << endl;
    } else {
    vout << "    Status        : host is not a desktop" << endl;
    vout << "    CPU penalty   : " << DesktopCPUPenalty << endl;
    }
        }
        if( p_ele->GetName() == "cpuinfo" ){
            pri++;
    vout << ">>> cpuinfo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    if( AlienHost == true ){
    vout << "    Enabled       : " << "disabled - alien host" << endl;
    vout << "    Priority      : " << pri << endl;
    } else {
    vout << "    Enabled       : " << WhatIsEnabled(p_ele) << endl;
    vout << "    Priority      : " << pri << endl;
    vout << "    Num of CPUs   : " << CPUInfoNumOfHostCPUs << endl;
    vout << "    SMP CPU model : " << GetCPUModel() << endl;
    CPrintEngine::PrintTokens(vout,"    CPU flags     : ",GetSecTokens(CPUInfoFlags));
    vout << "    CPU spec      : " << CPUSpec << endl;
    if( HTDetected ){
    vout << "    HypThreading  : detected ";
        if( HTEnabled ){
            vout << "and enabled" << endl;
        } else {
            vout << "but disabled" << endl;
        }
    } else {
    vout << "    HypThreading  : not found" << endl;
    }
    CPrintEngine::PrintTokens(vout,"    CPU tokens    : ",GetSecTokens(CPUInfoTokens));
    }
        }
        if( p_ele->GetName() == "gpuinfo" ){
            pri++;
    vout << ">>> gpuinfo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Enabled       : " << "tokens if requested num of GPUs > 0" << endl;
    vout << "    Priority      : " << pri << endl;
    vout << "    Requested GPUs: " << NGPUs << endl;
    vout << "    Arch tokens   : " << GetSecTokens(GPUInfoTokens) << endl;
        }
        if( p_ele->GetName() == "cuda" ){
            pri++;
    vout << ">>> cuda ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    if( AlienHost == true ){
    vout << "    Enabled       : " << "disabled - alien host" << endl;
    vout << "    Priority      : " << pri << endl;
    } else {
    vout << "    Enabled       : " << "tokens/ngpus" << endl;
    vout << "    Priority      : " << pri << endl;
    vout << "    Host filter   : " << CudaFilter << endl;
    if( CudaFilter != "-none-" ){
    vout << "    CUDA library  : " << CudaLib << endl;
    vout << "    Num of GPUs   : " << NumOfHostGPUs << endl;
    if( NumOfHostGPUs > 0 ){
    if( IsGPUModelSMP() == false ){
    for(size_t i=0; i < GPUModelNames.size(); i++){
    vout << "    GPU model #" << setw(1) << i+1 << "  : " << GPUModelNames[i] << endl;
    }
    } else {
    vout << "    SMP GPU model : " << GPUModelNames[0] << endl;
    }
    CPrintEngine::PrintTokens(vout,"    Capabilities  : ",GetSecTokens(GPUCapabilities));
    CPrintEngine::PrintTokens(vout,"    CUDA tokens   : ",GetSecTokens(CUDATokens));
    }
    }
    }
        }
        if( p_ele->GetName() == "net" ){
            pri++;
    vout << ">>> network ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    if( AlienHost == true ){
    vout << "    Enabled       : " << "disabled - alien host" << endl;
    vout << "    Priority      : " << pri << endl;
    } else {
    vout << "    Priority      : " << pri << endl;
    vout << "    Net devices   : " << GetSecTokens(NetDevs) << endl;
    vout << "    Net filters   : " << NetFilters << endl;
    CPrintEngine::PrintTokens(vout,"    Net tokens    : ",GetSecTokens(NetTokens));
    }
        }

        p_ele = p_ele->GetNextSiblingElement();
    }

    vout << ">>> compatibility ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Arch tokens   : " << GetSecTokens(CompatTokens) << endl;
    vout << "===================================================================" << endl;
    vout << ">>> final" << endl;
    vout << "    Num of CPUs   : " << GetNumOfHostCPUs() << endl;
    vout << "    SMP CPU model : " << GetCPUModel() << endl;
    if( NumOfHostGPUs > 0 ){
    vout << "    Num of GPUs   : " << GetNumOfHostGPUs() << endl;
    if( IsGPUModelSMP() == false ){
    for(size_t i=0; i < GPUModelNames.size(); i++){
    vout << "    GPU model #" << setw(1) << i+1 << "  : " << GPUModelNames[i] << endl;
    }
    } else {
    vout << "    SMP GPU model : " << GPUModelNames[0] << endl;
    }
    CPrintEngine::PrintTokens(vout,"    Arch tokens   : ",GetArchTokens());
    }
}

//------------------------------------------------------------------------------

const CSmallString CHost::WhatIsEnabled(CXMLElement* p_ele)
{
    if( p_ele == NULL ) return("-nothing-");
    bool tokens;
    bool ncpus;
    CSmallString value;
    tokens = p_ele->GetAttribute("tokens",value);
    ncpus = p_ele->GetAttribute("ncpus",value);

    CSmallString output;
    if( tokens || ncpus ){
        if( tokens ) output << "tokens";
        if( tokens && ncpus ) output << "/";
        if( ncpus ) output << "ncpus";
    } else {
        output << "-nothing-";
    }

    return(output);
}

//------------------------------------------------------------------------------

int CHost::GetArchTokenScore(const CSmallString& token)
{
    CXMLElement* p_ele = Hosts.GetChildElementByPath("config/tokens");
    CXMLIterator I(p_ele);
    CXMLElement* p_tele;

    while( (p_tele = I.GetNextChildElement("token")) != NULL ){
        CSmallString stname;
        p_tele->GetAttribute("name",stname);
        if( stname == token ){
            int score = 0;
            p_tele->GetAttribute("score",score);
            return(score);
        }
    }

    return(0.0);
}

//------------------------------------------------------------------------------

bool CHost::HasToken(const CSmallString& token)
{
    CSmallString sys_arch = Host.GetArchTokens();

    list<string> sys_tokens;
    string       ssys_arch(sys_arch);
    split(sys_tokens,ssys_arch,is_any_of(","));

    list<string>::iterator sit = sys_tokens.begin();
    list<string>::iterator set = sys_tokens.end();

    while( sit != set ){
        if( *sit == string(token) ){
            return(true);
        }
        sit++;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CHost::IsLoadedFromCache(void)
{
    return( CacheLoaded );
}

//------------------------------------------------------------------------------

long int CHost::CacheValidity(void)
{
    if( CacheLoaded == false ) return(0);
    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();
    long int tv = CacheTime + CACHE_VALIDITY - current_time.GetSecondsFromBeginning();
    if( tv > 0 ){
        return( tv );
    } else {
        return( 0 );
    }
}

//------------------------------------------------------------------------------

void CHost::PrintHWSpec(CVerboseStr& vout)
{
    vout << GetCPUModel();
    if( NumOfHostGPUs > 0 ){
       vout << " | " << GPUModelNames[0];
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHost::PrintNodeInfo(CVerboseStr& vout)
{
    if( IsDesktop ){
        vout << "ncpus " << CPUInfoNumOfHostCPUs - DesktopCPUPenalty << endl;
    } else {
        vout << "ncpus " << CPUInfoNumOfHostCPUs << endl;
    }
    vout << "cpu_model " << CPUModelName << endl;
    vout << "cpu_flags " << join(CPUInfoFlags,",") << endl;
    vout << "spec " << CPUSpec << endl;
    vout << "ngpus " << NumOfHostGPUs << endl;
    if( NumOfHostGPUs > 0 ){
    vout << "gpu_model " << GPUModelNames[0] << endl;
    } else {
    vout << "gpu_model" << endl;
    }
    vout << "gpu_cap " << join(GPUCapabilities,",") << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CHost::GetCUDACapabilities(void)
{
    return(Hosts.GetChildElementByPath("config/cuda_caps"));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

