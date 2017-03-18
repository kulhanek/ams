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
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <PrintEngine.hpp>
#include <AMSUserConfig.hpp>
#include <Site.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <Utils.hpp>

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

// ----------------------------------------------
void VisualizationMenu(void);
void PrintVisualizationMenu(void);

void EnableColors(void);
void SetSiteSectionDelimiter(void);
void SetSiteSectionBgColor(void);
void SetSiteSectionFgColor(void);
void SetSectionDelimiter(void);
void SetSectionBgColor(void);
void SetSectionFgColor(void);
void SetCategoryDelimiter(void);
void SetCategoryBgColor(void);
void SetCategoryFgColor(void);
void SetModuleBgColor(void);
void SetModuleFgColor(void);
void SetIncludeVersion(void);

const char* GetColorName(int color_id);
char SetDelimiter(const char* p_name);
void PrintColorDefinition(void);
int SetColor(const char* p_name);
int Question(const char *p_quest);
void GetTextValue(const char *p_epilog,CSmallString& value);

//-----------------------------------------------------------------------------

void AutorestoredModulesMenu(void);
void PrintAutorestoredModulesMenu(void);
void ListAutorestoredModules(void);
void AddAutorestoredModule(void);
bool AddAutorestoredModule(const CSmallString& module);
void RemoveAutorestoredModule(void);
bool RemoveAutorestoredModule(const CSmallString& module);
void RemoveAllAutorestoredModules(void);

//------------------------------------------------------------------------------

void PrintUserSetupMenu(void);
void UserSetupMenu(void);
void PrintUserUMask(void);
void ChangeUserUMask(void);

//------------------------------------------------------------------------------

void PrintSitePrioritiesMenu(void);
void SitePrioritiesMenu(void);
void PrintSitePriorities(void);
void ChangeSitePriorities(void);
void UseDefaultSitePriorities(void);
void ListSites(void);

//------------------------------------------------------------------------------

void PrintModulePrioritiesMenu(void);
void ModulePrioritiesMenu(void);
void PrintModulePriorities(void);
void ChangeModulePriorities(void);
void UseDefaultModulePriorities(void);

//------------------------------------------------------------------------------

bool VisualizationChanged = false;
bool AutorestoredModuleChanged = false;
bool UserSetupChanged = false;
bool SitePrioritiesChanged = false;
bool ModulePrioritiesChanged = false;

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
    // check if site is active
    if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    // initialze AMS cache
    if( Cache.LoadCache() == false) {
        ES_ERROR("unable to load AMS cache");
        return(false);
    }

    // load print config
    if( PrintEngine.LoadConfig() == false) {
        ES_ERROR("unable to load print engine config");
        return(false);
    }

    // init global host and user data
    Host.InitGlobalSetup();
    User.InitGlobalSetup();

    // initialize hosts -----------------------------
    Host.InitHostFile();
    Host.InitHost();

    if( AMSGlobalConfig.GetActiveSiteID() != NULL ){
        // initialize user -----------------------------
        User.InitUserFile(AMSGlobalConfig.GetActiveSiteID());
        User.InitUser();
    }

    // load user config
    AMSUserConfig.LoadUserConfig();

    // load site config
    if( Site.LoadConfig() == false) {
        ES_ERROR("unable to load site config");
        return(false);
    }

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
    printf(" 1   - configure visualization (colors, delimiters, etc.)\n");
    printf(" 2   - configure auto-restored modules\n");
    printf(" 3   - configure user details (umask)\n");
// FIX ME
/*
    printf(" 4   - configure site priorities\n");
    printf(" 5   - configure module priorities\n");
*/
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
            AutorestoredModulesMenu();
            PrintMainMenu();
            break;
        case '3':
            UserSetupMenu();
            PrintMainMenu();
            break;
 // FIX ME
 /*       case '4':
            SitePrioritiesMenu();
            PrintMainMenu();
            break;
        case '5':
            ModulePrioritiesMenu();
            PrintMainMenu();
            break; */

        case 's': {
            bool result = true;
            ErrorSystem.RemoveAllErrors();
            if( VisualizationChanged ) {
                result = PrintEngine.SaveUserConfig();
            }
            if( AutorestoredModuleChanged || UserSetupChanged || SitePrioritiesChanged || ModulePrioritiesChanged ){
                result = AMSUserConfig.SaveUserConfig();
            }
            if( result == true ) {
                VisualizationChanged = false;
                AutorestoredModuleChanged = false;
                UserSetupChanged = false;
                SitePrioritiesChanged = false;
                printf("\n");
                printf(" >>> User configuration was successfully saved.\n");
                printf("\n");
            } else {
                printf("\n");
                printf(" ERROR: Unable to save your modified setup.\n");
                printf("\n");
                ErrorSystem.PrintErrors();
            }
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
    bool changed = VisualizationChanged || AutorestoredModuleChanged || UserSetupChanged || SitePrioritiesChanged || ModulePrioritiesChanged;

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
            bool result = true;
            ErrorSystem.RemoveAllErrors();
            if( VisualizationChanged ) {
                result = PrintEngine.SaveUserConfig();
            }
            if( AutorestoredModuleChanged || UserSetupChanged || SitePrioritiesChanged || ModulePrioritiesChanged ){
                result = AMSUserConfig.SaveUserConfig();
            }
            if( result == true ) {
                VisualizationChanged = false;
                AutorestoredModuleChanged = false;
                UserSetupChanged = false;
                SitePrioritiesChanged = false;
                printf(" >>> User configuration was successfully saved.\n");
            } else {
                printf("\n");
                printf(" ERROR: Unable to save your modified setup.\n");
                printf("\n");
                ErrorSystem.PrintErrors();
            }
            return(result);
        }

        if( strcmp(buffer,"no") == 0 ) {
            return(true);
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword ! Try again ...\n");
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintVisualizationMenu(void)
{
    char           cdat;
    int            idat;
    bool           bdat;
    bool           usersetup;
    char           sexist;

    printf("\n");
    printf("                     Visualization Menu                     \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                       Current value   \n");
    printf("----------------------------------------- ------------------\n");

    sexist = '*';
    cdat = PrintEngine.GetSiteSectionDelimiter(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 1 - Site section delimiter             : '%c'       %c\n",cdat,sexist);

    sexist = '*';
    idat = PrintEngine.GetSiteSectionBgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 2 - Background color of site section   : %-9s %c\n",GetColorName(idat),sexist);

    sexist = '*';
    idat = PrintEngine.GetSiteSectionFgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 3 - Foreground color of site section   : %-9s %c\n",GetColorName(idat),sexist);
    printf("----------------------------------------- ------------------\n");

    sexist = '*';
    cdat = PrintEngine.GetSectionDelimiter(usersetup);
    if( usersetup == false ) cdat = PrintEngine.GetSectionDelimiter();
    if( usersetup == true ) sexist = ' ';
    printf(" 4 - Section delimiter                  : '%c'       %c\n",cdat,sexist);

    sexist = '*';
    idat = PrintEngine.GetSectionBgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 5 - Background color of sections       : %-9s %c\n",GetColorName(idat),sexist);

    sexist = '*';
    idat = PrintEngine.GetSectionFgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 6 - Foreground color of sections       : %-9s %c\n",GetColorName(idat),sexist);
    printf("----------------------------------------- ------------------\n");

    sexist = '*';
    cdat = PrintEngine.GetCategoryDelimiter(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 7 - Category delimiter                 : '%c'       %c\n",cdat,sexist);

    sexist = '*';
    idat = PrintEngine.GetCategoryBgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 8 - Background color of categories     : %-9s %c\n",GetColorName(idat),sexist);

    sexist = '*';
    idat = PrintEngine.GetCategoryFgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf(" 9 - Foreground color of categories     : %-9s %c\n",GetColorName(idat),sexist);
    printf("----------------------------------------- ------------------\n");

    sexist = '*';
    idat = PrintEngine.GetModuleBgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf("10 - Background color of modules        : %-9s %c\n",GetColorName(idat),sexist);

    sexist = '*';
    idat = PrintEngine.GetModuleFgColor(usersetup);
    if( usersetup == true ) sexist = ' ';
    printf("11 - Foreground color of modules        : %-9s %c\n",GetColorName(idat),sexist);
    printf("----------------------------------------- ------------------\n");

    sexist = '*';
    bdat = PrintEngine.IncludeVersion(usersetup);
    if( usersetup == true ) sexist = ' ';

    if( bdat == true ) {
        printf(" i - Include versions into module names : yes       %c\n",sexist);
    } else {
        printf(" i - Include versions into module names : no        %c\n",sexist);
    }

    sexist = '*';
    bdat = PrintEngine.AreColorsEnabled(usersetup);
    if( usersetup == true ) sexist = ' ';

    if( bdat == true ) {
        printf(" c - Use colors                         : yes       %c\n",sexist);
    } else {
        printf(" c - Use colors                         : no        %c\n",sexist);
    }
    printf("------------------------------------------------------------\n");
    printf("                                    * - system default setup\n");
    printf("------------------------------------------------------------\n");
    printf(" s - load system configuration\n");
    printf(" t - clear user visual configuration\n");
    printf("------------------------------------------------------------\n");
    printf(" d - test visualization (site avail)   [user setup]\n");
    printf(" f - test visualization (module avail) [user setup]\n");
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
            switch(buffer[1]) {
            case '0':
                SetModuleBgColor();
                PrintVisualizationMenu();
                break;
            case '1':
                SetModuleFgColor();
                PrintVisualizationMenu();
                break;
            case '\n':
                SetSiteSectionDelimiter();
                PrintVisualizationMenu();
                break;
            };
            break;
        case '2':
            SetSiteSectionBgColor();
            PrintVisualizationMenu();
            break;
        case '3':
            SetSiteSectionFgColor();
            PrintVisualizationMenu();
            break;

        case '4':
            SetSectionDelimiter();
            PrintVisualizationMenu();
            break;
        case '5':
            SetSectionBgColor();
            PrintVisualizationMenu();
            break;
        case '6':
            SetSectionFgColor();
            PrintVisualizationMenu();
            break;

        case '7':
            SetCategoryDelimiter();
            PrintVisualizationMenu();
            break;
        case '8':
            SetCategoryBgColor();
            PrintVisualizationMenu();
            break;
        case '9':
            SetCategoryFgColor();
            PrintVisualizationMenu();
            break;

        case 'i':
            SetIncludeVersion();
            PrintVisualizationMenu();
            break;
        case 'c':
            EnableColors();
            PrintVisualizationMenu();
            break;

        case 's':
            PrintEngine.SetUserConfigFromSystem();
            printf("\n");
            printf(" >>> User visual configuration was taken from system one.\n");
            printf("\n");
            VisualizationChanged = true;
            PrintVisualizationMenu();
            break;

        case 't':
            PrintEngine.ClearUserConfig();
            printf("\n");
            printf(" >>> User visual configuration was cleared.\n");
            printf("\n");
            VisualizationChanged = true;
            PrintVisualizationMenu();
            break;

        case 'd':
            PrintEngine.PrintAvailableSites(Console.GetTerminal(),false);
            PrintVisualizationMenu();
            break;

        case 'f':
            printf("\n");
            PrintEngine.PrintModAvailableModules(Console.GetTerminal());
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

const char* GetColorName(int color_id)
{
    switch(color_id) {
    case -1:
        return("default");
    case 0:
        return("black");
    case 1:
        return("red");
    case 2:
        return("green");
    case 3:
        return("brown");
    case 4:
        return("blue");
    case 5:
        return("magenta");
    case 6:
        return("cyan");
    case 7:
        return("white");
    default:
        return("illegal");
    }
}

//-----------------------------------------------------------------------------

void SetSiteSectionDelimiter(void)
{
    char delim = SetDelimiter("site section");
    if( delim == -1 ) return;

    PrintEngine.SetSiteSectionDelimiter(delim);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetSectionDelimiter(void)
{
    char delim = SetDelimiter("section");
    if( delim == -1 ) return;

    PrintEngine.SetSectionDelimiter(delim);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetCategoryDelimiter(void)
{
    char delim = SetDelimiter("category");
    if( delim == -1 ) return;

    PrintEngine.SetCategoryDelimiter(delim);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

char SetDelimiter(const char* p_name)
{
    for(;;) {
        printf("\n");
        printf(" Enter new %s delimiter or 'skip' to avoid change and press enter: ",p_name);
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return(-1);

        if( strlen(buffer) > 1 ) {
            printf("\n");
            printf(" ERROR: Only one character can be used as %s delimiter but '%d' were entered ...\n",
                   p_name,(int)strlen(buffer));
            continue;
        }

        if( (isspace(buffer[0]) > 0) && (buffer[0] != ' ') ) {
            printf("\n");
            printf(" ERROR: %s delimiter cannot be white character (except space), try again ...\n",
                   p_name);
            continue;
        }
        return(buffer[0]);
    }
}

//-----------------------------------------------------------------------------

void SetSiteSectionBgColor(void)
{
    int color = SetColor("site section title background");
    if( color == -2 ) return;

    PrintEngine.SetSiteSectionBgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetSiteSectionFgColor(void)
{
    int color = SetColor("site section title foreground");
    if( color == -2 ) return;

    PrintEngine.SetSiteSectionFgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetSectionBgColor(void)
{
    int color = SetColor("section title background");
    if( color == -2 ) return;

    PrintEngine.SetSectionBgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetSectionFgColor(void)
{
    int color = SetColor("section title foreground");
    if( color == -2 ) return;

    PrintEngine.SetSectionFgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetCategoryBgColor(void)
{
    int color = SetColor("category name background");
    if( color == -2 ) return;

    PrintEngine.SetCategoryBgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetCategoryFgColor(void)
{
    int color = SetColor("category name foreground");
    if( color == -2 ) return;

    PrintEngine.SetCategoryFgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetModuleBgColor(void)
{
    int color = SetColor("module name background");
    if( color == -2 ) return;

    PrintEngine.SetModuleBgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void SetModuleFgColor(void)
{
    int color = SetColor("module name foreground");
    if( color == -2 ) return;

    PrintEngine.SetModuleFgColor(color);
    VisualizationChanged = true;
}

//-----------------------------------------------------------------------------

void PrintColorDefinition(void)
{
    printf("\n");
    printf(" Color codes:\n");
    printf("------------------------------------------------------------\n");
    printf(" -1 - default\n");
    printf("  0 - black\n");
    printf("  1 - red\n");
    printf("  2 - green\n");
    printf("  3 - brown\n");
    printf("  4 - blue\n");
    printf("  5 - purple\n");
    printf("  6 - cyan\n");
    printf("  7 - white\n");
}

//-----------------------------------------------------------------------------

int SetColor(const char* p_name)
{
    PrintColorDefinition();

    for(;;) {
        printf("\n");
        printf(" Enter color code for %s or 'skip' to avoid changes: ",p_name);
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return(-2);

        int color = -2;
        sscanf(buffer,"%d",&color);

        if( (color >= -1) && (color <= 7) ) {
            return(color);
            continue;
        }

        printf("\n");
        printf(" ERROR: Color code '%d' is out of range! Try again ...\n",color);
    }

    return(-2);
}

//-----------------------------------------------------------------------------

void EnableColors(void)
{
    int result = Question("Would you like to use colors in moudle visualizations");

    switch(result) {
    case 0:
        PrintEngine.EnableColors(false);
        VisualizationChanged = true;
        break;
    case 1:
        PrintEngine.EnableColors(true);
        VisualizationChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void SetIncludeVersion(void)
{
    int result = Question("Would you like to include module versions into module names");

    switch(result) {
    case 0:
        PrintEngine.SetIncludeVersion(false);
        VisualizationChanged = true;
        break;
    case 1:
        PrintEngine.SetIncludeVersion(true);
        VisualizationChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

int Question(const char *p_quest)
{
    for(;;) {
        printf("\n");
        printf(" %s (yes/no/skip) ? ",p_quest);
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return(-1);

        if( strcmp(buffer,"yes") == 0 ) {
            return(1);
        }

        if( strcmp(buffer,"no") == 0 ) {
            return(0);
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword! Try again ...\n");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintAutorestoredModulesMenu(void)
{
    printf("\n");
    printf("                    Auto-restored Modules                   \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - List auto-restored modules\n");
    printf(" 2 - List available modules\n");
    printf(" 3 - Add module to the list of auto-restored modules\n");
    printf(" 4 - Remove module from the list of auto-restored modules\n");
    printf(" 5 - Remove all auto-restored modules\n");
    printf("------------------------------------------------------------\n");
    printf("* NOTE: All changes are effective in newly opened sessions !\n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void AutorestoredModulesMenu(void)
{
    bool cont = true;
    PrintAutorestoredModulesMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            ListAutorestoredModules();
            PrintAutorestoredModulesMenu();
            break;

        case '2':
            fprintf(stdout,"\n");
            PrintEngine.PrintModAvailableModules(Console.GetTerminal(),false,true);
            PrintAutorestoredModulesMenu();
            break;

        case '3':
            AddAutorestoredModule();
            PrintAutorestoredModulesMenu();
            break;

        case '4':
            RemoveAutorestoredModule();
            PrintAutorestoredModulesMenu();
            break;

        case '5':
            RemoveAllAutorestoredModules();
            PrintAutorestoredModulesMenu();
            break;

        case 'p':
            PrintAutorestoredModulesMenu();
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

void ListAutorestoredModules(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" List of Auto-restored Modules\n");
    printf("============================================================\n");
    AMSUserConfig.PrintAutorestoredModules();
    printf("\n");
}

//-----------------------------------------------------------------------------

void AddAutorestoredModule(void)
{
    printf("\n");
    for(;;) {
        printf(" Type the module name to be added to auto-restored modules (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( AddAutorestoredModule(buffer) == true ) {
            printf(" >>> Module was successfully added.\n");
            AutorestoredModuleChanged = true;
            printf("\n");
        }
    }
}

//-----------------------------------------------------------------------------

bool AddAutorestoredModule(const CSmallString& module)
{
    CSmallString name,ver,arch,mode;
    CUtils::ParseModuleName(module,name,ver,arch,mode);

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

    CXMLElement* p_module = Cache.GetModule(name);
    if( p_module == NULL ){
        printf("\n");
        printf(" >>> ERROR: Specified module was not found in the site database!\n");
        printf("\n");
        return(false);
    }

    if( (ver != NULL) && (Cache.CheckModuleVersion(p_module,ver) == false) ){
        printf("\n");
        printf(" >>> ERROR: Specified module version does not exist for the selected module!\n");
        printf("\n");
        return(false);
    }

    if( AMSUserConfig.IsAutorestoredModule(name) ){
        printf("\n");
        printf(" >>> ERROR: Specified module is already in the list of autorestored modules!\n");
        printf("\n");
        return(false);
    }

    AMSUserConfig.AddAutorestoredModule(module);
    return(true);
}

//-----------------------------------------------------------------------------

void RemoveAutorestoredModule(void)
{
    printf("\n");
    ListAutorestoredModules();
    printf("\n");
    for(;;) {
        printf(" Type the module name to be removed from auto-restored modules (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( RemoveAutorestoredModule(buffer) == true ) {
            printf(" >>> Module was successfully removed.\n");
            AutorestoredModuleChanged = true;
            printf("\n");
        }

    }
}

//-----------------------------------------------------------------------------

bool RemoveAutorestoredModule(const CSmallString& module)
{
    if( AMSUserConfig.IsAutorestoredModule(module) == false ){
        printf("\n");
        printf(" >>> ERROR: Specified module is not in the list of autorestored modules!\n");
        printf("\n");
        return(false);
    }

    AMSUserConfig.RemoveAutorestoredModule(module);
    return(true);
}

//-----------------------------------------------------------------------------

void RemoveAllAutorestoredModules(void)
{
    for(;;) {
        printf("\n");
        printf(" Do you want to remove all auto-restored modules (yes/no/skip) ? ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return;

        if( strcmp(buffer,"yes") == 0 ) {
            AMSUserConfig.ClearAutorestoredConfig();
            printf(" >>> All auto-restored modules were successfully removed.\n");
            AutorestoredModuleChanged = true;
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
    printf(" >>> User umask is : %s\n",(const char*)AMSUserConfig.GetUserUMask());
    printf("\n");
}

//-----------------------------------------------------------------------------

void ChangeUserUMask(void)
{
    printf("\n");
    for(;;) {
        printf(" Type user file creation mask (umask) setup (or empty string to return back): ");
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

        AMSUserConfig.SetUserUMask(buffer);
        printf(" >>> The umask was successfully set!\n");
        printf("\n");
        UserSetupChanged = true;
        break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintSitePrioritiesMenu(void)
{
    printf("\n");
    printf("                       Site priorities                      \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - Print site priorities\n");
    printf(" 2 - Change site priorities\n");
    printf(" 3 - Use default site priorities\n");
    printf(" 4 - List available sites\n");
    printf("------------------------------------------------------------\n");
    printf("* NOTE: All changes are effective in new terminal sessions! \n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void SitePrioritiesMenu(void)
{
    bool cont = true;
    PrintSitePrioritiesMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            PrintSitePriorities();
            PrintSitePrioritiesMenu();
            break;
        case '2':
            ChangeSitePriorities();
            PrintSitePrioritiesMenu();
            break;
        case '3':
            UseDefaultSitePriorities();
            PrintSitePrioritiesMenu();
            break;
        case '4':
            ListSites();
            PrintSitePrioritiesMenu();
            break;

        case 'p':
            PrintSitePrioritiesMenu();
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

void PrintSitePriorities(void)
{
    printf("\n");
    if( AMSUserConfig.GetSitePriorities() == NULL ){
        printf(" >>> Site priorities are : -automatically determined-\n");
    } else {
        printf(" >>> Site priorities are  : %s\n",(const char*)AMSUserConfig.GetSitePriorities());
    }
    printf("\n");
}

//-----------------------------------------------------------------------------

void ChangeSitePriorities(void)
{
    printf("\n");
    for(;;) {
        printf(" Type site priorities separated by comma (or empty string to return back): ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( AMSUserConfig.IsAvailableSite(buffer) == false ){
            printf("\n");
            printf(" >>> ERROR: Specified site name is not on the list of available sites!\n");
            printf("\n");
            continue;
        }

        AMSUserConfig.SetSitePriorities(buffer);
        printf(" >>> The site priorities were successfully set!\n");
        printf("\n");
        SitePrioritiesChanged = true;
        break;
    }
}

//-----------------------------------------------------------------------------

void UseDefaultSitePriorities(void)
{
    AMSUserConfig.SetSitePriorities("");
    printf("\n");
    printf(" >>> Default site priorities will be used by AMS!\n");
    printf("\n");
    SitePrioritiesChanged = true;
}

//-----------------------------------------------------------------------------

void ListSites(void)
{
    printf("\n");
    printf("List of available sites:\n");

    // print available sites
    PrintEngine.PrintAvailableSites(Console.GetTerminal(),false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintModulePrioritiesMenu(void)
{
    printf("\n");
    printf("                       Module Priorities                    \n");
    printf("             -----------------------------------            \n");
    printf(" Item                                                       \n");
    printf("----------------------------------------- ------------------\n");
    printf(" 1 - Print module priorities\n");
    printf(" 2 - Change module priorities\n");
    printf(" 3 - Use default module priorities\n");
    printf("------------------------------------------------------------\n");
    printf("* NOTE: All changes are effective in new terminal sessions! \n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void ModulePrioritiesMenu(void)
{
    bool cont = true;
    PrintModulePrioritiesMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            PrintModulePriorities();
            PrintModulePrioritiesMenu();
            break;
        case '2':
            ChangeModulePriorities();
            PrintModulePrioritiesMenu();
            break;
        case '3':
            UseDefaultModulePriorities();
            PrintModulePrioritiesMenu();
            break;

        case 'p':
            PrintModulePrioritiesMenu();
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

void PrintModulePriorities(void)
{
    printf("\n");
    if( AMSUserConfig.GetModulePriorities() == NULL ){
        printf(" >>> Module priorities are : %s (-system default-)\n",(const char*)AMSUserConfig.GetDefaultModulePriorities());
    } else {
        printf(" >>> Module priorities are : %s\n",(const char*)AMSUserConfig.GetModulePriorities());
    }
    printf("\n");
}

//-----------------------------------------------------------------------------

void ChangeModulePriorities(void)
{
    printf("\n");
    for(;;) {
        printf(" Specify module priorities (or empty string to return back).\n"
               " Allowed values are: amsmodule; sysmodule; amsmodule/sysmodule; sysmodule/amsmodule: ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"") == 0 ) return;

        if( (strcmp(buffer,"amsmodule") != 0) &&
            (strcmp(buffer,"sysmodule") != 0) &&
            (strcmp(buffer,"amsmodule/sysmodule") != 0) &&
            (strcmp(buffer,"sysmodule/amsmodule") != 0) ) {
            printf("\n");
            printf(" >>> ERROR: Incorrect value for module priorities!\n");
            printf("\n");
            continue;
        }

        AMSUserConfig.SetModulePriorities(buffer);
        printf(" >>> The module priorities were successfully set!\n");
        printf("\n");
        ModulePrioritiesChanged = true;
        break;
    }
}

//-----------------------------------------------------------------------------

void UseDefaultModulePriorities(void)
{
    AMSUserConfig.SetModulePriorities("");
    printf("\n");
    printf(" >>> Default module priorities will be used by AMS!\n");
    printf("\n");
    ModulePrioritiesChanged = true;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


