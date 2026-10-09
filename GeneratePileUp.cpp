/*
 * GeneratePileUp.cpp -- pileup profile of a simulation campaign, from the CMSSW mixing
 *                       configuration to a ROOT histogram
 *
 * The input files are mix_<year>_25ns_<campaign>_PoissonOOTPU_cfi.py fragments, copies
 * of the files of the same name in SimGeneral/MixingModule/python of CMSSW. Each one
 * holds the pileup distribution a campaign was simulated with: the list
 * mix.input.nbPileupEvents.probValue gives the probability of 0, 1, 2, ... pileup
 * interactions. The program reads this list and writes it as a histogram.
 *
 * Build
 * -----
 *     g++ -o GeneratePileUp GeneratePileUp.cpp $(root-config --cflags --libs)
 *
 * It needs ROOT (root-config in the PATH, e.g. after cmsenv) and ArgumentParser.h next
 * to this file.
 *
 * Usage
 * -----
 *     ./GeneratePileUp -f <cfi 1> [<cfi 2> ...] [--keyword <text>]
 *     ./GeneratePileUp -h
 *
 *     -f          input files; all the arguments up to the next one starting with "-"
 *     --keyword   text that marks the line where the list starts
 *                 (default: mix.input.nbPileupEvents.probValue)
 *
 * Output
 * ------
 * For each input <name>.py, the file <name>-<N>bins.root is written next to the input,
 * N being the number of values read. It contains one TH1F called "pileup" with N bins
 * from 0 to N: the bin [k, k+1) holds the probability of k pileup interactions.
 * N is 100 for the Run 3 Summer24 files and 99 for the 2017 and 2018 UltraLegacy ones.
 *
 * The 2024 and 2018 outputs are the simulation pileup profiles expected in
 * HSCPAnalysis/Ntuplizer/data/pileup/ (see inputFiles in submitCrabJobsMC.py and
 * submitCrabJobsSignals.py).
 *
 * Things to know
 * --------------
 * - The reading is done line by line: it starts at the first line that contains the
 *   keyword, takes every comma-separated number of the lines that follow, and stops at
 *   the first line that contains ")". The numbers must start on the line after the
 *   keyword, as in the CMSSW files: with numbers on the keyword line itself the
 *   program aborts (std::invalid_argument from stof).
 * - The values are written as they are read, in single precision, without any
 *   normalisation (the CMSSW lists sum to 1).
 * - The name of the output depends on the input file and on N only, and an existing
 *   file is overwritten: running again on the same file with another keyword that
 *   gives the same N replaces the previous output.
 * - If the keyword is not found, ROOT prints "nbins is <=0 - set to nbins = 1" and a
 *   file <name>-0bins.root with an empty one-bin histogram is still written.
 * - A file that cannot be opened is reported and skipped; the others are processed.
 * - Without -f, or with -h, the program prints the usage and exits with code 0. The
 *   name printed in the usage and in the banner is the one of the source file
 *   (__FILE__), not the one of the executable.
 */
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "TFile.h"
#include "TBenchmark.h"
#include "TVector3.h"
#include "TH1F.h"

#include "ArgumentParser.h"

// Name of this source file, used in the usage line, the banner and the timing.
#define __FILENAME__ (__FILE__)


using namespace std;

// Reads the list that follows key_word in infile and writes it as a histogram.
void GeneratePileup(std::string infile, std::string key_word);

// Splits s at each delim and appends the pieces to v (as strings or as floats).
void split(std::string s, char delim, std::vector<std::string>& v);

void split(std::string s, char delim, std::vector<float>& v);

// Reads the options, then processes the input files one after the other.
int main(int argc, char* argv[]) {

    std::string usage = "Usage: "+(string)__FILENAME__+" -f file1 file2 fileN --keyword key_word\n";

    // Input files (-f) and keyword that marks the start of the list (--keyword).
    vector<std::string> input;
    std::string key_word = "mix.input.nbPileupEvents.probValue";

    ArgumentParser parser(argc,argv);

    // -h, or no -f: print the usage and stop.
    if( parser.findOption("-h") ){
        cout << usage << endl;
        return 0;
    }
    if( parser.findOption("-f") ) parser.getArgument("-f", input);
    else {
        cout << usage << endl;
        return 0;
    }
    if( parser.findOption("--keyword") ) parser.getArgument("--keyword", key_word);

    cout << "======================" << endl;
    cout << " "<<__FILENAME__<<" " << endl;
    cout << "======================\n" << endl;

    // Timing of the whole job, printed at the end.
    TBenchmark clock;
    clock.Start(__FILENAME__);
 
    // One output file per input file.
    for(auto const &file : input){
        GeneratePileup(file, key_word);
    }
    cout << "" << endl;
    clock.Show(__FILENAME__);
    cout << "" << endl;

    return 0;
}


// One input file: probabilities read after key_word, written to
// <infile without .py>-<N>bins.root as the TH1F "pileup".
void GeneratePileup(std::string infile, std::string key_word){
	ifstream ifs(infile, ios_base::in);

    if (!ifs)  {
        cerr << "Can't open file '" << infile << "'\n";
        return;
    }
    cout << "Reading file " << infile << endl;

    // Values read, in the order of the file: pu[k] is the probability of k interactions.
    vector<float> pu;

    // From the line that contains key_word on, take the numbers of every line that
    // has a comma; stop at the first line that contains ")".
    bool found(false);
    for(std::string line; getline(ifs, line); ) {
        if (line.find(key_word) != std::string::npos) {
            found=true;
        }
        if (found && line.find(",") != std::string::npos) {
            split(line,',',pu);
        }
        if (found && line.find(")") != std::string::npos) break;
    }
    // Number of values read = number of bins.
    Int_t Nbins = pu.size();

    // Output name: ".py" replaced by "-<N>bins.root", next to the input file.
    TString rootfile = infile, suffix = "-"+to_string(Nbins)+"bins.root";
    rootfile = rootfile.ReplaceAll(".py",suffix);

    TFile* fo = new TFile(rootfile,"recreate");

    // Bin k+1 covers [k, k+1) and holds pu[k].
    TH1F* h = new TH1F("pileup","pileup",Nbins,0, float(Nbins));
    for(int ibin=1; ibin<Nbins+1; ibin++){
        h->SetBinContent(ibin,pu[ibin-1]);
    }

    
    h->Write();
    fo->Close();
}


// Splits s at each delim and appends the pieces to v, converted to float with stof
// (leading blanks are ignored; an empty piece after a trailing delim is dropped).
void split(std::string s, char delim, std::vector<float>& v) {
  std::string t;
  for (uint i = 0; i < s.length(); i++) {
    if (s[i] == delim) {
      v.push_back(stof(t));
      t = "";
    } else {
      t += s[i];
    }
  }
  if (t != "")
    v.push_back(stof(t));  //include last word
}

// Same for strings. Not used by this program.
void split(std::string s, char delim, std::vector<std::string>& v) {
  std::string t;
  for (uint i = 0; i < s.length(); i++) {
    if (s[i] == delim) {
      v.push_back(t);
      t = "";
    } else {
      t += s[i];
    }
  }
  if (t != "")
    v.push_back(t);  //include last word
}