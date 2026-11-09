#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TKey.h>
#include <TLatex.h>
#include <TList.h>
#include <TROOT.h>
#include <TString.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TH1.h>
#include <TH2.h>
#include <TObject.h>

struct RowSource {
   TString label;
   TString listName;
   TString ptRange;
   TList *list = nullptr;
   Bool_t isGen = kFALSE;
   Bool_t isData = kFALSE;
};

static TString ExtractConfigLabel(const TString &listName)
{
   TString label = listName;
   if (label.BeginsWith("list_")) label.Remove(0, 5);

   Ssiz_t mcPos = label.Index("MC_");
   Ssiz_t dataPos = label.Index("DATA_");
   if (mcPos != kNPOS && (dataPos == kNPOS || mcPos < dataPos))
      label.Remove(0, mcPos + 3);
   else if (dataPos != kNPOS)
      label.Remove(0, dataPos + 5);

   // Strip the production suffix, e.g. _1pts_05cent_taskfm or _3pts_05cent_taskfm.
   Ssiz_t ptsPos = label.Index("pts_");
   if (ptsPos != kNPOS) {
      Ssiz_t usPos = ptsPos;
      while (usPos > 0 && label[usPos - 1] != '_') --usPos;
      if (usPos > 0) label.Remove(usPos - 1);
   }

   return label;
}

static TString ExtractYear(const TString &listName)
{
   TString year = listName;
   if (year.BeginsWith("list_")) year.Remove(0, 5);
   Ssiz_t mcPos = year.Index("MC_");
   if (mcPos != kNPOS) {
      year.Remove(mcPos);
      return year;
   }
   TString digits;
   for (Ssiz_t i = 0; i < year.Length(); ++i) {
      if (year[i] >= '0' && year[i] <= '9')
         digits += year[i];
      else if (digits.Length() > 0)
         break;
   }
   if (digits.Length() > 0) return digits;
   return TString("?");
}

static TString ExtractPtRange(const TString &histName)
{
   Ssiz_t binPos = histName.Index("bin");
   if (binPos == kNPOS) return TString("?");
   TString range = histName;
   range.Remove(0, binPos + 3);
   Ssiz_t usPos = range.Index("_");
   if (usPos != kNPOS) range.Remove(usPos);
   return range;
}

static std::vector<TString> ExtractDataPtRanges(TList *list, const TString &configLabel)
{
   std::vector<TString> ranges;
   if (!list) return ranges;

   const TString prefix = "hMultBinbin";
   TString suffix = Form("_%s", configLabel.Data());

   TIter next(list);
   TObject *obj = nullptr;
   while ((obj = next())) {
      TString name = obj->GetName();
      if (!name.BeginsWith(prefix)) continue;
      if (!name.EndsWith(suffix)) continue;

      TString range = name;
      range.Remove(0, prefix.Length());
      Ssiz_t usPos = range.Index("_");
      if (usPos != kNPOS) range.Remove(usPos);
      if (range.Length() == 0) continue;

      Bool_t known = kFALSE;
      for (const auto &existing : ranges) {
         if (existing == range) {
            known = kTRUE;
            break;
         }
      }
      if (!known) ranges.push_back(range);
   }

   return ranges;
}

static TObject *FindMatchingObject(TList *list, const std::vector<TString> &exactNames, const TString &prefix,
                                   const TString &suffix = TString())
{
   if (!list) return nullptr;

   for (const auto &name : exactNames) {
      if (name.Length() == 0) continue;
      TObject *obj = list->FindObject(name.Data());
      if (obj) return obj;
   }

   TIter next(list);
   TObject *obj = nullptr;
   while ((obj = next())) {
      TString name = obj->GetName();
      if (!name.BeginsWith(prefix)) continue;
      if (suffix.Length() > 0 && !name.EndsWith(suffix)) continue;
      return obj;
   }

   return nullptr;
}

static TObject *FindPlotObject(TList *list, Int_t col, const TString &configLabel, Bool_t isGenRow,
                               Bool_t isDataRow = kFALSE, const TString &ptRange = TString())
{
   std::vector<TString> exactNames;
   TString prefix;
   TString suffix;

   switch (col) {
   case 0:
      if (isGenRow) {
         exactNames = {"hMultBinGenbin0.40to2.00"};
         prefix = "hMultBinGenbin";
      } else if (isDataRow) {
         if (ptRange.Length() > 0) {
            exactNames = {Form("hMultBinbin%s_%s", ptRange.Data(), configLabel.Data())};
            prefix = "hMultBin";
            suffix = Form("%s_%s", ptRange.Data(), configLabel.Data());
         } else {
            prefix = "hMultBinbin";
            suffix = configLabel;
         }
      } else {
         exactNames = {Form("hMultBinrecbin0.40to2.00_%s", configLabel.Data())};
         prefix = "hMultBinrecbin";
         suffix = configLabel;
      }
      break;
   case 1:
      if (isGenRow) {
         exactNames = {"hEtaBinGenbin0.40to2.00"};
         prefix = "hEtaBinGenbin";
      } else if (isDataRow) {
         if (ptRange.Length() > 0) {
            exactNames = {Form("hEtaBinbin%s_%s", ptRange.Data(), configLabel.Data())};
            prefix = "hEtaBin";
            suffix = Form("%s_%s", ptRange.Data(), configLabel.Data());
         } else {
            prefix = "hEtaBinbin";
            suffix = configLabel;
         }
      } else {
         exactNames = {Form("hEtaBinrecbin0.40to2.00_%s", configLabel.Data())};
         prefix = "hEtaBinrecbin";
         suffix = configLabel;
      }
      break;
   case 2:
      if (isGenRow) {
         exactNames = {"hPhiBinGenbin0.40to2.00"};
         prefix = "hPhiBinGenbin";
      } else if (isDataRow) {
         if (ptRange.Length() > 0) {
            exactNames = {Form("hPhiBinbin%s_%s", ptRange.Data(), configLabel.Data())};
            prefix = "hPhiBin";
            suffix = Form("%s_%s", ptRange.Data(), configLabel.Data());
         } else {
            prefix = "hPhiBinbin";
            suffix = configLabel;
         }
      } else {
         exactNames = {Form("hPhiBinrecbin0.40to2.00_%s", configLabel.Data())};
         prefix = "hPhiBinrecbin";
         suffix = configLabel;
      }
      break;
   case 3:
      exactNames = {Form("hNFoundClsVsPt%s", configLabel.Data())};
      prefix = "hNFoundClsVsPt";
      suffix = configLabel;
      break;
   case 4:
      exactNames = {Form("hNShClsVsPt%s", configLabel.Data())};
      prefix = "hNShClsVsPt";
      suffix = configLabel;
      break;
   case 5:
      exactNames = {Form("hNShClsFra%s", configLabel.Data())};
      prefix = "hNShClsFra";
      suffix = configLabel;
      break;
   case 6:
      exactNames = {Form("hDCAxypT%s", configLabel.Data())};
      prefix = "hDCAxypT";
      suffix = configLabel;
      break;
   case 7:
      exactNames = {Form("hDCAzpT%s", configLabel.Data())};
      prefix = "hDCAzpT";
      suffix = configLabel;
      break;
   case 8:
      exactNames = {Form("hnTPCcrossedrows%s", configLabel.Data())};
      prefix = "hnTPCcrossedrows";
      suffix = configLabel;
      break;
   case 9:
      exactNames = {Form("hnTPCcls%s", configLabel.Data())};
      prefix = "hnTPCcls";
      suffix = configLabel;
      break;
   default:
      return nullptr;
   }

   return FindMatchingObject(list, exactNames, prefix, suffix);
}

static void StylePad(TPad *pad, Bool_t is2D)
{
   pad->SetFrameBorderMode(0);
   pad->SetTicks(1, 1);
   pad->SetLeftMargin(0.12);
   pad->SetBottomMargin(0.12);
   pad->SetTopMargin(0.08);
   pad->SetRightMargin(is2D ? 0.15 : 0.04);
}

static void DrawMissingCell(Int_t row, Int_t col)
{
   TLatex latex;
   latex.SetNDC();
   latex.SetTextAlign(22);
   latex.SetTextFont(42);
   latex.SetTextSize(0.08);
   latex.DrawLatex(0.50, 0.52, "N/A");

   if (row == 0 && col == 0) {
      latex.SetTextSize(0.05);
      latex.DrawLatex(0.50, 0.25, "No plots found");
   }
}

void plot_configs_grid(const char *rootFile, const char *outName = "", Bool_t rebin = kTRUE)
{
   gROOT->SetStyle("ATLAS");
   gStyle->SetImageScaling(2);
   gStyle->SetOptStat(0);

   const int nRGB = 256;
   Double_t rArr[nRGB], gArr[nRGB], bArr[nRGB];
   Bool_t useCork = kFALSE;
   TString palPath = gSystem->GetDirName(__FILE__) + "/corkO.txt";
   ifstream palFile(palPath.Data());
   if (!palFile.is_open()) {
      std::cout << "Cannot open corkO.txt at: " << palPath << std::endl;
      gStyle->SetPalette(kBird);
   } else {
      for (int i = 0; i < nRGB; i++) palFile >> rArr[i] >> gArr[i] >> bArr[i];
      palFile.close();
      Double_t stops[nRGB];
      for (int i = 0; i < nRGB; i++) stops[i] = (double)i / (nRGB - 1);
      TColor::CreateGradientColorTable(nRGB, stops, rArr, gArr, bArr, nRGB);
      gStyle->SetNumberContours(nRGB);
      useCork = kTRUE;
   }
   Int_t fillCol = useCork ? gStyle->GetColorPalette(100) : (kAzure - 9);
   Int_t lineCol = useCork ? gStyle->GetColorPalette(210) : (kAzure + 2);
   Int_t infoCol = useCork ? gStyle->GetColorPalette(30) : (kBlue + 2);

   TFile *f = TFile::Open(rootFile, "READ");
   if (!f || f->IsZombie()) {
      std::cout << "Cannot open file: " << rootFile << std::endl;
      return;
   }

   std::vector<RowSource> rows;

   TIter nextKey(f->GetListOfKeys());
   TKey *key = nullptr;
   while ((key = (TKey *)nextKey())) {
      if (strcmp(key->GetClassName(), "TList") != 0) continue;

      TString keyName = key->GetName();

      if (keyName.BeginsWith("list_") && keyName.EndsWith("_taskfm")) {
         RowSource row;
         row.label = ExtractConfigLabel(keyName);
         row.listName = keyName;
         row.list = nullptr;
         row.isGen = (row.label == "gen");
         row.isData = keyName.Contains("DATA_");
         rows.push_back(row);
      }
   }

   if (rows.empty()) {
      std::cout << "No configuration lists found in: " << rootFile << std::endl;
      f->Close();
      return;
   }

   Bool_t hasGenRow = kFALSE;
   Bool_t hasMcRow = kFALSE;
   TString genListName;
   for (const auto &row : rows) {
      if (row.isGen) hasGenRow = kTRUE;
      if (!row.isData) {
         hasMcRow = kTRUE;
         if (genListName.Length() == 0) genListName = row.listName;
      }
   }

   // MC files store the generator-level histograms in every reconstruction list.
   // DATA files have no generator-level histograms, so do not add a gen row there.
   if (!hasGenRow && hasMcRow && genListName.Length() > 0) {
      RowSource genRow;
      genRow.label = "gen";
      genRow.listName = genListName;
      genRow.list = nullptr;
      genRow.isGen = kTRUE;
      genRow.isData = kFALSE;
      rows.push_back(genRow);
   }

   // DATA files can contain several pT bins in one list. Add one row per pT bin
   // for DATA only; MC keeps its existing one-row-per-list layout.
   std::vector<RowSource> expandedRows;
   for (const auto &row : rows) {
      if (!row.isData) {
         expandedRows.push_back(row);
         continue;
      }

      std::vector<TString> ptRanges;
      TList *dataList = (TList *)f->Get(row.listName);
      if (dataList) {
         ptRanges = ExtractDataPtRanges(dataList, row.label);
         dataList->Delete();
      }

      if (ptRanges.empty()) {
         expandedRows.push_back(row);
         continue;
      }

      for (const auto &ptRange : ptRanges) {
         RowSource dataRow = row;
         dataRow.ptRange = ptRange;
         expandedRows.push_back(dataRow);
      }
   }
   rows.swap(expandedRows);

   const Int_t cols = 11;
   const Int_t rowsCount = static_cast<Int_t>(rows.size());
   const Int_t canvasW = std::max(1800, cols * 260);
   const Int_t canvasH = std::max(800, rowsCount * 220);
   TCanvas *c1 = new TCanvas("c1_plot_configs_grid", "c1_plot_configs_grid", canvasW, canvasH);

   const Double_t xgap = 0.004, ygap = 0.004;
   const Double_t wPlot = (1.0 - (cols + 1) * xgap) / 10.5;
   const Double_t wLabel = 0.5 * wPlot;
   const Double_t hRow = (1.0 - (rowsCount + 1) * ygap) / rowsCount;
   std::vector<Double_t> colX(cols + 1, 0.0);
   colX[0] = xgap;
   colX[1] = colX[0] + wLabel + xgap;
   for (Int_t cc = 2; cc <= cols; ++cc) colX[cc] = colX[cc - 1] + wPlot + xgap;

   const char *colTitles[cols] = {"",            "hMultBin...",          "hEtaBin...",          "hPhiBin...",
                                   "hNFoundClsVsPt...", "hNShClsVsPt...", "hNShClsFra...",    "hDCAxypT...",
                                   "hDCAzpT...",        "hnTPCcrossedrows...", "hnTPCcls..."};
   const Bool_t colIs2D[cols] = {kFALSE, kFALSE, kFALSE, kFALSE, kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kFALSE, kFALSE};

   TString activeListName;
   TList *activeList = nullptr;
   for (Int_t r = 0; r < rowsCount; ++r) {
      RowSource &row = rows[r];
      if (!activeList || row.listName != activeListName) {
         if (activeList) {
            activeList->Delete();
            activeList = nullptr;
         }
         activeListName = row.listName;
         activeList = (TList *)f->Get(row.listName);
         if (!activeList) std::cout << "Cannot load list: " << row.listName << std::endl;
      }
      row.list = activeList;
      if (!row.list) continue;

      for (Int_t c = 0; c < cols; ++c) {
         Double_t y2 = 1.0 - ygap - r * (hRow + ygap);
         Double_t y1 = y2 - hRow;
         c1->cd();
         TPad *pad = new TPad(Form("pad_r%d_c%d", r, c), "", colX[c], y1, colX[c + 1] - xgap, y2);
         pad->Draw();
         pad->cd();
         StylePad(pad, colIs2D[c]);

         if (c == 0) {
            pad->cd();
            TLatex rowLabel;
            rowLabel.SetNDC();
            rowLabel.SetTextFont(42);
            rowLabel.SetTextSize(row.ptRange.Length() > 0 ? 0.075 : 0.11);
            rowLabel.SetTextAlign(22);
            rowLabel.SetTextAngle(90);
            rowLabel.SetTextColor(kAzure + 2);
            if (row.ptRange.Length() > 0) {
               rowLabel.DrawLatex(0.35, 0.50, Form("#bf{%s}", row.label.Data()));
               rowLabel.DrawLatex(0.65, 0.50, Form("#bf{%s}", row.ptRange.Data()));
            } else {
               rowLabel.DrawLatex(0.50, 0.50, Form("#bf{%s}", row.label.Data()));
            }
            continue;
         }

         Int_t plotCol = c - 1;
         TObject *obj = FindPlotObject(row.list, plotCol, row.label, row.isGen, row.isData, row.ptRange);
         if (!obj) {
            DrawMissingCell(r, c);
            continue;
         }

         if (obj->InheritsFrom(TH2::Class())) {
            TH2 *h2 = (TH2 *)obj;
            if (rebin) {
               Int_t nx = h2->GetNbinsX();
               Int_t ny = h2->GetNbinsY();
               Int_t rebinX = 1, rebinY = 1;
               if (nx > 500) rebinX = nx / 50;
               if (ny > 500) rebinY = ny / 50;
               if (rebinX > 1 || rebinY > 1) h2->Rebin2D(rebinX, rebinY);
            }
            h2->SetTitle(colTitles[c]);
            h2->GetXaxis()->SetTitleSize(0.06);
            h2->GetYaxis()->SetTitleSize(0.06);
            h2->GetXaxis()->SetLabelSize(0.05);
            h2->GetYaxis()->SetLabelSize(0.05);
            h2->GetZaxis()->SetLabelSize(0.05);
            if (c == 4) {
               h2->GetYaxis()->SetRangeUser(0.0, 3.0);
            } else if (c == 5) {
               h2->GetYaxis()->SetRangeUser(0.0, 3.0);
            } else if (c == 6) {
               h2->GetXaxis()->SetRangeUser(0.0, 0.6);
               h2->GetYaxis()->SetRangeUser(0.0, 0.6);
            } else if (c == 7) {
               h2->GetYaxis()->SetRangeUser(-1.0, 1.0);
            } else if (c == 8) {
               h2->GetYaxis()->SetRangeUser(-2.0, 2.0);
            }
            h2->DrawCopy("colz");
         } else if (obj->InheritsFrom(TH1::Class())) {
            TH1 *h1 = (TH1 *)obj;
            h1->SetLineWidth(2);
            h1->SetLineColor(lineCol);
            h1->SetFillColor(fillCol);
            h1->SetFillStyle(1001);
            h1->SetTitle(colTitles[c]);
            h1->GetXaxis()->SetTitleSize(0.06);
            h1->GetYaxis()->SetTitleSize(0.06);
            h1->GetXaxis()->SetLabelSize(0.05);
            h1->GetYaxis()->SetLabelSize(0.05);
            if (c == 1) {
               Int_t nb = h1->GetNbinsX();
               Int_t first = 1, last = nb;
               while (first <= nb && h1->GetBinContent(first) <= 0) ++first;
               while (last >= first && h1->GetBinContent(last) <= 0) --last;
               if (first <= last) {
                  Double_t xlo = h1->GetBinLowEdge(first) - 100;
                  Double_t xhi = h1->GetBinLowEdge(last) + h1->GetBinWidth(last) + 100;
                  h1->GetXaxis()->SetRangeUser(xlo, xhi);
               }
            } else if (c == 9) {
               h1->GetXaxis()->SetRangeUser(40.0, h1->GetXaxis()->GetXmax());
               pad->SetLogy();
            } else if (c == 10) {
               pad->SetLogy();
            }
            h1->DrawCopy("hist");
            if (c == 1) {
               TLatex meanTex;
               meanTex.SetNDC();
               meanTex.SetTextFont(42);
               meanTex.SetTextSize(0.055);
               meanTex.SetTextAlign(13);
               meanTex.DrawLatex(0.55, 0.85, Form("Mean = %.2f", h1->GetMean()));
            }
         } else {
            DrawMissingCell(r, c);
         }
      }
   }

   if (activeList) {
      activeList->Delete();
      activeList = nullptr;
   }

   c1->cd();
   {
      TString yearAll = ExtractYear(rows[0].listName);
      TString infoText;
      if (rows[0].isData) {
         infoText = Form("%s, DATA", yearAll.Data());
      } else {
         TString ptAll("?");
         TList *tmpList0 = (TList *)f->Get(rows[0].listName);
         if (tmpList0) {
            TObject *mult0 = FindPlotObject(tmpList0, 0, rows[0].label, rows[0].isGen, rows[0].isData,
                                            rows[0].ptRange);
            if (mult0) ptAll = ExtractPtRange(mult0->GetName());
            tmpList0->Delete();
         }
         infoText = Form("%s, %s", yearAll.Data(), ptAll.Data());
      }

      TLatex infoTex;
      infoTex.SetNDC();
      infoTex.SetTextFont(42);
      infoTex.SetTextSize(0.02);
      infoTex.SetTextAlign(22);
      infoTex.SetTextAngle(90);
      infoTex.SetTextColor(infoCol);
      infoTex.DrawLatex(0.012, 0.50, infoText);
   }

   TString outputName = outName;
   if (outputName.Length() == 0) {
      TString base = gSystem->BaseName(rootFile);
      Ssiz_t dot = base.Last('.');
      if (dot != kNPOS) base.Remove(dot);
      outputName = Form("%s_grid.pdf", base.Data());
   }

   c1->SaveAs(outputName.Data());
   TString pngName = outputName;
   if (pngName.EndsWith(".pdf"))
      pngName.Replace(pngName.Length() - 4, 4, ".png");
   else
      pngName += ".png";
   c1->SaveAs(pngName.Data());
   f->Close();
}

void plot_configs_grid_norebin(const char *rootFile, const char *outName = "")
{
   plot_configs_grid(rootFile, outName, kFALSE);
}

