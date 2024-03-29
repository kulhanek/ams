// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2023      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include "BundleCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSRegistry.hpp>
#include <HostGroup.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <FileSystem.hpp>
#include <ModBundle.hpp>
#include <PrintEngine.hpp>
#include <string.h>
#include <DirNodeItem.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CBundleCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CBundleCmd::Init(int argc, char* argv[])
{
    ForcePrintErrors = false;

    // encode program options, all check procedures are done inside of Options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout is used for shell processor
    Console.Attach(stdout);
    ConsoleErr.Attach(stderr);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    verr.Attach(ConsoleErr);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
        verr.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
        verr.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-bundle (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CBundleCmd::Run(void)
{   
// init AMS registry
    AMSRegistry.LoadRegistry(vout);

// init host group
    HostGroup.InitHostsConfig();
    HostGroup.InitHostGroup();

// init host
    Host.InitHostSubSystems(HostGroup.GetHostSubSystems());
    Host.InitHost();

// init user
    User.InitUserConfig();
    User.InitUser();

    // ----------------------------------------------
    if( (Options.GetArgAction() == "info") ) {
        return( InfoBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "create" ) {
        return( CreateBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rebuild" ) {
        return( RebuildBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail" ) {
        return( PrintAvailMods() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "index" ) {
        return( BundleIndex() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "dirname" ) {
        return( BundleDirName() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rootpath" ) {
        return( BundleRootPath() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "sources" ) {
        return( BundleSources() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "newverindex" ) {
        return( NewVerIndex() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "sync" ) {
        return( SyncBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "dirlist" ) {
        return( BundleDirList() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "dpkg-deps" ) {
        return( ShowDpkgDeps() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "allmodules" ) {
        return( ListAllModules() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "allbuilds" ) {
        return( ListAllBuilds() );
    }
    // ----------------------------------------------
    else {
        CSmallString error;
        error << "not implemented action '" << Options.GetArgAction() << "'";
        ES_ERROR(error);
        ForcePrintErrors =true;
    }

    return(false);
}

//------------------------------------------------------------------------------

void CBundleCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    if( (! ForcePrintErrors) && (Options.GetArgAction() != "dirname")
            && (Options.GetArgAction() != "rootpath")
            && (Options.GetArgAction() != "sources")
            && (Options.GetArgAction() != "allmodules")
            && (Options.GetArgAction() != "allbuilds")
            && (Options.GetArgAction() != "dpkg-deps")
            && (Options.GetArgAction() != "dirlist")
            && (Options.GetOptSilent() == false) ) vout << endl;

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-bundle (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        verr << high;
        if( ForcePrintErrors ) verr << low;
        ErrorSystem.PrintErrors(verr);
        if( ForcePrintErrors ) verr << endl;
    } else {
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBundleCmd::InfoBundle(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    vout << endl;
    vout << "# Scaning doc and bld files ... ";
    bundle.FindAllFragmentFiles();
    vout << "done" << endl;

    bool modstat = true;

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_TRACE_ERROR(error);
        modstat = false;
    }

    vout << endl;
    bundle.PrintInfo(vout,modstat,true,true);

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::CreateBundle(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);

    if( CModBundle::GetBundleRoot(cwd,bundle_root) == true ){
        CSmallString error;
        error << "the current directory is the part of the existing AMS bundle";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( Options.GetNumberOfProgArgs() != 4 ){
        CSmallString error;
        error << "incorrect number of arguments";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.CreateBundle(cwd,Options.GetProgArg(1),Options.GetProgArg(2),Options.GetProgArg(3),
                            Options.GetOptForce()) == false ){
        CSmallString error;
        error << "unable to create bundle '" << cwd / Options.GetProgArg(1) << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::RebuildBundle(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    vout << endl;
    vout << "# Scaning doc and bld files ... ";
    bundle.FindAllFragmentFiles();
    vout << "done" << endl;

    vout << endl;
    bundle.PrintInfo(vout);

    vout << endl;
    if( bundle.RebuildCache(vout) == false ) return(false);

    return( bundle.SaveCaches() );
}

//------------------------------------------------------------------------------

bool CBundleCmd::PrintAvailMods(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    PrintEngine.InitPrintProfile();

    PrintEngine.PrintHeader(Console.GetTerminal(),"AVAILABLE MODULES (ams-bundle)",EPEHS_SECTION);
    bundle.PrintAvail(Console.GetTerminal(),Options.GetOptIncludeVersions(),true);

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::BundleIndex(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( Options.GetProgArg(1) == "new" ){
        if( bundle.ListBuildsForIndex(vout,Options.GetOptPersonal()) == false ){
            CSmallString error;
            error << "unable to list build for index";
            ES_ERROR(error);
            ForcePrintErrors = true;
            return(false);
        }
        bundle.CalculateNewIndex(vout);
        if( bundle.SaveNewIndex()  == false ){
            CSmallString error;
            error << "unable to save new index";
            ES_ERROR(error);
            ForcePrintErrors = true;
            return(false);
        }
        return(true);
    } else if( Options.GetProgArg(1) == "diff" ){
        if( bundle.LoadIndexes()  == false ){
            CSmallString error;
            error << "unable to load new and old indexes";
            ES_ERROR(error);
            ForcePrintErrors = true;
            return(false);
        }
        bundle.DiffIndexes(vout,Options.GetOptSkipRemovedEntries(),
                           Options.GetOptSkipAddedEntries(),! Options.GetOptSilent());
    } else if( Options.GetProgArg(1) == "commit" ){
        if( bundle.CommitNewIndex()  == false ){
            CSmallString error;
            error << "unable to commit new index";
            ES_ERROR(error);
            ForcePrintErrors = true;
            return(false);
        }
    } else {
        CSmallString error;
        error << "unsupported index operation '" << Options.GetProgArg(1) << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::BundleDirName(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        return(false);
    }

    vout << bundle.GetFullBundleName() << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::BundleRootPath(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        return(false);
    }

    vout << bundle.GetBundleRootPath() << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::NewVerIndex(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    double verindex = bundle.GetNewVerIndex(Options.GetProgArg(1));
    vout << low;
    vout << verindex;
    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::SyncBundle(void)
{
    CFileName cmd;
    cmd = AMSRegistry.GetAMSRootDIR() / "share" / "scripts" / "ams-sync-bundle";

    execl(cmd, cmd, Options.GetProgArg(1).GetBuffer(),(char*)NULL);

    // The exec() functions return only if an error has occurred.

    CSmallString error;
    error << "Unable to run '" << cmd << " " << Options.GetProgArg(1) << "' errno: ";
    error << strerror(errno);
    ES_ERROR(error);

    return(false);
}

//------------------------------------------------------------------------------

bool CBundleCmd::BundleSources(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.SaveSourceFile(Options.GetProgArg(1)) == false ){
        CSmallString error;
        error << "unable to save bundle source file";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::BundleDirList(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CDirNodeItemPtr  dir_tree(new CDirNodeItem);

    CFileName root;
    if( Options.GetOptPersonal() ){
        root = bundle.GetFullBundleName();
        dir_tree->ScanSoftRepoTree(root,4);
    } else {
        root = bundle.GetBundleRootPath();
        dir_tree->ScanSoftRepoTree(root,bundle.GetName(),4);
    }

     CXMLElement* p_cele = bundle.GetCacheElement();
     if( p_cele == NULL ){
         RUNTIME_ERROR("unable to find root cache element");
     }

     CXMLElement* p_mele = p_cele->GetFirstChildElement("module");

     while( p_mele != NULL ) {
         CSmallString name;
         p_mele->GetAttribute("name",name);
         CXMLElement* p_build = p_mele->GetChildElementByPath("builds/build");
         while( p_build != NULL ) {
             CSmallString ver,arch,mode;
             p_build->GetAttribute("ver",ver);
             p_build->GetAttribute("arch",arch);
             p_build->GetAttribute("mode",mode);
             CFileName package_dir;
             package_dir = CModCache::GetVariableValue(p_build,"AMS_PACKAGE_DIR");
             if( package_dir != NULL ){
                 CSmallString build_name;
                 build_name << name << ":" << ver << ":" << arch << ":" << mode;
                 dir_tree->AddPackageDir(package_dir,build_name);
             }
             p_build = p_build->GetNextSiblingElement("build");
         }
         p_mele = p_mele->GetNextSiblingElement("module");
     }

    if( Options.GetProgArg(1) == "missing" ){
        // print missing records
        dir_tree->PrintMissing(vout,root);
    } else if( Options.GetProgArg(1) == "orphans" ){
        // print missing records
        dir_tree->CountOrphanedChilds();
        dir_tree->PrintOrphans(vout,root);
    } else if( Options.GetProgArg(1) == "existing" ){
        // print missing records
        dir_tree->PrintExisting(vout,root);
    } else if( Options.GetProgArg(1) == "all" ){
        // print all records
        dir_tree->PrintTree(vout,root,0);
    } else {
        CSmallString error;
        error << "unsupported action for dirlist: '" << Options.GetProgArg(1) << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::ShowDpkgDeps(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    std::list<CSmallString> deps;
    bundle.GetDPKGDeps(deps);

    for(CSmallString dep : deps){
        cout << dep << endl;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::ListAllModules(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    bundle.PrintAllModules(vout);

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::ListAllBuilds(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at the current directory: '" << cwd << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    if( bundle.LoadCache(EMBC_SMALL) == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        ForcePrintErrors = true;
        return(false);
    }

    bundle.PrintAllBuilds(vout);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


