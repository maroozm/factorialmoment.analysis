#include "AliAODInputHandler.h"
#include "AliAnalysisAlien.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisTaskFM_marooz.h"
#include "TChain.h"
#include "TMacro.h"
#include "TSystem.h"
#include "runLists.h"

#ifdef __CLING__
R__ADD_INCLUDE_PATH($ALICE_ROOT)
R__ADD_INCLUDE_PATH($ALICE_PHYSICS)
R__ADD_INCLUDE_PATH($ROOTSYS)
#endif

#include <ANALYSIS/macros/AddTaskPIDResponse.C>
#include <ANALYSIS/macros/AddTaskPIDqa.C>
#include <ANALYSIS/macros/train/AddAODHandler.C>
#include <ANALYSIS/macros/train/AddMCHandler.C>
#include <OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C>
#include <OADB/macros/AddTaskPhysicsSelection.C>


void runCode(TString taskName = "test_ME",
  Int_t _mode= 0, // 0: test, 1: full, 2: terminate, 3: merge
  Int_t _nruns = 1, // 0 for all, or a specific number
  Int_t period = 18 // 10, 15, 18
  ) {
  TDatime date;
  TString outputdir = Form("MC.20%d.%s.%d", period, taskName.Data(), date.GetDate());

  std::vector<Int_t> runlist = runLists::GetRunList(period);
  if (_nruns == 0) _nruns = runlist.size();

  std::cout << "\033[31m ============ ================== \033[0m" << std::endl;
  std::cout << "\033[31m == :: task marooz == :: == " << " RUNNING FOR THE YEAR 20" << period << " \n"
            << " ============ NUMBER OF RUNS: " << _nruns << "\n"
            << " ============ OUTPUT DIR: " << outputdir << "\033[0m" << std::endl;
  std::cout << "\033[31m ============ ================== \033[0m" << std::endl;

  //==== Load common libraries
  gSystem->Load("libCore");
  gSystem->Load("libTree");
  gSystem->Load("libGeom");
  gSystem->Load("libVMC");
  gSystem->Load("libXMLIO");
  gSystem->Load("libPhysics");
  gSystem->Load("libSTEERBase");
  gSystem->Load("libESD");
  gSystem->Load("libAOD");
  gSystem->Load("libANALYSIS");
  gSystem->Load("libANALYSISalice");

  AliAnalysisManager *mgr = new AliAnalysisManager("AnalysisTaskExample");
  AliAODInputHandler *aodH = new AliAODInputHandler();
  aodH->SetNeedField();
  mgr->SetInputEventHandler(aodH);

  AddMCHandler(kTRUE);
  AddTaskMultSelection();
  if (period != 10) AddTaskPhysicsSelection(kTRUE);
  AddTaskPIDResponse(kTRUE, kFALSE, kTRUE /* , "3", kFALSE */);
  AddTaskPIDqa();
  // AddTaskCentrality();

  gInterpreter->LoadMacro("AliAnalysisTaskFM_marooz.cxx++g");
  AliAnalysisTaskFM_marooz *task;
  if (period == 10) task = reinterpret_cast<AliAnalysisTaskFM_marooz *>(gInterpreter->ExecuteMacro("AddTaskFM_marooz.C(2010, 1, \"taskfm\")"));
  if (period == 15) task = reinterpret_cast<AliAnalysisTaskFM_marooz *>(gInterpreter->ExecuteMacro("AddTaskFM_marooz.C(2015, 1, \"taskfm\")"));
  if (period == 18) task = reinterpret_cast<AliAnalysisTaskFM_marooz *>(gInterpreter->ExecuteMacro("AddTaskFM_marooz.C(2018, 1, \"taskfm\")"));
  std::cout << "\033[31m == :: == task marooz == :: == " << " running AddTaskFM_marooz.C with year 20" << period << "\033[0m" << std::endl;

  if (!mgr->InitAnalysis()) return;
  mgr->SetDebugLevel(2);
  mgr->PrintStatus();
  mgr->SetUseProgressBar(1, 25);

  AliAnalysisAlien *plugin = new AliAnalysisAlien();
  plugin->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");
  plugin->SetAdditionalLibs("AliAnalysisTaskFM_marooz.cxx " "AliAnalysisTaskFM_marooz.h");
  plugin->SetAnalysisSource("AliAnalysisTaskFM_marooz.cxx");
  if (period == 10) {
    plugin->SetGridDataDir("/alice/sim/LHC11a10a_bis");
    plugin->SetDataPattern("/AOD162/*AliAOD.root");
  }
  if (period == 15) {
    plugin->SetGridDataDir("/alice/sim/2020/LHC20j6a/");
    plugin->SetDataPattern("/AOD243/*AliAOD.root");
  }
  if (period == 18) {
    plugin->SetGridDataDir("/alice/sim/2020/LHC20e3a/");
    plugin->SetDataPattern("/AOD243/*AliAOD.root");
  }
  // process run numbers
  for (Int_t k = 0; k < _nruns; k++)
    plugin->AddRunNumber(runlist[k]);

  plugin->SetAliPhysicsVersion("vAN-20241010_O2-1");
  plugin->SetSplitMaxInputFileNumber(30);
  plugin->SetExecutable(Form("task_marooz_%s_20%d.sh", taskName.Data(), period));
  plugin->SetAnalysisMacro(Form("task_marooz_%s_20%d.C", taskName.Data(), period));
  plugin->SetTTL(10000);
  plugin->SetJDLName(Form("task_marooz_%s_20%d.jdl", taskName.Data(), period));
  plugin->SetOutputToRunNo(kFALSE);
  plugin->SetKeepLogs(kTRUE);
  plugin->SetMaxMergeStages(1);

  if (_mode== 3) plugin->SetMergeViaJDL(kFALSE);
  else plugin->SetMergeViaJDL(kTRUE);

  plugin->SetGridWorkingDir(Form("%s", outputdir.Data()));
  plugin->SetGridOutputDir("output");
  mgr->SetGridHandler(plugin);
  plugin->SetNtestFiles(1);
  if (_mode== 0) plugin->SetRunMode("test");
  if (_mode== 1) plugin->SetRunMode("full");
  if (_mode== 2 || _mode== 3) plugin->SetRunMode("terminate");
  mgr->StartAnalysis("grid");
}
