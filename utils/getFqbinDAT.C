#include <iostream>
#include <fstream>
#include <cmath>
#include <TStyle.h>
#include <TNtuple.h>
#include <TFile.h>
#include <TObjString.h>
#include <TDatime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include "ScanOutput.h"

void getFqbinDAT(const char *inputFile = "/eos/user/s/salman/EPOS_JOBS_PRYI/RUN2_JOBS/2015_MC_BIN1_fb128/AnalysisResults.root", Int_t sampleID = 1, Int_t doMixed = 0)
{
  const Int_t SAMPLES = 100;
  Bool_t deBug = 0;

  TString absInputPath = TString(std::filesystem::absolute(inputFile).string().c_str());

  TFile* file = new TFile(inputFile, "read");
  if (!file->IsOpen()) {
    std::cout << "\033[31mError opening file\033[0m" << std::endl;
    exit(1);
  }
  std::cout << "\033[32mFile " << file->GetName() << " opened successfully\033[0m" << std::endl;

  // Auto-detect what's in the file
  // MC: lists are "ntplistRe..."/"ntplistGen..."/"ntplistMixed..."
  // DATA: lists are "ntplist..." (no "Re"), ntuples have no "rec"
  ScanOutput scanReal = ScanFile(file, "ntplistRe", "fntpMBin");
  ScanOutput scanData = ScanFile(file, "ntplist", "fntpMBin", {"ntplistRe", "ntplistGen", "ntplistMixed"});
  ScanOutput scanGen = ScanFile(file, "ntplistGen", "fntpMBingen");
  ScanOutput scanMixed = ScanFile(file, "ntplistMixed", "fntpMBinMixed");

  Bool_t hasReal = (!scanReal.ptNames.empty() && !scanReal.condNames.empty());
  Bool_t hasData = (!scanData.ptNames.empty() && !scanData.condNames.empty());
  Bool_t hasGen = (!scanGen.ptNames.empty());
  Bool_t hasMixed = (doMixed && !scanMixed.ptNames.empty() && !scanMixed.condNames.empty());

  if (!hasReal && !hasData && !hasGen && !hasMixed) {
    std::cout << "\033[31mNo ntuples found in file\033[0m" << std::endl;
    exit(1);
  }

  if (hasReal) {
    std::cout << "\033[32m[Real] " << scanReal.condNames.size() << " conditions, " << scanReal.ptNames.size() << " pt bins, Ntmax=" << scanReal.Ntmax << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanReal.condNames.size(); i++) std::cout << "\033[32m  condition[" << i << "] = " << scanReal.condNames[i] << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanReal.ptNames.size(); i++) std::cout << "\033[32m  ptBin[" << i << "] = " << scanReal.ptNames[i] << "\033[0m" << std::endl;
  }
  if (hasData) {
    std::cout << "\033[32m[Data] " << scanData.condNames.size() << " conditions, " << scanData.ptNames.size() << " pt bins, Ntmax=" << scanData.Ntmax << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanData.condNames.size(); i++) std::cout << "\033[32m  condition[" << i << "] = " << scanData.condNames[i] << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanData.ptNames.size(); i++) std::cout << "\033[32m  ptBin[" << i << "] = " << scanData.ptNames[i] << "\033[0m" << std::endl;
  }
  if (hasGen) {
    std::cout << "\033[32m[Gen] " << scanGen.ptNames.size() << " pt bins, Ntmax=" << scanGen.Ntmax << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanGen.ptNames.size(); i++) std::cout << "\033[32m  ptBin[" << i << "] = " << scanGen.ptNames[i] << "\033[0m" << std::endl;
  }
  if (hasMixed) {
    std::cout << "\033[32m[Mixed] " << scanMixed.condNames.size() << " conditions, " << scanMixed.ptNames.size() << " pt bins, Ntmax=" << scanMixed.Ntmax << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanMixed.condNames.size(); i++) std::cout << "\033[32m  condition[" << i << "] = " << scanMixed.condNames[i] << "\033[0m" << std::endl;
    for (UInt_t i = 0; i < scanMixed.ptNames.size(); i++) std::cout << "\033[32m  ptBin[" << i << "] = " << scanMixed.ptNames[i] << "\033[0m" << std::endl;
  }

  TNtuple* fntupleM[200];
  TNtuple* FqStanDefM[200];
  TNtuple* EvFactDefM[200];

  Float_t SumFqeq2 = 0.0, SumFqeq3 = 0.0, SumFqeq4 = 0.0, SumFqeq5 = 0.0, SumFqeq6 = 0.0, SumFqeq7 = 0.0;
  Float_t SumAvBincon = 0.0;
  Float_t Fqq2, Fqq3, Fqq4, Fqq5, Fqq6, Fqq7, RFqq2, RFqq3, RFqq4, RFqq5, RFqq6, RFqq7;
  Float_t nevents, nbins_val;
  Float_t Avbin2, Avbin3, Avbin4, Avbin5, Avbin6, Avbin7;

  std::ofstream outfile1, outfile2;
  Int_t name = sampleID;

  // Helper lambda to process one (ptBin, condition) or (ptBin) combination
  auto processNtuples = [&](TList *list1, Int_t Ntmax, const std::string &ptName, const std::string &condName, const std::string &outputDir, Int_t branchOffset, const std::string &typePrefix, const std::string &metaType) {

    for (Int_t t = 0; t < Ntmax; t++) {
      fntupleM[t] = nullptr;
      if (!condName.empty()) {
        TString fNtpname = Form("fntpMBin%srecbin%s_%s_M%d", typePrefix.c_str(), ptName.c_str(), condName.c_str(), t);
        fntupleM[t] = (TNtuple*)list1->FindObject(fNtpname.Data());
        if (!fntupleM[t]) {
          fNtpname = Form("fntpMBin%sbin%s_%s_M%d", typePrefix.c_str(), ptName.c_str(), condName.c_str(), t);
          fntupleM[t] = (TNtuple*)list1->FindObject(fNtpname.Data());
        }
      } else {
        TString fNtpname = Form("fntpMBingenbin%s_M%d", ptName.c_str(), t);
        fntupleM[t] = (TNtuple*)list1->FindObject(fNtpname.Data());
      }
    }

    std::filesystem::path dir(outputDir);
    if (!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);

    outfile1.open(Form("%sFqe_StdErrorSubSampling%d.dat", outputDir.c_str(), name), std::ofstream::out);
    outfile2.open(Form("%sFqebeforeRatio%d.dat", outputDir.c_str(), name), std::ofstream::out);

    TFile* Samplefile = new TFile(Form("%sFqe_standardErrorSample%d.root", outputDir.c_str(), name), "RECREATE");
    TList* fl1 = new TList();
    fl1->SetOwner(kTRUE);

    for (Int_t ik = 0; ik < Ntmax; ik++) {
      TString Ntname = "FqNtpStndBinM" + TString::Itoa(ik + 1, 10);
      TString Nttitle = "StdDefM" + TString::Itoa(ik + 1, 10);
      FqStanDefM[ik] = new TNtuple(Ntname.Data(), Nttitle.Data(), "nevents:nbins:Avbin:Fqq2:Fqq3:Fqq4:Fqq5:Fqq6:Fqq7");
      fl1->Add(FqStanDefM[ik]);
    }
    TFile* Rudy = new TFile(Form("%sTFqe_FqebeforeRatio_Sample%d.root", outputDir.c_str(), name), "RECREATE");
    TList* fl2 = new TList();
    fl2->SetOwner(kTRUE);
    for (Int_t ik = 0; ik < Ntmax; ik++) {
      TString RNtname = "FqeNtpNumBinM" + TString::Itoa(ik + 1, 10);
      TString RNttitle = "NumeratorM" + TString::Itoa(ik + 1, 10);
      EvFactDefM[ik] = new TNtuple(RNtname.Data(), RNttitle.Data(), "nevents:nbins:Avbin:Avbin2:Avbin3:Avbin4:Avbin5:Avbin6:Avbin7:RFqq2:RFqq3:RFqq4:RFqq5:RFqq6:RFqq7");
      fl2->Add(EvFactDefM[ik]);
    }
    Float_t* row_content;

    for (Int_t Ntnum = 0; Ntnum < Ntmax; Ntnum++) {
      Int_t TEvents = 0;
      Int_t counter = 0;
      Int_t events = 0;
      Int_t Evloop_Counter = 0;
      Int_t limit = 0;
      Int_t status = 0;
      Float_t Avbin_val = 0.0;
      Float_t NumBin_val = 0.0;
      SumFqeq2 = SumFqeq3 = SumFqeq4 = SumFqeq5 = SumFqeq6 = SumFqeq7 = 0.;
      SumAvBincon = 0.;

      if (!fntupleM[Ntnum]) continue;
      TEvents = fntupleM[Ntnum]->GetEntries();
      Evloop_Counter = ceil(TEvents / SAMPLES);
      limit = TEvents - (floor(TEvents / SAMPLES) * SAMPLES);
      if (deBug) std::cout << TEvents << " events  " << Evloop_Counter << "  " << limit << std::endl;

      for (Int_t irow = 0; irow < (TEvents - limit); irow++) {
        status = 0;
        events++;

        fntupleM[Ntnum]->GetEntry(irow);
        row_content = fntupleM[Ntnum]->GetArgs();

        NumBin_val = row_content[branchOffset + 1];
        Float_t aav = row_content[branchOffset + 2];
        SumFqeq2 += row_content[branchOffset + 3];
        SumFqeq3 += row_content[branchOffset + 4];
        SumFqeq4 += row_content[branchOffset + 5];
        SumFqeq5 += row_content[branchOffset + 6];
        SumFqeq6 += row_content[branchOffset + 7];
        SumFqeq7 += row_content[branchOffset + 8];

        SumAvBincon += aav;

        if (events == Evloop_Counter) { counter++; status = 1; }
        if (counter == SAMPLES && events == limit) status = 2;

        if (status == 1 || status == 2) {
          Int_t Totevents = events;
          nevents = events;
          nbins_val = NumBin_val;
          Avbin_val = SumAvBincon / events;

          Float_t AvFqeq2 = SumFqeq2 / Totevents;
          Float_t AvFqeq3 = SumFqeq3 / Totevents;
          Float_t AvFqeq4 = SumFqeq4 / Totevents;
          Float_t AvFqeq5 = SumFqeq5 / Totevents;
          Float_t AvFqeq6 = SumFqeq6 / Totevents;
          Float_t AvFqeq7 = SumFqeq7 / Totevents;

          Fqq2 = AvFqeq2 / (pow(Avbin_val, 2.0));
          Fqq3 = AvFqeq3 / (pow(Avbin_val, 3.0));
          Fqq4 = AvFqeq4 / (pow(Avbin_val, 4.0));
          Fqq5 = AvFqeq5 / (pow(Avbin_val, 5.0));
          Fqq6 = AvFqeq6 / (pow(Avbin_val, 6.0));
          Fqq7 = AvFqeq7 / (pow(Avbin_val, 7.0));

          Avbin2 = pow(Avbin_val, 2.0);
          Avbin3 = pow(Avbin_val, 3.0);
          Avbin4 = pow(Avbin_val, 4.0);
          Avbin5 = pow(Avbin_val, 5.0);
          Avbin6 = pow(Avbin_val, 6.0);
          Avbin7 = pow(Avbin_val, 7.0);

          RFqq2 = AvFqeq2;
          RFqq3 = AvFqeq3;
          RFqq4 = AvFqeq4;
          RFqq5 = AvFqeq5;
          RFqq6 = AvFqeq6;
          RFqq7 = AvFqeq7;

          outfile1 << " " << nevents << "  " << nbins_val << " " << Avbin_val << " " << Fqq2 << " " << Fqq3 << " " << Fqq4 << " " << Fqq5 << " " << Fqq6 << " " << Fqq7 << std::endl;
          outfile2 << " " << nevents << "  " << nbins_val << " " << Avbin_val << " " << Avbin2 << "  " << Avbin3 << "  " << Avbin4 << "  " << Avbin5 << "  " << Avbin6 << "  " << Avbin7 << " " << RFqq2 << " " << RFqq3 << " " << RFqq4 << " " << RFqq5 << " " << RFqq6 << " " << RFqq7 << std::endl;

          FqStanDefM[Ntnum]->Fill(nevents, nbins_val, Avbin_val, Fqq2, Fqq3, Fqq4, Fqq5, Fqq6, Fqq7);
          EvFactDefM[Ntnum]->Fill(nevents, nbins_val, Avbin_val, Avbin2, Avbin3, Avbin4, Avbin5, Avbin6, Avbin7, RFqq2, RFqq3, RFqq4, RFqq5, RFqq6, RFqq7);

          events = 0.;
          status = 0;
          SumAvBincon = 0.;
          SumFqeq2 = SumFqeq3 = SumFqeq4 = SumFqeq5 = SumFqeq6 = SumFqeq7 = 0.;
        }
      }
    }

    // Write metadata to file
    Samplefile->cd();
    TObjString *metaInputFile = new TObjString(absInputPath.Data());
    metaInputFile->Write("meta_inputFile");
    TObjString *metaPtBinName = new TObjString(ptName.c_str());
    metaPtBinName->Write("meta_ptBinName");
    if (!condName.empty()) {
      TObjString *metaCondName = new TObjString(condName.c_str());
      metaCondName->Write("meta_condName");
    }
    TObjString *metaTypeStr = new TObjString(metaType.c_str());
    metaTypeStr->Write("meta_type");
    Samplefile->Write();
    Samplefile->Close();
    Rudy->Write();
    Rudy->Close();
    outfile1.close();
    outfile2.close();
  };

  // Process reconstructed real data (MC "ntplistRe..." or DATA "ntplist...")
  // Both have the Mult branch as first variable -> offset=0
  auto processRealLike = [&](const ScanOutput &scan, const char *listBase,
                             const std::vector<std::string> &excludedListPrefixes,
                             const char *label, const char *metaType) {
    for (UInt_t nWagons = 0; nWagons < scan.condNames.size(); nWagons++) {
      for (UInt_t nBins = 0; nBins < scan.ptNames.size(); nBins++) {
        std::string outputDir = scan.condNames[nWagons] + "/bin" + std::to_string(nBins + 1) + "/datafiles/";

        TList* list1 = nullptr;
        list1 = (TList*)file->Get(Form("%s%d%s_cond%d", listBase, nBins + 1, "taskfm", nWagons + 1));
        if (!list1) {
          TIter nextkey(file->GetListOfKeys());
          TKey *key;
          while ((key = (TKey*)nextkey())) {
            TString keyName = key->GetName();
            if (!keyName.BeginsWith(listBase)) continue;
            if (IsExcludedListName(keyName, excludedListPrefixes)) continue;
            TList *l = (TList*)file->Get(keyName.Data());
            if (!l) continue;
            TIter nextobj(l);
            TObject *obj;
            while ((obj = nextobj())) {
              if (!obj->InheritsFrom("TNtuple")) continue;
              TString ntupleName = obj->GetName();
              if (!ntupleName.BeginsWith("fntpMBin") || ntupleName.Contains("Mixed") || ntupleName.Contains("gen")) continue;
              std::string ptN, condN;
              Int_t mV;
              ParseNtupleName(ntupleName, ptN, condN, mV);
              if (ptN == scan.ptNames[nBins] && condN == scan.condNames[nWagons]) {
                list1 = l;
                break;
              }
            }
            if (list1) break;
          }
        }
        if (!list1) {
          std::cout << "\033[31mCould not find " << label << " list for ptBin=" << scan.ptNames[nBins] << " cond=" << scan.condNames[nWagons] << "\033[0m" << std::endl;
          continue;
        }
        std::cout << "Processing " << label << ": " << scan.condNames[nWagons] << " / " << scan.ptNames[nBins] << std::endl;
        processNtuples(list1, scan.Ntmax, scan.ptNames[nBins], scan.condNames[nWagons], outputDir, 0, "", metaType);
      }
    }
  };

  if (hasReal) processRealLike(scanReal, "ntplistRe", {}, "real", "real");
  if (hasData) processRealLike(scanData, "ntplist", {"ntplistRe", "ntplistGen", "ntplistMixed"}, "data", "data");

  // Process mixed (has Mult branch -> offset=0)
  if (hasMixed) {
    for (UInt_t nWagons = 0; nWagons < scanMixed.condNames.size(); nWagons++) {
      for (UInt_t nBins = 0; nBins < scanMixed.ptNames.size(); nBins++) {
        std::string outputDir = scanMixed.condNames[nWagons] + "/bin" + std::to_string(nBins + 1) + "/datafiles/mixed/";

        TList* list1 = nullptr;
        list1 = (TList*)file->Get(Form("ntplistMixed%d%s_cond%d", nBins + 1, "taskfm", nWagons + 1));
        if (!list1) {
          TIter nextkey(file->GetListOfKeys());
          TKey *key;
          while ((key = (TKey*)nextkey())) {
            TString keyName = key->GetName();
            if (!keyName.BeginsWith("ntplistMixed")) continue;
            TList *l = (TList*)file->Get(keyName.Data());
            if (!l) continue;
            TIter nextobj(l);
            TObject *obj;
            while ((obj = nextobj())) {
              if (!obj->InheritsFrom("TNtuple")) continue;
              TString ntupleName = obj->GetName();
              if (!ntupleName.BeginsWith("fntpMBinMixed")) continue;
              std::string ptN, condN;
              Int_t mV;
              ParseNtupleName(ntupleName, ptN, condN, mV);
              if (ptN == scanMixed.ptNames[nBins] && condN == scanMixed.condNames[nWagons]) {
                list1 = l;
                break;
              }
            }
            if (list1) break;
          }
        }
        if (!list1) {
          std::cout << "\033[31mCould not find mixed list for ptBin=" << scanMixed.ptNames[nBins] << " cond=" << scanMixed.condNames[nWagons] << "\033[0m" << std::endl;
          continue;
        }
        std::cout << "Processing mixed: " << scanMixed.condNames[nWagons] << " / " << scanMixed.ptNames[nBins] << std::endl;
        processNtuples(list1, scanMixed.Ntmax, scanMixed.ptNames[nBins], scanMixed.condNames[nWagons], outputDir, 0, "Mixed", "mixed");
      }
    }
  }

  // Process gen (no Mult branch -> offset=0)
  if (hasGen) {
    for (UInt_t nBins = 0; nBins < scanGen.ptNames.size(); nBins++) {
      std::string outputDir = "gen/bin" + std::to_string(nBins + 1) + "/datafiles/";

      TList* list1 = nullptr;
      list1 = (TList*)file->Get(Form("ntplistGen%d%s", nBins + 1, "taskfm"));
      if (!list1) {
        TIter nextkey(file->GetListOfKeys());
        TKey *key;
        while ((key = (TKey*)nextkey())) {
          TString keyName = key->GetName();
          if (!keyName.BeginsWith("ntplistGen")) continue;
          TList *l = (TList*)file->Get(keyName.Data());
          if (!l) continue;
          TIter nextobj(l);
          TObject *obj;
          while ((obj = nextobj())) {
            if (!obj->InheritsFrom("TNtuple")) continue;
            TString ntupleName = obj->GetName();
            if (!ntupleName.BeginsWith("fntpMBingen")) continue;
            std::string ptN, condN;
            Int_t mV;
            ParseNtupleName(ntupleName, ptN, condN, mV);
            if (ptN == scanGen.ptNames[nBins]) {
              list1 = l;
              break;
            }
          }
          if (list1) break;
        }
      }
      if (!list1) {
        std::cout << "\033[31mCould not find gen list for ptBin=" << scanGen.ptNames[nBins] << "\033[0m" << std::endl;
        continue;
      }
      std::cout << "Processing gen: " << scanGen.ptNames[nBins] << std::endl;
      processNtuples(list1, scanGen.Ntmax, scanGen.ptNames[nBins], "", outputDir, -1, "", "gen");
    }
  }

  std::cout << "\033[32mDone.\033[0m" << std::endl;
}
