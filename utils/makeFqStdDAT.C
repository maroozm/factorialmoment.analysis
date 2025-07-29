#include <iostream>
#include <fstream>
#include <TStyle.h>
#include <TNtuple.h>
#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>

void makeFqStdDAT()
{
  const Int_t NDIRS = 25;
  const Int_t NBINS = 2;
  const char* dirName[NDIRS] = { "fb128", "fb768", "fb128wcuts", "fb128Prim", "fb128womat", "fb128wowd", "fb128wcutsPrim", "fb128wcutswomat", "fb128wcutswowd", "fb768Prim", "fb768womat", "fb768wowd", "fb128i1", "fb128i2", "fb128wcutsi1", "fb128wcutsi2", "fb128i1i2", "fb128wcutsPrimi1", "fb128wcutsPrimi2", "fb768i1", "fb768i2", "fb768i1i2", "fb768Primi1", "fb768Primi2", "gen" };
  //const char* dirName[NDIRS] = { "fb128" };

  const Int_t Ntmax = 53;
  const TString Name = "C";
  for (Int_t nDirs = 0; nDirs < NDIRS; nDirs++) {
    for (Int_t nBins = 0; nBins < NBINS; nBins++) {

      TString fNtpname;
      TString canName[Ntmax];   // canvases to draw the fqs
      TString nt[Ntmax];        // canvases to draw the fqs
      TNtuple* fntupleM[Ntmax]; // getting Ntuples from the File
      Float_t NumBin[Ntmax], Avbin[Ntmax];

      TFile* f11 = new TFile(Form("%s/bin%d/datafiles/Fqe_standardErrorSample.root", dirName[nDirs], nBins + 1), "READ"); // File: .root file Fqe Ntuples
      if (!f11) {
        std::cout << "\033[31m"
                  << "Error opening file " << f11->GetName() << "\033[0m" << std::endl;
        exit(1);
      } else {
        std::cout << "\033[32m"
                  << "File " << f11->GetName() << " opened successfully \n"
                  << "\033[0m" << std::endl;
      }

      for (Int_t t = 1; t < Ntmax; t++) {
        fNtpname = "FqNtpStndBinM";
        fNtpname += t;
        fntupleM[t] = (TNtuple*)f11->Get(fNtpname.Data());
      }

      for (Int_t t = 0; t < Ntmax; t++) {
        canName[t] = Name;
        canName[t] += t;
      }

      Int_t x[Ntmax], y[Ntmax]; // x,y coordinates of the canvases
      for (Int_t ii = 0; ii < Ntmax; ii++) {
        x[ii] = 10 + (ii * 15);
        y[ii] = 10 + (ii * 15);
      }
      f11->cd();
      TString bin = "c";
      TString bincon = "bincontent";

      for (Int_t loop = 0; loop < Ntmax; loop++) {
        Avbin[loop] = 0.0;
        NumBin[loop] = 0.0;
      }
      std::ofstream outfile1;
      outfile1.open(Form("%s/bin%d/datafiles/FqeStdError.dat", dirName[nDirs], nBins + 1), std::ofstream::out); // Output file to store

      for (Int_t Ntnum = 1; Ntnum < Ntmax; Ntnum++) { //...... Loop over number of bins
        bin += 1;
        bincon += 1;
        //----------------------------------------------------
        //           F_q Plots and mean value
        //---------------------------------------------
        TCanvas* cc = new TCanvas(bin, "Mbinx", 5, 5, 200, 200);
        fntupleM[Ntnum]->Draw("nbins");
        TH1F* htemp = (TH1F*)cc->GetPrimitive("htemp");
        Float_t meanbin = htemp->GetMean(); //...........Number of bins in on eaxis

        //-----------Avg bin content--------------------------
        TCanvas* con = new TCanvas(bincon, "Average bin content", 15, 15, 200, 300);
        fntupleM[Ntnum]->Draw("Avbin");
        TH1F* htemp1 = (TH1F*)con->GetPrimitive("htemp");
        Float_t meanbincon = htemp1->GetMean(); // .........

        // -------Canvas drawing--------------for drawing histos's
        TCanvas* C1 = new TCanvas(canName[Ntnum], canName[Ntnum], x[Ntnum], y[Ntnum], 600, 500);
        Float_t* row_content;
        C1->Divide(2, 3);

        //-------------------q = 2---------------------------------
        C1->cd(1);
        fntupleM[Ntnum]->Draw("Fqq2");
        fntupleM[Ntnum]->Draw("Fqq2");
        TH1F* h1 = (TH1F*)C1->cd(1)->GetPrimitive("htemp");
        Float_t meanf2 = h1->GetMean();
        Float_t NumEnt = h1->GetEntries();
        Float_t stddev2 = (h1->GetStdDev()) / sqrt(NumEnt);
        // meanf2 = meanf2 / (meanbincon**2.0);  // As per standard definition
        //---------------------q = 3-------------------------------
        C1->cd(2);
        fntupleM[Ntnum]->Draw("Fqq3");
        TH1F* h2 = (TH1F*)C1->cd(2)->GetPrimitive("htemp");
        Float_t meanf3 = h2->GetMean();
        Float_t stddev3 = (h2->GetStdDev()) / sqrt(NumEnt);
        // meanf3 = meanf3 / (meanbincon**3.0);
        //---------------------q = 4-------------------------------
        C1->cd(3);
        fntupleM[Ntnum]->Draw("Fqq4");
        TH1F* h3 = (TH1F*)C1->cd(3)->GetPrimitive("htemp");
        Float_t meanf4 = h3->GetMean();
        Float_t stddev4 = (h3->GetStdDev()) / sqrt(NumEnt);
        //  meanf4 = meanf4 / (meanbincon**4.0);
        //---------------------q = 5-------------------------------
        C1->cd(4);
        fntupleM[Ntnum]->Draw("Fqq5");
        TH1F* h4 = (TH1F*)C1->cd(4)->GetPrimitive("htemp");
        Float_t meanf5 = h4->GetMean();
        Float_t stddev5 = (h4->GetStdDev()) / sqrt(NumEnt);
        //  meanf5 = meanf5 / (meanbincon**5.0);

        //---------------------q = 6-------------------------------
        C1->cd(5);
        fntupleM[Ntnum]->Draw("Fqq6");
        TH1F* h5 = (TH1F*)C1->cd(5)->GetPrimitive("htemp");
        Float_t meanf6 = h5->GetMean();
        Float_t stddev6 = (h5->GetStdDev()) / sqrt(NumEnt);
        //  meanf5 = meanf5 / (meanbincon**5.0);

        //---------------------q = 7-------------------------------
        C1->cd(6);
        fntupleM[Ntnum]->Draw("Fqq7");
        TH1F* h6 = (TH1F*)C1->cd(6)->GetPrimitive("htemp");
        Float_t meanf7 = h6->GetMean();
        Float_t stddev7 = (h6->GetStdDev()) / sqrt(NumEnt);
        //  meanf5 = meanf5 / (meanbincon**5.0);

        // -------------------------------------------------------------
        // WRITTING INTO DATA FILE
        //---------------------------------------------------------
        outfile1 << meanbin << "\t" << meanbincon << "\t" << meanf2 << "\t" << stddev2 << "\t" << meanf3
                 << "\t" << stddev3 << "\t" << meanf4 << "\t" << stddev4 << "\t" << meanf5 << "\t" << stddev5 /* << "\t" << meanf6 << "\t" << stddev6 << "\t" << meanf7 << "\t" << stddev7 */ << "\n";
      }

      outfile1.close();
    } // end of loop over bins
  } // end of loop over dirs
}
