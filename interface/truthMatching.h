#ifndef TRUTHMATCHING_H_
#define TRUTHMATCHING_H_

#include "TROOT.h"
#include "TFile.h"
#include "TSystem.h"

#include <string>
#include <map>
#include <vector>

#include "Analysis/cheetah/interface/tools.h"
#include "Analysis/cheetah/interface/configuration.h"
#include "Analysis/cheetah/interface/physicsObjects.h"

class truthMatching {
  public:

    // Default
    truthMatching(configuration &cmaConfig);

    // Default - so we can clean up;
    virtual ~truthMatching();
    void initialize();
    void setTruthPartons(const std::vector<Parton> truth_partons);
    void setTruthTops(const std::vector<TruthTop> truth_tops);

    void matchJetToTruthTop(Jet& jet);
    void matchJetToTruthJet(Jet& jet, const std::vector<Jet>& truth_jets);
    void parton_match(const Parton& p, Jet& r, double dR=-1.0);

  protected:

    configuration *m_config;

    std::vector<TruthTop> m_truth_tops;
    std::vector<Parton> m_truth_partons;
};

#endif

