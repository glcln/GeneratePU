# GeneratePU

Converts the pileup distribution of a CMS simulation campaign, as written in the CMSSW mixing configuration, into a ROOT histogram.

The outputs are the simulation pileup profiles shipped with the CRAB jobs of the HSCP ntuplizer (`HSCPAnalysis/Ntuplizer/data/pileup/`), next to the data pileup histograms.


## Disclaimer

I wrote all the code in this project myself. However, this README and the comments in the scripts were generated using Claude.


## Content

| File | Role |
|---|---|
| `mix_<year>_25ns_<campaign>_PoissonOOTPU_cfi.py` | Pileup distribution of a campaign, copied from `SimGeneral/MixingModule/python` in CMSSW: `probValue` lists the probability of 0, 1, 2, ... pileup interactions |
| `GeneratePileUp.cpp` | Reads the `probValue` list of each input file and writes it as a histogram |
| `ArgumentParser.h` | Command-line reader used by `GeneratePileUp.cpp` |
| `GeneratePileUp` | Compiled executable; rebuild it after changing the source |

Campaigns available: 2017 and 2018 UltraLegacy, 2022, 2023 and 2024 Summer24.

## Build

Needs ROOT (`root-config` in the `PATH`, e.g. after `cmsenv`).

```
g++ -o GeneratePileUp GeneratePileUp.cpp $(root-config --cflags --libs)
```

## Usage

```
./GeneratePileUp -f <cfi 1> [<cfi 2> ...]
```

For example, for all the campaigns at once:

```
./GeneratePileUp -f mix_*_cfi.py
```

## Output

For each input `<name>.py`, the file `<name>-<N>bins.root` is written next to it, `N` being the number of values in the list: 100 for the Summer24 campaigns, 99 for 2017 and 2018.

It contains one `TH1F` called `pileup`, with `N` bins from 0 to `N`. The bin `[k, k+1)` holds the probability of `k` pileup interactions.

```
mix_2024_25ns_RunIII2024Summer24_PoissonOOTPU_cfi.py  ->  mix_2024_25ns_RunIII2024Summer24_PoissonOOTPU_cfi-100bins.root
mix_2018_25ns_UltraLegacy_PoissonOOTPU_cfi.py         ->  mix_2018_25ns_UltraLegacy_PoissonOOTPU_cfi-99bins.root
```

## Adding a campaign

Copy its `mix_..._cfi.py` from `SimGeneral/MixingModule/python` of CMSSW into this directory, run `GeneratePileUp` on it, and copy the `.root` file to `HSCPAnalysis/Ntuplizer/data/pileup/`.

## Notes

- An existing output file is overwritten.
- If the list is not found in an input file, a file `<name>-0bins.root` with an empty histogram is still written: check the number of bins in the name.
- Other options and details are in the header of `GeneratePileUp.cpp`.