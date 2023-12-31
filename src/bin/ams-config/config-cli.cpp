// ===============================================================================
// AMS - Advanced Module System
// -------------------------------------------------------------------------------
//    Copyright (C) 2004,2005,2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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
// ===============================================================================


#include "config-cli-opts.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <TerminalStr.hpp>
#include <Shell.hpp>
#include <ctype.h>
#include <AMSRegistry.hpp>
#include <Site.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <Utils.hpp>
#include <PrintEngine.hpp>
#include <ModuleController.hpp>
#include <ModUtils.hpp>
#include <ModCache.hpp>
#include <SiteController.hpp>
#include <HostGroup.hpp>

//------------------------------------------------------------------------------

CConfigCliOpts  Options;
CTerminalStr    Console;

//------------------------------------------------------------------------------

int Init(int argc, char* argv[]);
bool Run(void);
bool Finalize(void);

//-----------------------------------------------

void PrintHeader(void);
void MainMenu(void);
void PrintMainMenu(void);
bool QuitProgram(void);
bool SaveSetup(void);

//-----------------------------------------------------------------------------

void VisualizationMenu(void);
void PrintVisualizationMenu(void);
void ShowActivePrintProfile(void);
void ListAvailablePrintProfiles(void);
void SetPrintProfile(void);
bool SetPrintProfile(const CSmallString& profile);

//-----------------------------------------------------------------------------

void AutoLoadedModulesMenu(void);
void PrintAutoLoadedModulesMenu(void);
void ListAvailableModules(void);
void ListAutoLoadedModules(void);
void AddAutoLoadedModule(void);
bool AddAutoLoadedModule(const CSmallString& module);
void RemoveAutoLoadedModule(void);
bool RemoveAutoLoadedModule(const CSmallString& module);
void RemoveAllAutoLoadedModules(void);

//------------------------------------------------------------------------------

void PrintUserSetupMenu(void);
void UserSetupMenu(void);
void PrintUserUMask(void);
void ChangeUserUMask(void);

//-----------------------------------------------------------------------------

void UserBundlesMenu(void);
void PrintUserBundlesMenu(void);
void ListUserBundles(void);
void AddUserBundle(void);
bool AddUserBundle(const CSmallString& bundle);
void RemoveUserBundle(void);
bool RemoveUserBundle(const CSmallString& bundle);
void RemoveAllUserBundles(void);
void PrintUserBundlePath(void);
void SetUserBundlePath(void);
bool SetUserBundlePath(const CSmallString& profile);

//------------------------------------------------------------------------------

bool VisualizationChanged       = false;
bool AutoLoadedModuleChanged    = false;
bool UserSetupChanged           = false;
bool UserBundlesChanged         = false;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int main(int argc, char* argv[])
{
    int result = Init(argc,argv);

    Console.Attach(stdout);

    switch(result) {
    case SO_EXIT:
        return(0);
    case SO_CONTINUE:
        if( Run() == false ) {
            fprintf(stderr,"\n");
            ErrorSystem.PrintLastError("ams-config");;
            if( Options.GetOptVerbose() == false ) fprintf(stderr,"\n");
            Finalize();
            return(1);
        }
        Finalize();
        return(0);
    case SO_USER_ERROR:
        Finalize();
        return(2);
    case SO_OPTS_ERROR:
    default:
        return(3);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool Run(void)
{
// init AMS registry
    AMSRegistry.LoadRegistry();

// init host group
    HostGroup.InitHostsConfig();
    HostGroup.InitHostGroup();

// init host
    Host.InitHostSubSystems(HostGroup.GetHostSubSystems());
    Host.InitHost();

// init user
    User.InitUserConfig();
    User.InitUser();

// init site controller
    SiteController.InitSiteControllerConfig();

// init module controller
    ModuleController.InitModuleControllerConfig();

// enter to menu driven configuration setup
    PrintHeader();
    MainMenu();

    return(true);
}

//------------------------------------------------------------------------------

bool Finalize(void)
{
    if( Options.GetOptVerbose() ) {
        ErrorSystem.PrintErrors(stderr);
        fprintf(stderr,"\n");
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintHeader(void)
{
    printf("\n");
    printf("           ***  AMS Configuration Centre ***                \n");
    printf("          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~         \n");
    printf("------------------------------------------------------------\n");
    printf("\n");
}

//------------------------------------------------------------------------------

void PrintMainMenu(void)
{
    printf("\n");
    printf(" Main menu\n");
    printf("------------------------------------------------------------\n");
    printf("             GLOBAL SETUP (valid for all sites)             \n");
    printf("------------------------------------------------------------\n");
    printf(" 1   - configure visualization (color profiles)\n");
    printf(" 2   - configure auto-loaded modules\n");
    printf(" 3   - configure user details (umask)\n");
    printf(" 4   - module bundles\n");
    printf("------------------------------------------------------------\n");
    printf(" s   - save changes\n");
    printf(" p   - print this menu once again\n");
    printf(" q/r - quit program\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void MainMenu(void)
{
    bool cont = true;
    PrintMainMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {

        case '1':
            VisualizationMenu();
            PrintMainMenu();
            break;

        case '2':
            AutoLoadedModulesMenu();
            PrintMainMenu();
            break;

        case '3':
            UserSetupMenu();
            PrintMainMenu();
            break;

        case '4':
            UserBundlesMenu();
            PrintMainMenu();
            break;

        case 's': {
            SaveSetup();
        }
        break;
        case 'p':
            PrintMainMenu();
            break;
        case 'q':
        case 'r':
            cont = ! QuitProgram();
            printf("\n");
            break;
        default:
            printf("\n");
            printf(" ERROR: Unrecognized menu item, try again ...\n");
            printf("\n");
            break;
        }
    } while( cont == true );

    printf(" Module configuration terminated on user request.\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

bool QuitProgram(void)
{
    bool changed = VisualizationChanged || AutoLoadedModuleChanged || UserSetupChanged || UserBundlesChanged;
    if( changed == false ) return(true);

    for(;;) {
        printf("\n");
        printf(" WARNING: Configuration was changed!\n");
        printf("          Do you want to save it before program will be terminated (yes/no/skip)? ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return(false);

        if( strcmp(buffer,"yes") == 0 ) {
            return(SaveSetup());
        }

        if( strcmp(buffer,"no") == 0 ) {
            return(true);
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword ! Try again ...\n");
    }

    return(false);
}

//------------------------------------------------------------------------------

bool SaveSetup(void)
{
    bool changed = VisualizationChanged || AutoLoadedModuleChanged || UserSetupChanged || UserBundlesChanged;

    bool result = true;
    ErrorSystem.RemoveAllErrors();
    if( changed ){
        result = AMSRegistry.SaveUserConfig();
    }
    if( result == true ) {
        VisualizationChanged = false;
        AutoLoadedModuleChanged = false;
        UserSetupChanged = false;
        UserBundlesChanged = false;
        printf(" >>> User configuration was successfully saved.\n\n");
    } else {
        printf("\n");
        printf(" ERROR: Unable to save your modified setup.\n");
        printf("\n");
        ErrorSystem.PrintErrors();
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintVisualizationMenu(void)
{
    printf("\n");
    printf("                     Visualization Menu                     \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - Print color profile\n");
    printf(" 2 - List available color profiles\n");
    printf(" 3 - Change color profile\n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void VisualizationMenu(void)
{
    bool cont = true;
    PrintVisualizationMenu();
    do {
        char buffer[80];
        memset(buffer,0,80);
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            ShowActivePrintProfile();
            PrintVisualizationMenu();
            break;
        case '2':
            ListAvailablePrintProfiles();
            PrintVisualizationMenu();
            break;
        case '3':
            SetPrintProfile();
            PrintVisualizationMenu();
            break;

        case 'p':
            PrintVisualizationMenu();
            break;
        case 'r':
        case 'q':
            cont=false;
            break;
        default:
            printf("\n");
            printf(" ERROR: Unrecognized menu item, try again ...\n");
            printf("\n");
            break;
        }
    } while( cont == true );
    printf("\n");
}

//-----------------------------------------------------------------------------

void ShowActivePrintProfile(void)
{
    printf("\n");
    printf(" >>> Current print color profile is : %s\n",(const char*)AMSRegistry.GetUserPrintProfile());
    printf("\n");
}

//-----------------------------------------------------------------------------

void ListAvailablePrintProfiles(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" Available Print Color Profiles\n");
    printf("============================================================\n");
    printf("\n");

    std::list<CSmallString> profiles;
    AMSRegistry.GetPrintProfiles(profiles);

    for(CSmallString profile : profiles){
        printf("* %s\n",(const char*)profile);
    }
}

//-----------------------------------------------------------------------------

void SetPrintProfile(void)
{
    printf("\n");
    ListAvailablePrintProfiles();
    printf("\n");
    for(;;) {
        printf(" Type the print color profile name, or 'default' to set the default one (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( SetPrintProfile(buffer) == true ) {
            printf(" >>> Print color profile was successfully set.\n");
            VisualizationChanged = true;
            printf("\n");
            return;
        }
    }
}

//-----------------------------------------------------------------------------

bool SetPrintProfile(const CSmallString& profile)
{
    if( AMSRegistry.IsUserPrintProfile(profile) == false ){
        printf("\n");
        printf(" >>> ERROR: Specified print color profile does not exist!\n");
        printf("\n");
        return(false);
    }

    AMSRegistry.SetUserPrintProfile(profile);
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintAutoLoadedModulesMenu(void)
{
    printf("\n");
    printf("                     Auto-loaded Modules                    \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - List auto-loaded modules\n");
    printf(" 2 - List available modules\n");
    printf(" 3 - Add module to the list of auto-loaded modules\n");
    printf(" 4 - Remove module from the list of auto-loaded modules\n");
    printf(" 5 - Remove all auto-loaded modules\n");
    printf("------------------------------------------------------------\n");
    printf("* NOTE: All changes are effective in newly opened sessions !\n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void AutoLoadedModulesMenu(void)
{
    bool cont = true;
    PrintAutoLoadedModulesMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            ListAutoLoadedModules();
            PrintAutoLoadedModulesMenu();
            break;

        case '2':
            ListAvailableModules();
            PrintAutoLoadedModulesMenu();
            break;

        case '3':
            AddAutoLoadedModule();
            PrintAutoLoadedModulesMenu();
            break;

        case '4':
            RemoveAutoLoadedModule();
            PrintAutoLoadedModulesMenu();
            break;

        case '5':
            RemoveAllAutoLoadedModules();
            PrintAutoLoadedModulesMenu();
            break;

        case 'p':
            PrintAutoLoadedModulesMenu();
            break;
        case 'r':
        case 'q':
            cont=false;
            break;
        default:
            printf("\n");
            printf(" ERROR: Unrecognized menu item, try again ...\n");
            printf("\n");
            break;
        }
    } while( cont == true );
    printf("\n");
}

//-----------------------------------------------------------------------------

void ListAutoLoadedModules(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" List of Auto-loaded Modules\n");
    printf("============================================================\n");

    PrintEngine.InitPrintProfile();
    ModuleController.PrintUserAutoLoadedModules(Console.GetTerminal());
    printf("\n");
}

//-----------------------------------------------------------------------------

void ListAvailableModules(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" List of Available Modules\n");
    printf("============================================================\n");

    ModuleController.LoadBundles(EMBC_SMALL);
    ModuleController.MergeBundles();
    PrintEngine.InitPrintProfile();
    PrintEngine.PrintHeader(Console.GetTerminal(),"AVAILABLE MODULES (Infinity Software Base | amsmodule)",EPEHS_SECTION);
    ModCache.PrintAvail(Console.GetTerminal(),true,true);
}

//-----------------------------------------------------------------------------

void AddAutoLoadedModule(void)
{
    printf("\n");
    ListAvailableModules();
    printf("\n");
    for(;;) {
        printf(" Type the module name to be added to auto-loaded modules (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( AddAutoLoadedModule(buffer) == true ) {
            printf(" >>> Module was successfully added.\n");
            AutoLoadedModuleChanged = true;
            printf("\n");
        }
    }
}

//-----------------------------------------------------------------------------

bool AddAutoLoadedModule(const CSmallString& module)
{
    CSmallString name,ver,arch,mode;
    CModUtils::ParseModuleName(module,name,ver,arch,mode);

    if( arch != NULL ){
        printf("\n");
        printf(" >>> ERROR: Module architecture cannot be specified for autoloaded modules!\n");
        printf("\n");
        return(false);
    }

    if( mode != NULL ){
        printf("\n");
        printf(" >>> ERROR: Module parallel mode cannot be specified for autoloaded modules!\n");
        printf("\n");
        return(false);
    }

    CXMLElement* p_module = ModCache.GetModule(name);
    if( p_module == NULL ){
        printf("\n");
        printf(" >>> ERROR: Specified module was not found in the site database!\n");
        printf("\n");
        return(false);
    }

    if( (ver != NULL) && (ModCache.CheckModuleVersion(p_module,ver) == false) ){
        printf("\n");
        printf(" >>> ERROR: Specified module version does not exist for the selected module!\n");
        printf("\n");
        return(false);
    }

    if( AMSRegistry.IsUserAutoLoadedModule(name) ){
        printf("\n");
        printf(" >>> ERROR: Specified module is already in the list of auto-loaded modules!\n");
        printf("\n");
        return(false);
    }

    AMSRegistry.AddUserAutoLoadedModule(module);
    return(true);
}

//-----------------------------------------------------------------------------

void RemoveAutoLoadedModule(void)
{
    printf("\n");
    ListAutoLoadedModules();
    printf("\n");
    for(;;) {
        printf(" Type the module name to be removed from auto-loaded modules (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( RemoveAutoLoadedModule(buffer) == true ) {
            printf(" >>> Module was successfully removed.\n");
            AutoLoadedModuleChanged = true;
            printf("\n");
        }

    }
}

//-----------------------------------------------------------------------------

bool RemoveAutoLoadedModule(const CSmallString& module)
{
    if( AMSRegistry.IsUserAutoLoadedModule(module) == false ){
        printf("\n");
        printf(" >>> ERROR: Specified module is not in the list of auto-loaded modules!\n");
        printf("\n");
        return(false);
    }

    AMSRegistry.RemoveUserAutoLoadedModule(module);
    return(true);
}

//-----------------------------------------------------------------------------

void RemoveAllAutoLoadedModules(void)
{
    for(;;) {
        printf("\n");
        printf(" Do you want to remove all auto-loaded modules (yes/no/skip) ? ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return;

        if( strcmp(buffer,"yes") == 0 ) {
            AMSRegistry.RemoveAllUserAutoLoadedModules();
            printf(" >>> All auto-loaded modules were successfully removed.\n");
            AutoLoadedModuleChanged = true;
            return;
        }

        if( strcmp(buffer,"no") == 0 ) {
            return;
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword! Try again ...\n");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintUserSetupMenu(void)
{
    printf("\n");
    printf("                         User Details                       \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - Print user umask\n");
    printf(" 2 - Change user umask\n"); 
    printf("-----------------------------------------                   \n");
    printf(" 3 - Print user info\n");
    printf("------------------------------------------------------------\n");
    printf("* NOTE: All changes are effective in new terminal sessions !\n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void UserSetupMenu(void)
{
    bool cont = true;
    PrintUserSetupMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            PrintUserUMask();
            PrintUserSetupMenu();
            break;
        case '2':
            ChangeUserUMask();
            PrintUserSetupMenu();
            break;
        case '3':{
            CVerboseStr vout;
            vout.Attach(Console);
            User.PrintUserDetailedInfo(vout);
            PrintUserSetupMenu();
            }
            break;

        case 'p':
            PrintUserSetupMenu();
            break;
        case 'r':
        case 'q':
            cont=false;
            break;
        default:
            printf("\n");
            printf(" ERROR: Unrecognized menu item, try again ...\n");
            printf("\n");
            break;
        }
    } while( cont == true );
    printf("\n");
}

//-----------------------------------------------------------------------------

void PrintUserUMask(void)
{
    printf("\n");
    printf(" >>> User umask is : %s\n",(const char*)AMSRegistry.GetUserUMask());
    printf("\n");
}

//-----------------------------------------------------------------------------

void ChangeUserUMask(void)
{
    printf("\n");
    for(;;) {
        printf(" Type user file creation mask (umask) setup, or 'default' to set the default one (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( strlen(buffer) != 3 ){
            printf(" >>> ERROR: User file creation mask (umask) must be composed from three numbers!\n");
            printf("\n");
            continue;
        }
        bool error = false;
        for(int i=0; i < 3; i++){
            if( (buffer[i] < '0') || (buffer[i] > '7') ){
                printf(" >>> ERROR: Illegal number %c at possition %d in user file creation mask (umask)!\n",buffer[i],i+1);
                printf("            Allowed values: 0-7\n");
                printf("\n");
                error = true;
                break;
            }
        }
        if( error ) continue;

        AMSRegistry.SetUserUMask(buffer);
        printf(" >>> The umask was successfully set!\n");
        printf("\n");
        UserSetupChanged = true;
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintUserBundlesMenu(void)
{
    printf("\n");
    printf("                     User Module Bundles                    \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - List user bundles\n");
    printf(" 2 - Add user bundle\n");
    printf(" 3 - Remove user bundle\n");
    printf(" 4 - Remove all user bundles\n");
    printf(" 5 - Print user bundle path\n");
    printf(" 6 - Set user bundle path\n");
    printf("------------------------------------------------------------\n");
    printf("* NOTE: All changes are effective in newly opened sessions !\n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void UserBundlesMenu(void)
{
    bool cont = true;
    PrintUserBundlesMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            ListUserBundles();
            PrintUserBundlesMenu();
            break;

        case '2':
            AddUserBundle();
            PrintUserBundlesMenu();
            break;

        case '3':
            RemoveUserBundle();
            PrintUserBundlesMenu();
            break;

        case '4':
            RemoveAllUserBundles();
            PrintUserBundlesMenu();
            break;

        case '5':
            PrintUserBundlePath();
            PrintUserBundlesMenu();
            break;

        case '6':
            SetUserBundlePath();
            PrintUserBundlesMenu();
            break;

        case 'p':
            PrintUserBundlesMenu();
            break;
        case 'r':
        case 'q':
            cont=false;
            break;
        default:
            printf("\n");
            printf(" ERROR: Unrecognized menu item, try again ...\n");
            printf("\n");
            break;
        }
    } while( cont == true );
    printf("\n");
}

//-----------------------------------------------------------------------------

void ListUserBundles(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" List of User Module Bundles\n");
    printf("============================================================\n");

    std::list<CSmallString> bundles;

    AMSRegistry.GetUserBundleNames(bundles);

    if( bundles.size() == 0 ){
        bundles.push_back("-none-");
    }

    for(CSmallString bundle : bundles){
        printf("* %s\n",(const char*)bundle);
    }
}

//-----------------------------------------------------------------------------

void AddUserBundle(void)
{
    printf("\n");
    for(;;) {
        printf(" Type the user bundle name to be added (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( AddUserBundle(buffer) == true ) {
            printf(" >>> User bundle was successfully added.\n");
            UserBundlesChanged = true;
            printf("\n");
        }
    }
}

//-----------------------------------------------------------------------------

bool AddUserBundle(const CSmallString& bundle)
{
    if( AMSRegistry.IsUserBundleName(bundle) == true ){
        printf("\n");
        printf(" >>> ERROR: Specified user bundle is already in the list of module bundles!\n");
        printf("\n");
        return(false);
    }

    AMSRegistry.AddUserBundleName(bundle);
    return(true);
}

//-----------------------------------------------------------------------------

void RemoveUserBundle(void)
{
    printf("\n");
    ListUserBundles();
    printf("\n");
    for(;;) {
        printf(" Type the user bundle name to be removed (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( RemoveUserBundle(buffer) == true ) {
            printf(" >>> User bundle was successfully removed.\n");
            UserBundlesChanged = true;
            printf("\n");
        }

    }
}

//-----------------------------------------------------------------------------

bool RemoveUserBundle(const CSmallString& bundle)
{
    if( AMSRegistry.IsUserBundleName(bundle) == false ){
        printf("\n");
        printf(" >>> ERROR: Specified user bundle is not in the list of module bundles!\n");
        printf("\n");
        return(false);
    }

    AMSRegistry.RemoveUserBundleName(bundle);
    return(true);
}

//-----------------------------------------------------------------------------

void RemoveAllUserBundles(void)
{
    for(;;) {
        printf("\n");
        printf(" Do you want to remove all user bundles (yes/no/skip) ? ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return;

        if( strcmp(buffer,"yes") == 0 ) {
            AMSRegistry.RemoveAllUserBundleNames();
            printf(" >>> All user bundles were successfully removed.\n");
            UserBundlesChanged = true;
            return;
        }

        if( strcmp(buffer,"no") == 0 ) {
            return;
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword! Try again ...\n");
    }
}

//-----------------------------------------------------------------------------

void PrintUserBundlePath(void)
{
    CSmallString path = AMSRegistry.GetUserBundlePath();
    if( path == NULL ){
        path = "-none-";
    }
    printf("\n");
    printf(" >>> User bundle path is : %s\n",(const char*)path);
    printf("\n");
}

//-----------------------------------------------------------------------------

void SetUserBundlePath(void)
{
    printf("\n");
    PrintUserBundlePath();
    printf("\n");
    for(;;) {
        printf(" Type user bundle paths delimited by ':', or 'default' to remove the setup (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( SetUserBundlePath(buffer) == true ) {
            printf(" >>> User bundle path was successfully set.\n");
            UserBundlesChanged = true;
            printf("\n");
            return;
        }
    }
}

//-----------------------------------------------------------------------------

bool SetUserBundlePath(const CSmallString& profile)
{
    AMSRegistry.SetUserBundlePath(profile);
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

