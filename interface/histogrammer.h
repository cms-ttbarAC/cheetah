#ifndef HISTOGRAMMER_H_
#define HISTOGRAMMER_H_

#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TSystem.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

#include <string>
#include <map>
#include <vector>

#include "Analysis/cheetah/interface/histogrammerBase.h"
#include "Analysis/cheetah/interface/configuration.h"
#include "Analysis/cheetah/interface/tools.h"
#include "Analysis/cheetah/interface/Event.h"

class histogrammer : public histogrammerBase {
  public:

    // Default - so root can load based on a name;
    histogrammer( configuration& cmaConfig, std::string name="" );

    // Default - so we can clean up;
    virtual ~histogrammer();

    /* fill histograms */
    void fill( const std::map<std::string,double> top, double weight=1.0 );

    /* Book histograms */
    void initialize( TFile& outputFile );
    void bookHists();

  protected:

    configuration *m_config;
    std::string m_name;

    // Target values for system
    std::vector<std::string> m_targets = {"0","1"};
};

#endif

