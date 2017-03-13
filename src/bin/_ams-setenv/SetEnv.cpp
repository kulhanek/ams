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
#include <GlobalConfig.hpp>
#include <Host.hpp>
#include <iomanip>
#include <User.hpp>
#include <sys/types.h>
#include <unistd.h>

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
    // check if site is active
    if( GlobalConfig.GetActiveSiteID() == NULL ) {
        vout << low;
        vout << endl;
        vout << "<red>>>> ERROR:</red> No site is active!" << endl;
        return(false);
    }

    // init global host and user data
    Host.InitGlobalSetup();
    User.InitGlobalSetup();

    // initialize hosts -----------------------------
    Host.InitHostFile(GlobalConfig.GetActiveSiteID());

    // determine number of host CPUs and GPUs
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
    vout <<     ">>> Host info and requested resources" << endl;
    vout <<     "  Requested CPUs     : " << setw(3) << Host.GetNCPUs();
    vout <<     "  Requested GPUs     : " << setw(3) << Host.GetNGPUs()<< endl;
    vout <<     "  Num of host CPUs   : " << setw(3) << Host.GetNumOfHostCPUs();
    vout <<     "  Num of host GPUs   : " << setw(3) << Host.GetNumOfHostGPUs() << endl;
    vout <<     "  Requested nodes    : " << setw(3) << Host.GetNNodes() << endl;
    vout <<     "  Host arch tokens   : " << Host.GetArchTokens() << endl;
    vout <<     "  Host SMP CPU model : " << Host.GetCPUModel() << endl;
    if( Host.GetNumOfHostGPUs() > 0 ){
    if( Host.IsGPUModelSMP() == false ){
    for(size_t i=0; i < Host.GetGPUModels().size(); i++){
    vout <<     "  Host GPU model #" << setw(1) << i+1 << "  : " << Host.GetGPUModels()[i] << endl;
    }
    } else{
    vout <<     "  Host SMP GPU model : " << Host.GetGPUModels()[0] << endl;
    }
    }

    // generate node file and gpunode files
    pid_t  pid = getpid();
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

    nfstr.open(gpufile);
    if( nfstr ) {
        for(int i=0; i < Host.GetNCPUs(); i++){
            nfstr << Host.GetHostName() << endl;
        }
        for(int i=0; i < Host.GetNGPUs(); i++){
            nfstr << Host.GetHostName() << "-gpu" << i << endl;
        }
    }
    nfstr.close();

    // generate environment
    ShellProcessor.SetVariable("AMS_NCPU",Host.GetNCPUs());
    ShellProcessor.SetVariable("OMP_NUM_THREADS",Host.GetNCPUs());
    ShellProcessor.SetVariable("MKL_NUM_THREADS",Host.GetNCPUs());
    ShellProcessor.SetVariable("AMS_NGPU",Host.GetNGPUs());
    ShellProcessor.SetVariable("AMS_NNODE",1);
    ShellProcessor.SetVariable("AMS_NODEFILE",nodefile);
    ShellProcessor.SetVariable("AMS_GPUFILE",gpufile);

    ShellProcessor.SetVariable("INF_NCPU",Host.GetNCPUs());
    ShellProcessor.SetVariable("INF_NGPU",Host.GetNGPUs());
    ShellProcessor.SetVariable("INF_NNODE",1);
    ShellProcessor.SetVariable("INF_NODEFILE",nodefile);
    ShellProcessor.SetVariable("INF_GPUFILE",gpufile);

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
