// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "SetEnv.hpp"
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <ostream>
#include <SmallTimeAndDate.hpp>
#include <ShellProcessor.hpp>
#include <AMSRegistry.hpp>
#include <HostGroup.hpp>
#include <Host.hpp>
#include <iomanip>
#include <User.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <UserUtils.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CSetEnv)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSetEnv::Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout is used for shell processor
    Console.Attach(stderr);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-setenv (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CSetEnv::Run(void)
{
    // is it Infinity job?
    if( CShell::GetSystemVariable("_INFINITY_JOB_") == "_INFINITY_JOB_" ){
        vout << endl;
        vout << "<red>>>> ERROR:</red> amssetenv cannot be used in the Infinity job!" << endl;
        vout << endl;
        return(false);
    }


// get process parent ID as unique identifier
    pid_t  pid = getppid();

// generate unique host cache name
    CSmallString cachename = "/tmp/ams_cache_r09." + CUserUtils::GetUserName() + ".";
    cachename << pid;
    CShell::SetSystemVariable("AMS_HOST_CACHE",cachename);

// init AMS registry
    AMSRegistry.LoadRegistry(vout);

// init host group
    HostGroup.InitHostsConfig();
    HostGroup.InitHostGroup();

// init host
    Host.InitHostSubSystems(HostGroup.GetHostSubSystems());
    Host.InitHost(Options.GetOptNCPUs(),Options.GetOptNGPUs());

    if( Options.GetOptNCPUs() > Host.GetNumOfHostCPUs() ){
        vout << low;
        vout << endl;
        vout << "<red>>>> ERROR:</red> Number of requested CPUs '" << Options.GetOptNCPUs() << "' is higher then the number of host CPUs '" << Host.GetNumOfHostCPUs() << "'!" << endl;
        return(false);
    }
    if( Options.GetOptNGPUs() > Host.GetNumOfHostGPUs() ){
        vout << low;
        vout << endl;
        vout << "<red>>>> ERROR:</red> Number of requested GPUs '" << Options.GetOptNGPUs() << "' is higher then the number of host GPUs '" << Host.GetNumOfHostGPUs() << "'!" << endl;
        return(false);
    }

    // print info
    vout << low;
    vout <<     endl;
    vout <<     "# Host info and requested resources" << endl;
    vout <<     "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    Host.PrintHostInfoForModule(vout);

    // generate node file and gpunode files
    CSmallString nodefile = "/tmp/nodefile.";
    nodefile << pid;
    CSmallString gpufile  = "/tmp/gpufile.";
    gpufile << pid;

    ofstream nfstr;
    nfstr.open(nodefile);
    if( nfstr ) {
        for(int i=0; i < Host.GetNCPUs(); i++){
            nfstr << Host.GetHostName() << endl;
        }
    }
    nfstr.close();

    CSmallString gpu_visible;
    nfstr.open(gpufile);
    if( nfstr ) {
        for(int i=0; i < Host.GetNGPUs(); i++){
            if( i > 0 ) gpu_visible << ",";
            nfstr << Host.GetHostName() << " gpu=" << i << endl;
            gpu_visible << i;
        }
    }
    nfstr.close();
    if( gpu_visible == NULL ) gpu_visible = "-1";

    // generate environment
    ShellProcessor.SetVariable("AMS_HOST_CACHE",cachename);
    ShellProcessor.SetVariable("INF_NCPUS",Host.GetNCPUs());
    ShellProcessor.SetVariable("INF_NGPUS",Host.GetNGPUs());
    ShellProcessor.SetVariable("INF_NNODES",1);
    ShellProcessor.SetVariable("INF_NODEFILE",nodefile);
    ShellProcessor.SetVariable("INF_GPUFILE",gpufile);
    ShellProcessor.SetVariable("CUDA_VISIBLE_DEVICES",gpu_visible);

    return(true);
}

//------------------------------------------------------------------------------

void CSetEnv::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-setenv (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        ErrorSystem.PrintErrors(vout);
    }

    vout << low;
    vout <<     endl;

    int exit_code = 0;
    if( ErrorSystem.IsError() ) {
        exit_code = 1;
        ShellProcessor.RollBack();
    }

    ShellProcessor.SetExitCode(exit_code);
    ShellProcessor.BuildEnvironment();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
