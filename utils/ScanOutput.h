#ifndef SCAN_OUTPUT_H
#define SCAN_OUTPUT_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <TFile.h>
#include <TList.h>
#include <TNtuple.h>
#include <TKey.h>
#include <TString.h>

struct ScanOutput {
  std::vector<std::string> ptNames;
  std::vector<std::string> condNames;
  std::map<std::pair<int,int>, std::string> listMap;
  std::map<std::pair<int,int>, std::string> condMap;
  int Ntmax;
};

inline Bool_t IsExcludedListName(const TString &keyName, const std::vector<std::string> &excludedPrefixes) {
  for (const auto &prefix : excludedPrefixes) {
    if (keyName.BeginsWith(prefix.c_str())) return kTRUE;
  }
  return kFALSE;
}

inline void ParseNtupleName(const TString &name, std::string &ptName, std::string &condName, Int_t &mVal) {
  ptName = "";
  condName = "";
  mVal = -1;

  Int_t binPos = name.Index("bin");
  if (binPos < 0) return;

  Int_t mPos = name.Index("_M");
  if (mPos < 0) return;

  mVal = TString(name.Data() + mPos + 2).Atoi();
  TString middle(name.Data() + binPos + 3, mPos - binPos - 3);

  Int_t underPos = middle.Index("_");
  if (underPos < 0) {
    ptName = middle.Data();
  } else {
    ptName = std::string(middle.Data(), underPos);
    condName = std::string(middle.Data() + underPos + 1);
  }
}

inline ScanOutput ScanFile(TFile *file, const char *listPrefix, const char *ntuplePrefix,
                           const std::vector<std::string> &excludedListPrefixes) {
  ScanOutput result;
  result.Ntmax = 0;

  TIter nextkey(file->GetListOfKeys());
  TKey *key;

  while ((key = (TKey*)nextkey())) {
    TString keyName = key->GetName();
    if (!keyName.BeginsWith(listPrefix)) continue;
    if (IsExcludedListName(keyName, excludedListPrefixes)) continue;

    TList *list = (TList*)file->Get(keyName.Data());
    if (!list) continue;

    std::set<std::string> ptSet, condSet;
    Int_t localMaxM = 0;

    TIter nextobj(list);
    TObject *obj;
    while ((obj = nextobj())) {
      if (!obj->InheritsFrom("TNtuple")) continue;

      TString ntupleName = obj->GetName();
      if (!ntupleName.BeginsWith(ntuplePrefix)) continue;
      if (ntupleName.Contains("Mixed") && !TString(ntuplePrefix).Contains("Mixed")) continue;
      if (ntupleName.Contains("gen") && !TString(ntuplePrefix).Contains("gen")) continue;

      std::string ptName, condName;
      Int_t mVal;
      ParseNtupleName(ntupleName, ptName, condName, mVal);

      if (!ptName.empty()) {
        ptSet.insert(ptName);
        if (!condName.empty()) condSet.insert(condName);
      }
      if (mVal + 1 > localMaxM) localMaxM = mVal + 1;
    }

    if (localMaxM > result.Ntmax) result.Ntmax = localMaxM;

    for (auto &p : ptSet) {
      bool found = false;
      for (auto &existing : result.ptNames) { if (existing == p) { found = true; break; } }
      if (!found) result.ptNames.push_back(p);
    }
    for (auto &c : condSet) {
      bool found = false;
      for (auto &existing : result.condNames) { if (existing == c) { found = true; break; } }
      if (!found) result.condNames.push_back(c);
    }
  }

  return result;
}

inline ScanOutput ScanFile(TFile *file, const char *listPrefix, const char *ntuplePrefix) {
  return ScanFile(file, listPrefix, ntuplePrefix, std::vector<std::string>());
}

#endif
