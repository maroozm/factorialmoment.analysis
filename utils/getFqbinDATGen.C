#include <iostream>
#include <fstream>
#include <TStyle.h>
#include <TNtuple.h>
#include <TFile.h>
#include <sys/stat.h>
#include <sys/types.h>

void getFqbinDATGen()
{
  const Int_t Ntmax = 52;
  const Int_t SAMPLES = 100; // Change here to increase or decrease number of samples
  Bool_t deBug = 0;

  const Int_t files = 1;
  TFile* file[files];
  file[0] = new TFile("./AnalysisResults.root", "read");
  // NOTE: In case you have more than 2 files and a bunch of wagons, please read not more than 2 files at a time.

  const Int_t wagons = 1;
  const char* wagonName[wagons] = { "gen" };
  const Int_t bins = 2;
  const char* ptName[bins]= {"0.40to2.00","0.40to0.60"};

  TString nt[Ntmax], ent[Ntmax];
  TNtuple* fntupleM[Ntmax];
  TNtuple* FqStanDefM[Ntmax];
  TNtuple* EvFactDefM[Ntmax];

  Float_t SumFqeq2 = 0.0, SumFqeq3 = 0.0, SumFqeq4 = 0.0, SumFqeq5 = 0.0, SumFqeq6 = 0.0, SumFqeq7 = 0.0;
  Float_t SumAvBincon = 0.0;
  Float_t Fqq2, Fqq3, Fqq4, Fqq5, Fqq6, Fqq7, RFqq2, RFqq3, RFqq4, RFqq5, RFqq6, RFqq7;
  Float_t nevents, nbins, av;
  Float_t NumBin, Avbin, Avbin2, Avbin3, Avbin4, Avbin5, Avbin6, Avbin7, aav;

  // -----------Output files----------------
  std::ofstream outfile1, outfile2;

  for (Int_t nFiles = 0; nFiles < files; nFiles++) {
    Int_t name = nFiles + 1; // just in case you read files in groups
    if (!file[nFiles]->IsOpen()) {
      std::cout << "\033[31m"
                << "Error opening file " << nFiles << "\033[0m" << std::endl;
      exit(1);
    } else {
      std::cout << "\033[32m"
                << "File " << file[nFiles]->GetName() << " opened successfully \n"
                << "\033[0m" << std::endl;
    }
    for (Int_t nWagons = 0; nWagons < wagons; nWagons++) {
      for (Int_t nBins = 0; nBins < bins; nBins++) {

        std::string outputDir = *wagonName[nWagons] != 0 ? wagonName[nWagons] : "default";
        outputDir += "/bin" + std::to_string(nBins + 1) + "/datafiles/";
        std::filesystem::path dir(outputDir);
        std::cout << " Looking for directory " << outputDir << std::endl;
        if (!std::filesystem::exists(dir)) {
          std::cout << " Did not find directory " << outputDir << " Creating one " << std::endl;
          std::filesystem::create_directories(dir);
        }
        outfile1.open(Form("%sFqe_StdErrorSubSampling%d.dat", outputDir.c_str(), name), std::ofstream::out); // to store Fq : standard definition for sample of events
        outfile2.open(Form("%sFqebeforeRatio%d.dat", outputDir.c_str(), name), std::ofstream::out);          // Numerator part of above definition is sampled.

        // file[nFiles]->ls();

        TList* list1 = (TList*)file[nFiles]->Get(Form("ntplistGen%dtaskfm", 1));
        if (!list1) {
          std::cout << "Error getting list " << Form("ntplistGen%dtaskfm", 1) << std::endl;
          exit(1);
        } else {
          std::cout << " Reading list " << Form("ntplistGen%dtaskfm", 1) << std::endl;
        }
        list1->ls();
        for (Int_t t = 0; t < Ntmax; t++) {
          //TString fNtpname = Form("fntpMBingenbin0.20to2.00_M%d", t);
          TString fNtpname = Form("fntpMBingenbin%s_M%d", ptName[nBins], t);
          // fNtpname += t + 1;
          fntupleM[t] = (TNtuple*)list1->FindObject(fNtpname.Data());
        }

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

        for (Int_t Ntnum = 0; Ntnum < Ntmax; Ntnum++) { //...... Loop over number of bins
          Int_t TEvents = 0;
          Int_t counter = 0;
          Int_t events = 0;
          Int_t Evloop_Counter = 0;
          Int_t limit = 0;
          Int_t status = 0;
          Float_t Avbin = 0.0;
          Float_t NumBin = 0.0;

          TEvents = fntupleM[Ntnum]->GetEntries();
          Evloop_Counter = ceil(TEvents / SAMPLES);
          limit = TEvents - (floor(TEvents / SAMPLES) * SAMPLES);
          if (deBug)
            std::cout << TEvents << "events  " << Evloop_Counter << "  " << limit << std::endl;

          for (Int_t irow = 0; irow < (TEvents - limit); irow++) { // EVENT LOOP
            status = 0;
            events++;

            fntupleM[Ntnum]->GetEntry(irow);
            row_content = fntupleM[Ntnum]->GetArgs();

            NumBin = row_content[0];
            Float_t aav = row_content[1];
            SumFqeq2 += row_content[2];
            SumFqeq3 += row_content[3];
            SumFqeq4 += row_content[4];
            SumFqeq5 += row_content[5];
            SumFqeq6 += row_content[6];
            SumFqeq7 += row_content[7];

            SumAvBincon += aav;

            if (events == Evloop_Counter) {
              counter++;
              status = 1;
            }
            if (counter == SAMPLES && events == limit)
              status = 2;

            if (status == 1 || status == 2) {

              Int_t Totevents = events; // Number of Events counted
              nevents = events;
              nbins = NumBin;
              //-----------No Correction-------------
              Avbin = SumAvBincon / events;

              Float_t AvFqeq2 = SumFqeq2 / Totevents; //
              Float_t AvFqeq3 = SumFqeq3 / Totevents;
              Float_t AvFqeq4 = SumFqeq4 / Totevents;
              Float_t AvFqeq5 = SumFqeq5 / Totevents;
              Float_t AvFqeq6 = SumFqeq6 / Totevents;
              Float_t AvFqeq7 = SumFqeq7 / Totevents;

              Fqq2 = AvFqeq2 / (pow(Avbin, 2.0)); // q = 2     // F_q Moments  as per stand definition for q = 2,3,4,5
              Fqq3 = AvFqeq3 / (pow(Avbin, 3.0)); // q = 3
              Fqq4 = AvFqeq4 / (pow(Avbin, 4.0)); // q = 4
              Fqq5 = AvFqeq5 / (pow(Avbin, 5.0)); // q = 5
              Fqq6 = AvFqeq6 / (pow(Avbin, 6.0)); // q = 5
              Fqq7 = AvFqeq7 / (pow(Avbin, 7.0)); // q = 5

              Avbin2 = pow(Avbin, 2.0);
              Avbin3 = pow(Avbin, 3.0);
              Avbin4 = pow(Avbin, 4.0);
              Avbin5 = pow(Avbin, 5.0);
              Avbin6 = pow(Avbin, 6.0);
              Avbin7 = pow(Avbin, 7.0);
              if (deBug)
                std::cout << Avbin << "\t" << Avbin2 << "\t" << Avbin3 << "\t" << Avbin4 << "\t" << Avbin5 << Avbin6 << Avbin7 << std::endl;
              RFqq2 = AvFqeq2; // Fq Moments : numerator----->sampling
              RFqq3 = AvFqeq3;
              RFqq4 = AvFqeq4;
              RFqq5 = AvFqeq5;
              RFqq6 = AvFqeq6;
              RFqq6 = AvFqeq7;
              //-----------With Correction-------------

              //-----------------WRITING TO FILES-------------------------

              outfile1 << " " << nevents << "  " << nbins << " " << Avbin << " " << Fqq2 << " " << Fqq3 << " " << Fqq4 << " " << Fqq5 << " " << Fqq6 << " " << Fqq7 << std::endl;
              outfile2 << " " << nevents << "  " << nbins << " " << Avbin << " " << Avbin2 << "  " << Avbin3 << "  " << Avbin4 << "  " << Avbin5 << "  " << Avbin6 << "  " << Avbin7 << " " << RFqq2 << " " << RFqq3 << " " << RFqq4 << " " << RFqq5 << " " << RFqq6 << " " << RFqq7 << std::endl;

              FqStanDefM[Ntnum]->Fill(nevents, nbins, Avbin, Fqq2, Fqq3, Fqq4, Fqq5, Fqq6, Fqq7);
              EvFactDefM[Ntnum]->Fill(nevents, nbins, Avbin, Avbin2, Avbin3, Avbin4, Avbin5, Avbin6, Avbin7, RFqq2, RFqq3, RFqq4, RFqq5, RFqq6, RFqq7);

              events = 0.;
              status = 0;
              SumAvBincon = 0.;
              SumFqeq2 = SumFqeq3 = SumFqeq4 = SumFqeq5 = SumFqeq6 = SumFqeq7 = 0.;

            } // if(status) close

          } // here ends event loop

        } // M loop

        delete row_content;

        Samplefile->Write();
        Samplefile->Close();
        Rudy->Write();
        Rudy->Close();

        outfile1.close();
        outfile2.close();
      } // nBins loop
    } // nWagons loop
  } // nFiles loop
}
