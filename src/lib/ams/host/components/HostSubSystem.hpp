#ifndef HostSubSystemH
#define HostSubSystemH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <AMSMainHeader.hpp>
#include <VerboseStr.hpp>
#include <FileName.hpp>
#include <XMLDocument.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

//------------------------------------------------------------------------------

enum EHostCacheMode {
    EHC_IGNORED = 1,
    EHC_LOADED  = 2,
    EHC_REININT = 3,
};

//------------------------------------------------------------------------------

enum EPrintHostInfo {
    EPHI_SITE   = 0,
    EPHI_MODULE = 1,
};


//------------------------------------------------------------------------------

class CHostSubSystem;
typedef boost::shared_ptr<CHostSubSystem>   CHostSubSystemPtr;

//------------------------------------------------------------------------------

class AMS_PACKAGE CHostSubSystem {
public:
// constructor and destructor --------------------------------------------------
    CHostSubSystem(const CFileName& config_file);
    virtual ~CHostSubSystem(void);

    /// object factory
    static CHostSubSystemPtr Create(const CFileName& file_name);

// input methods ---------------------------------------------------------------
    /// init host subsystem
    virtual void Init(void);

    /// apply setup to Host
    virtual void Apply(void);

    /// load host subsystem information from the host cache, if false, use Init()
    virtual EHostCacheMode LoadFromCache(CXMLElement* p_ele);

    /// save host subsystem information into the host cache
    virtual void SaveToCache(CXMLElement* p_ele);

    /// print subsystem info
    virtual void PrintSubSystemInfo(CVerboseStr& vout);

    /// print node resources (for PBSPro)
    virtual void PrintNodeResources(CVerboseStr& vout);

    /// print host resources for
    virtual void PrintHostInfoFor(CVerboseStr& vout,EPrintHostInfo mode);

// section of private data -----------------------------------------------------
private:
    CFileName       ConfigFile;
    CXMLDocument    Config;

// section of protected data ---------------------------------------------------
protected:
    /// get the config file name
    const CFileName& GetConfigFile(void);

    /// get subsystem config
    CXMLElement*     GetConfig(const CSmallString& section);

    /// return token list
    static const CSmallString GetTokenList(std::list<CSmallString>& tokens,const CSmallString delim=",");
};

//------------------------------------------------------------------------------

#endif
