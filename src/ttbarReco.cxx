/*
Created:        21 March 2018
Last Updated:   16 May   2018

Dan Marley
daniel.edison.marley@cernSPAMNOT.ch
Texas A&M University
-----

Tool for building ttbar system
 - all-had:  Two top-tagged AK8
 - l+jets:   1 top-tagged AK8 + 1 leptonic top (AK4+lep+nu)
 - dilepton: 2 leptonic tops (AK4+lep+nu)
*/
#include "Analysis/CyMiniAna/interface/ttbarReco.h"


ttbarReco::ttbarReco( configuration& cmaConfig ) :
  m_config(&cmaConfig){
    m_ttbar1L = {};
  }

ttbarReco::~ttbarReco() {}



// single lepton
void ttbarReco::execute(std::vector<Lepton>& leptons, std::vector<Neutrino>& nu, std::vector<Jet>& jets, std::vector<Ljet>& ljets){
    /* Build top quarks system 
       - lepton
       - AK4 near lepton (2D cut)
         > highest pT
         > Ref: https://github.com/UHH2/UHH2/blob/master/common/src/Utils.cxx#L34
       - AK8 away from lepton
         > Most 'top-like' = highest BEST_t score
    */
    m_ttbar1L = {};

    // Setup lepton (only 1 in the single lepton analysis)
    cma::DEBUG("TTBARRECO : building ttbar with "+std::to_string(leptons.size())+" leptons");
    Lepton lep;
    if (leptons.size()>0)
        lep = leptons.at(0);
    else{
        lep.p4.SetPtEtaPhiE(0,0,0,0);   // this event will fail the selection anway, use dummy value
        lep.isGood = false;
    }

    // Get Jets (only if lepton exists)
    int ak4candidate(-1);   // index in jets that corresponds to AK4 from leptonic top
    int ak8candidate(-1);   // index in ljets that corresponds to AK8 (hadronic top)

    if (leptons.size()>0){
        // -- Setup AK4 jet : 2D Cut
        cma::DEBUG("TTBARRECO : building ttbar with "+std::to_string(jets.size())+" ak4 candidates");
        float ak4_pt(0);

        for (auto& jet : jets){
            float jpt   = jet.p4.Pt();
            float dr    = jet.p4.DeltaR(lep.p4);              // DeltaR( lepton,AK4 )
            float ptrel = cma::ptrel( lep.p4,jet.p4 );        // pTrel(  lepton,AK4 )

            // Two possible scenarios -- only consider jets with DeltaR<PI/2:
            // > jet within (0.4<DeltaR<PI/2)
            // > jet closer than 0.4 but ptrel>25
            // Choose highest pT option

            if (dr<M_HALF_PI) { 
                if (dr<0.4 && ptrel>25 && jpt>ak4_pt){
                    ak4_pt = jpt;
                    ak4candidate = jet.index;
                }
                else if (dr>0.4 && jpt>ak4_pt){
                    ak4_pt = jpt;
                    ak4candidate = jet.index;
                }
            } // end possible AK4 candidate
        } // end loop over ak4 


        // -- Setup AK8 jet (highest BEST_t jet farther away than pi/2 from lepton)
        cma::DEBUG("TTBARRECO : building ttbar with "+std::to_string(ljets.size())+" ak8 candidates");
        float BEST_t(-999.);            // between 0 and 1 for real jets

        for (const auto& ljet : ljets){
            float dr = ljet.p4.DeltaR(lep.p4);
            if (dr>M_HALF_PI && ljet.BEST_t>BEST_t){
                BEST_t = ljet.BEST_t;
                ak8candidate = ljet.index;
            }
        } // end loop over ak8 candidates
    } // end if lepton has sufficient pT


    // Define objects in the struct
    m_ttbar1L.neutrino = nu.at(0);
    m_ttbar1L.lepton   = lep;

    Jet dummy_jet;
    dummy_jet.isGood = false;
    m_ttbar1L.jet = (ak4candidate>=0) ? jets.at(ak4candidate) : dummy_jet;

    TLorentzVector leptop;
    leptop = nu.at(0).p4 + lep.p4 + m_ttbar1L.jet.p4;
    m_ttbar1L.leptop = leptop;

    Ljet dummy_ljet;
    dummy_ljet.isGood = false;
    m_ttbar1L.ljet = (ak8candidate>=0) ? ljets.at(ak8candidate) : dummy_ljet;

    m_ttbar1L.dy = lep.charge * ( std::abs(leptop.Rapidity()) - std::abs(m_ttbar1L.ljet.p4.Rapidity()) );

    cma::DEBUG("TTBARRECO : Ttbar built ");

    return;
}

// THE END //
