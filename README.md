# Factorial Moments Analysis for ALICE

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
        ├── copyFiles.sh               # File copying utility
        ├── getFqbinDAT.C              # Factorial moments extraction and sampling error calculation implementation
        ├── makeFqStdDAT.C             # Final factorial moments calculation
        └── merge_batches.sh           # File merging utility
        └── mergeLocalOutputs.sh       # File merging utility
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

#### **Track Selection Criteria (n different conditions):**
- can implement n different cut conditions, the number should be put in header file, and definitions in ```AliAnalysisTaskFM_marooz::FillTrackInfo()

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

2. **Compile the task:**
   ```bash
   root -l -b -q 'AliAnalysisTaskFM_marooz.cxx++g'
   ```

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
2. **Data Extraction**: Use `utils/getFq` to extract factorial moments
3. **Calculation**: Use `utils/makeFq` to calculate and plot results
4. **Merging**: Use `utils/merge` to combine multiple runs


## Contributors

- **Salman** - Main implementation and analysis framework
- **Date**: 04-May-2025

## References

- ALICE Collaboration analysis notes
- Factorial moments methodology papers
- ALICE software documentation 