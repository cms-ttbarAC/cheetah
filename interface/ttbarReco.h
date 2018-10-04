#ifndef TTBARRECO_H
#define TTBARRECO_H

#include <string>
#include <map>
#include <vector>

#include "Analysis/cheetah/interface/physicsObjects.h"
#include "Analysis/cheetah/interface/tools.h"
#include "Analysis/cheetah/interface/configuration.h"

class ttbarReco {
  public:
    ttbarReco( configuration& cmaConfig );

    ~ttbarReco();

    Ttbar1L ttbar1L() {return m_ttbar1L;}

    // single lepton
    void execute(std::vector<Lepton>& leptons, std::vector<Neutrino>& nu, std::vector<Jet>& jets, std::vector<Ljet>& ljets);

  protected:

    configuration *m_config;

    Ttbar1L m_ttbar1L;
};

#endif
