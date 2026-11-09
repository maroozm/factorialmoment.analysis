# Factorial Moments Analysis for ALICE (Run 2)

> **Note:** This repository is for **ALICE Run 2** (LHC10h, LHC15o, LHC18q/r). For **ALICE Run 3** analysis, see: https://github.com/maroozm/o2-factorial-moments

This repository contains the implementation of factorial moments analysis for charged particles in two dimensions (eta and phi) for ALICE data and MC simulations.

## Overview

The analysis implements intermittency analysis using factorial moments to study particle multiplicity fluctuations in heavy-ion collisions. The code supports both data and Monte Carlo (MC) analysis with various track selection criteria and quality assurance (QA) procedures.

## Project Structure

```
└── main/
    ├── AliAnalysisTaskFM_marooz.cxx    # Main analysis task implementation
    ├── AliAnalysisTaskFM_marooz.h      # Header file with class definitions
    ├── AddTaskFM_marooz.C              # Task configuration and setup
    ├── runCode.C                       # Main execution script
    ├── runLists.h                      # Run list definitions for different 
    └── utils/                          # Utility scripts for offline processing
        ├── ScanOutput.h                # Shared auto-detection helper for output lists/ntuples
        ├── getFqbinDAT.C               # Factorial moments extraction (auto-detects real/data/gen/mixed)
        ├── makeFqStdDAT.C              # Final factorial moments calculation (auto-detects dirs/bins)
        ├── plot_configs_grid.C         # QA grid of multiplicity/eta/phi and track-quality plots
        ├── corkO.txt                   # Colour palette used by plot_configs_grid.C
        ├── copyGRIDFILES.sh            # File copying utility
        ├── merge_batches.sh            # File merging after copying
        └── mergeLocalOutputs.sh        # File merging utility
```

## Key Components

### 1. AliAnalysisTaskFM_marooz

The main analysis task class that implements factorial moments calculation. Key features:

- **Track Selection**: Multiple filter bit combinations (128, 768) with various quality cuts
- **Quality Cuts**: DCA, TPC clusters, chi2, shared clusters, etc.
- **Particle Identification**: PID response integration for particle type analysis
- **Event Selection**: Centrality, vertex, pileup rejection
- **MC Support**: Full Monte Carlo truth analysis capabilities
- **Output**: Histograms, TNtuples with factorial moments (Fq2-Fq7)

### 2. AddTaskFM_marooz.C

Configuration script that sets up the analysis task with various parameters:

#### **Track Selection Criteria (5 conditions):**
The analysis implements 5 track selection conditions:

1. **fb128** (baseline): fb128 filter bit + |DCAxy| < 2.4 cm + |DCAz| < 3.2 cm + TPC clusters > 50 + TPC chi2/cluster <= 4.0
2. **fb128crows**: fb128 + minimum TPC crossed rows (configurable via `SetCrowsMinCut()`, default 80)
3. **fb128shcls**: fb128 + maximum shared cluster fraction nSharedCls/nCls (configurable via `SetSharedClsFractionCut()`, default 0.4)
4. **fb128crowsoverfindable**: fb128 + minimum ratio of crossed rows/findable clusters (configurable via `SetCrowsOverFindableCut()`, default 0.8)
5. **fb768**: fb768 filter bit (hybrid TPC+ITS tracks)

The number of conditions is set in the header file (`_numEnvs = 5`) and the condition logic is defined in `AliAnalysisTaskFM_marooz::FillTrackInfo()`.

#### **Configuration Parameters (AddTaskFM_marooz.C):**
New configurable track selection cuts:
```cpp
// Track selection cuts for the 5 conditions
Double_t _crows_min = 80.0;              // Minimum TPC crossed rows for fb128crows
Double_t _shcls_max = 0.4;               // Maximum shared cluster fraction for fb128shcls  
Double_t _crows_over_findable_min = 0.8; // Minimum crossed rows/findable clusters for fb128crowsoverfindable
// Magnetic field polarity
Int_t _bField = 0;                       // 0=auto (2015:+1, 2018:-1, 2010:0), 1=positive, -1=negative
```

#### **Configuration Parameters:**
```cpp
// Centrality and acceptance
Int_t _cent_min = 0, _cent_max = 5;           // 0-5% centrality
Double_t _eta_min = -0.8, _eta_max = 0.8;     // Pseudorapidity range

// pT bins
Int_t _n_pt_bins = 1;
Double_t _pt_bins[10] = {0.2, 2.0, ...};      // pT bin edges

// Quality cuts
Int_t _max_m_bins = 82;                       // Phase space binning
Bool_t _pileup_rejection = kTRUE;             // Pileup rejection
```

### 3. runLists.h

Contains run list definitions for different ALICE data taking periods:

```cpp
namespace runLists {
    std::vector<Int_t> GetRunList(Int_t period) {
        if (period == 10) return { /* LHC10h runs */ };
        if (period == 15) return { /* LHC15o runs */ };
        if (period == 18) return { /* LHC18q/r runs */ };
    }
}
```

**Supported Periods:**
- **2010 (LHC10h)**: Pb-Pb collisions at √sNN = 2.76 TeV
- **2015 (LHC15o)**: Pb-Pb collisions at √sNN = 5.02 TeV  
- **2018 (LHC18q/r)**: Pb-Pb collisions at √sNN = 5.02 TeV

### 4. runCode.C

Main execution script for running the analysis on the ALICE Grid:

#### **Usage:**
```cpp
void runCode(TString taskName = "test_ME",
             Int_t _mode = 0,        // 0: test, 1: full, 2: terminate, 3: merge
             Int_t _nruns = 1,       // Number of runs (0 for all)
             Int_t period = 18)      // Data period (10, 15, 18)
```

#### **Execution Modes:**
- **Mode 0 (test)**: Test run with limited files
- **Mode 1 (full)**: Full production run
- **Mode 2 (terminate)**: Terminate grid jobs
- **Mode 3 (merge)**: Merge output files

#### **Example Usage:**
```bash
# Test run for 2018 data
root -l -b -q 'runCode.C("test_analysis", 0, 1, 18)'

# Full production run for 2015 data
root -l -b -q 'runCode.C("production_run", 1, 0, 15)'
```

## Analysis Output

### Histograms
- **Event Level**: Event counters, centrality distributions, vertex QA
- **Track Level**: DCA distributions, cluster distributions, PID information
- **Kinematic**: pT, η, φ distributions for different conditions
- **Quality**: Chi2, shared clusters, findable clusters

### TNtuples
Factorial moments stored in TNtuples with variables:
- `Mult`: Multiplicity per event
- `Mbins`: Number of phase space bins (M²)
- `Av_bincontent`: Average bin content
- `Fq2e` to `Fq7e`: Factorial moments of order 2-7

## Running the Analysis

### Prerequisites
- ALICE software environment (AliPhysics)
- ROOT framework
- Grid certificate for ALICE Grid access

### Setup
1. **Initialize ALICE environment:**
   ```bash
   alienv enter AliPhysics/latest
   ```

2. **Set parameters in AddTask**

3. **Run the analysis:**
   ```bash
   root -l -b -q 'runCode.C("my_analysis", 0, 1, 18)'
   ```

### Grid Submission
The analysis automatically submits jobs to the ALICE Grid with:
- **Data directories**: `/alice/sim/2020/LHC20j6a/` (2015), `/alice/sim/2020/LHC20e3a/` (2018)
- **Output**: Stored in Grid output directory
- **Logs**: Available for debugging

## Data Processing Workflow

1. **Analysis Execution**: Run `runCode.C` to submit grid jobs

2. **Data Extraction**: Use `utils/getFqbinDAT.C` to extract factorial moments offline from a merged `AnalysisResults.root`, along with the sampling error. The macro scans the file and auto-detects the pt bins, conditions and number of M bins (DATA, MC reconstructed, generated and mixed-event output are all handled). Run it from the output directory:
   ```bash
   root -l -q '/path/to/utils/getFqbinDAT.C("AnalysisResults.root")'
   ```
   The full signature is `getFqbinDAT(inputFile, sampleID = 1, doMixed = 0)`. The third argument enables mixed-event extraction (disabled by default); note that mixed-event ntuples are only produced if `AddTaskFM_marooz.C` was run with `_mixed_event = kTRUE` (it defaults to `kFALSE`):
   ```bash
   root -l -q '/path/to/utils/getFqbinDAT.C("AnalysisResults.root", 1, 1)'
   ```
   Mixed-event results are written under `<condition>/bin<N>/datafiles/mixed/` and are picked up automatically by `makeFqStdDAT.C`. The command also creates one `<condition>/bin<N>/datafiles/` directory per condition and pt bin with the `.dat` files and `Fqe_standardErrorSample*.root`.

3. **Merging samples**: `utils/mergeLocalOutputs.sh` — use after step 2 when several input files produced separate `Fqe_standardErrorSample*.root` files. It merges them into a single `Fqe_standardErrorSample.root` per directory.

4. **Calculation**: Use `utils/makeFqStdDAT.C` to calculate the final mean of factorial moments and their errors. It auto-detects the condition directories, pt bins and number of M bins from the current directory, then writes `FqeStdError.csv` (with bin, condition, type, date and source-file metadata):
   ```bash
   root -l -q '/path/to/utils/makeFqStdDAT.C'
   ```

5. **QA plotting**: Use `utils/plot_configs_grid.C` to produce a grid of multiplicity/eta/phi distributions and track-quality QA plots for every configuration contained in a merged ROOT file:
   ```bash
   root -l -q 'utils/plot_configs_grid.C("ROOTMERGE-....root")'
   ```
   An optional second argument sets the output name; `plot_configs_grid_norebin("file.root")` disables the automatic 2D rebinning. A `.pdf` and `.png` are written. The `corkO.txt` palette must stay next to the macro.

6. **Copying**: Use `utils/copyGRIDFILES.sh` in case you are not able to terminate grid jobs (by any reason), put the grid directory path in it, it will copy all the rootfiles in your local directory. Use `utils/merge_batches.sh` after `copyGRIDFILES.sh` to merge the files, so that you can have 10-20 files at the end.


## Contributors

- **Salman** - Main implementation and analysis framework
- **Date**: 04-May-2025

## References

- ALICE Collaboration analysis notes
- Factorial moments methodology papers
- ALICE software documentation 