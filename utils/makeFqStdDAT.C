#include <iostream>
#include <fstream>
#include <cmath>
#include <TStyle.h>
#include <TNtuple.h>
#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TString.h>
#include <TObjString.h>
#include <TDatime.h>
#include <vector>
#include <string>
#include <algorithm>

void makeFqStdDAT()
{
  // Auto-detect directories by scanning for bin*/datafiles/Fqe_standardErrorSample1.root
  std::vector<std::string> dirList;
  const char *baseDir = ".";
  void *dirp = gSystem->OpenDirectory(baseDir);
  if (!dirp) {
    std::cout << "\033[31mError opening current directory\033[0m" << std::endl;
    return;
  }
  const char *entry;
  while ((entry = gSystem->GetDirEntry(dirp))) {
    TString entryStr(entry);
    if (entryStr == "." || entryStr == "..") continue;
    // Check both real and mixed directory structures for any sample file
    Bool_t hasReal = kFALSE, hasMixed = kFALSE;
    void *dirp2 = gSystem->OpenDirectory(Form("%s/bin1/datafiles", entry));
    if (dirp2) {
      const char *e;
      while ((e = gSystem->GetDirEntry(dirp2))) {
        TString eStr(e);
        if (eStr.BeginsWith("Fqe_standardErrorSample") && eStr.EndsWith(".root")) { hasReal = kTRUE; break; }
      }
      gSystem->FreeDirectory(dirp2);
    }
    void *dirp3 = gSystem->OpenDirectory(Form("%s/bin1/datafiles/mixed", entry));
    if (dirp3) {
      const char *e;
      while ((e = gSystem->GetDirEntry(dirp3))) {
        TString eStr(e);
        if (eStr.BeginsWith("Fqe_standardErrorSample") && eStr.EndsWith(".root")) { hasMixed = kTRUE; break; }
      }
      gSystem->FreeDirectory(dirp3);
    }
    if (hasReal || hasMixed) {
      dirList.push_back(std::string(entry));
      std::cout << "\033[32mAuto-detected directory: " << entry << "\033[0m" << std::endl;
    }
  }
  gSystem->FreeDirectory(dirp);

  Int_t NDIRS = dirList.size();
  if (NDIRS == 0) {
    std::cout << "\033[31mNo directories found with expected structure\033[0m" << std::endl;
    return;
  }

  // Auto-detect number of bins from the first directory
  Int_t NBINS = 0;
  for (Int_t nBins = 1; nBins <= 100; nBins++) {
    Bool_t found = kFALSE;
    // Check real
    void *dp = gSystem->OpenDirectory(Form("%s/bin%d/datafiles", dirList[0].c_str(), nBins));
    if (dp) {
      const char *e;
      while ((e = gSystem->GetDirEntry(dp))) {
        TString eStr(e);
        if (eStr.BeginsWith("Fqe_standardErrorSample") && eStr.EndsWith(".root")) { found = kTRUE; break; }
      }
      gSystem->FreeDirectory(dp);
    }
    // Check mixed
    if (!found) {
      dp = gSystem->OpenDirectory(Form("%s/bin%d/datafiles/mixed", dirList[0].c_str(), nBins));
      if (dp) {
        const char *e;
        while ((e = gSystem->GetDirEntry(dp))) {
          TString eStr(e);
          if (eStr.BeginsWith("Fqe_standardErrorSample") && eStr.EndsWith(".root")) { found = kTRUE; break; }
        }
        gSystem->FreeDirectory(dp);
      }
    }
    if (!found) break;
    NBINS = nBins;
  }
  std::cout << "\033[32mAuto-detected: " << NDIRS << " directories, " << NBINS << " bins\033[0m" << std::endl;

  TDatime now;
  TString dateStr = Form("%d-%02d-%02d %02d:%02d:%02d", now.GetYear(), now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond());

  for (Int_t nDirs = 0; nDirs < NDIRS; nDirs++) {
    for (Int_t nBins = 0; nBins < NBINS; nBins++) {
      for (Int_t iPath = 0; iPath < 2; iPath++) {
      TString subDir = (iPath == 0) ? "datafiles" : "datafiles/mixed";
      TString dirPath = Form("%s/bin%d/%s", dirList[nDirs].c_str(), nBins + 1, subDir.Data());

      // Find any Fqe_standardErrorSample*.root in this directory
      TString filePath = "";
      void *dp = gSystem->OpenDirectory(dirPath.Data());
      if (!dp) continue;
      const char *e;
      while ((e = gSystem->GetDirEntry(dp))) {
        TString eStr(e);
        if (eStr.BeginsWith("Fqe_standardErrorSample") && eStr.EndsWith(".root")) {
          filePath = Form("%s/%s", dirPath.Data(), eStr.Data());
          break;
        }
      }
      gSystem->FreeDirectory(dp);
      if (filePath == "") continue;

      TFile* f11 = new TFile(filePath.Data(), "READ");
      if (!f11->IsOpen()) {
        std::cout << "\033[31mError opening file " << f11->GetName() << "\033[0m" << std::endl;
        continue;
      }
      std::cout << "\033[32mFile " << f11->GetName() << " opened successfully\033[0m" << std::endl;

      // Read metadata
      TString ptBinName = Form("bin%d", nBins + 1);
      TString rootFilePath = "unknown";
      TString condName = dirList[nDirs];
      TString metaType = "unknown";

      TObjString *metaPt = (TObjString*)f11->Get("meta_ptBinName");
      if (metaPt) ptBinName = metaPt->GetString();
      TObjString *metaFile = (TObjString*)f11->Get("meta_inputFile");
      if (metaFile) rootFilePath = metaFile->GetString();
      TObjString *metaCond = (TObjString*)f11->Get("meta_condName");
      if (metaCond) condName = metaCond->GetString();
      TObjString *metaTypeObj = (TObjString*)f11->Get("meta_type");
      if (metaTypeObj) metaType = metaTypeObj->GetString();

      // Auto-detect Ntmax
      Int_t useNtmax = 0;
      for (Int_t t = 1; t <= 200; t++) {
        TString fNtpname = "FqNtpStndBinM" + TString::Itoa(t, 10);
        TNtuple *ntTest = (TNtuple*)f11->Get(fNtpname.Data());
        if (!ntTest) break;
        useNtmax = t;
      }
      if (useNtmax == 0) {
        std::cout << "\033[31mNo ntuples found in file\033[0m" << std::endl;
        f11->Close();
        continue;
      }

      TNtuple* fntupleM[200];
      TString fNtpname;
      for (Int_t t = 1; t <= useNtmax; t++) {
        fNtpname = "FqNtpStndBinM" + TString::Itoa(t, 10);
        fntupleM[t] = (TNtuple*)f11->Get(fNtpname.Data());
      }

      f11->cd();
      std::ofstream outfile1;
      outfile1.open(Form("%s/bin%d/%s/FqeStdError.csv", dirList[nDirs].c_str(), nBins + 1, subDir.Data()), std::ofstream::out);

      // Write CSV header with metadata
      outfile1 << "# Bin: " << ptBinName.Data() << std::endl;
      outfile1 << "# Condition: " << condName.Data() << std::endl;
      outfile1 << "# Type: " << metaType.Data() << std::endl;
      outfile1 << "# Created: " << dateStr.Data() << std::endl;
      outfile1 << "# ROOT file: " << rootFilePath.Data() << std::endl;
      outfile1 << "M_bins,Avg_bin_content,Fq2,StdErr_Fq2,Fq3,StdErr_Fq3,Fq4,StdErr_Fq4,Fq5,StdErr_Fq5" << std::endl;

      for (Int_t Ntnum = 1; Ntnum <= useNtmax; Ntnum++) {
        if (!fntupleM[Ntnum]) continue;

        const char *cut = "abs(Fqq2) < 1e30 && abs(Fqq3) < 1e30 && abs(Fqq4) < 1e30 && abs(Fqq5) < 1e30 && abs(Fqq6) < 1e30 && abs(Fqq7) < 1e30";

        TCanvas* cc = new TCanvas(Form("c%d_%d_%d", iPath, nDirs, Ntnum), "Mbinx", 5, 5, 200, 200);
        fntupleM[Ntnum]->Draw("nbins");
        TH1F* htemp = (TH1F*)cc->GetPrimitive("htemp");
        Float_t meanbin = htemp ? htemp->GetMean() : 0;

        TCanvas* con = new TCanvas(Form("bc%d_%d_%d", iPath, nDirs, Ntnum), "Average bin content", 15, 15, 200, 300);
        fntupleM[Ntnum]->Draw("Avbin");
        TH1F* htemp1 = (TH1F*)con->GetPrimitive("htemp");
        Float_t meanbincon = htemp1 ? htemp1->GetMean() : 0;

        TCanvas* C1 = new TCanvas(Form("C%d_%d_%d", iPath, nDirs, Ntnum), Form("C%d", Ntnum), 10 + Ntnum * 15, 10 + Ntnum * 15, 600, 500);
        C1->Divide(2, 3);

        Float_t meanf2 = 0, meanf3 = 0, meanf4 = 0, meanf5 = 0;
        Float_t stddev2 = 0, stddev3 = 0, stddev4 = 0, stddev5 = 0;
        Float_t NumEnt = 0;

        C1->cd(1);
        fntupleM[Ntnum]->Draw("Fqq2", cut);
        TH1F* h1 = (TH1F*)C1->cd(1)->GetPrimitive("htemp");
        if (h1 && h1->GetEntries() > 0) {
          meanf2 = h1->GetMean();
          NumEnt = h1->GetEntries();
          stddev2 = (h1->GetStdDev()) / sqrt(NumEnt);
        }

        C1->cd(2);
        fntupleM[Ntnum]->Draw("Fqq3", cut);
        TH1F* h2 = (TH1F*)C1->cd(2)->GetPrimitive("htemp");
        if (h2 && h2->GetEntries() > 0) {
          meanf3 = h2->GetMean();
          stddev3 = (h2->GetStdDev()) / sqrt(NumEnt > 0 ? NumEnt : 1);
        }

        C1->cd(3);
        fntupleM[Ntnum]->Draw("Fqq4", cut);
        TH1F* h3 = (TH1F*)C1->cd(3)->GetPrimitive("htemp");
        if (h3 && h3->GetEntries() > 0) {
          meanf4 = h3->GetMean();
          stddev4 = (h3->GetStdDev()) / sqrt(NumEnt > 0 ? NumEnt : 1);
        }

        C1->cd(4);
        fntupleM[Ntnum]->Draw("Fqq5", cut);
        TH1F* h4 = (TH1F*)C1->cd(4)->GetPrimitive("htemp");
        if (h4 && h4->GetEntries() > 0) {
          meanf5 = h4->GetMean();
          stddev5 = (h4->GetStdDev()) / sqrt(NumEnt > 0 ? NumEnt : 1);
        }

        C1->cd(5);
        fntupleM[Ntnum]->Draw("Fqq6", cut);
        TH1F* h5 = (TH1F*)C1->cd(5)->GetPrimitive("htemp");
        if (h5 && h5->GetEntries() > 0) {
          Float_t meanf6 = h5->GetMean();
        }

        C1->cd(6);
        fntupleM[Ntnum]->Draw("Fqq7", cut);
        TH1F* h6 = (TH1F*)C1->cd(6)->GetPrimitive("htemp");
        if (h6 && h6->GetEntries() > 0) {
          Float_t meanf7 = h6->GetMean();
        }

        outfile1 << (Int_t)meanbin << "," << meanbincon << "," << meanf2 << "," << stddev2 << ","
                 << meanf3 << "," << stddev3 << "," << meanf4 << "," << stddev4 << ","
                 << meanf5 << "," << stddev5 << std::endl;
      }

      outfile1.close();
      f11->Close();
      } // iPath
    }
  }
}
