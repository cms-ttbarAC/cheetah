/*
Created:        --
Last Updated:   29 August    2017

Dan Marley
daniel.edison.marley@cernSPAMNOT.ch
Texas A&M University
-----

Make histograms for systematic uncertainties (& nominal) 
to go into plots || TRexFitter
*/
#include "Analysis/cheetah/interface/histogrammer.h"


histogrammer::histogrammer( configuration& cmaConfig, std::string name ) :
  histogrammerBase::histogrammerBase(cmaConfig,name),
  m_config(&cmaConfig),
  m_name(name){}

histogrammer::~histogrammer() {}


/**** INITIALIZE HISTOGRAMS ****/
void histogrammer::initialize( TFile& outputFile ){
    /* Setup some values and book histograms */
    outputFile.cd();

    bookHists();

    return;
}

void histogrammer::bookHists(){
    /* 
      Book histograms -- modify/inherit this function for analysis-specific hists 

      @param name   This is the string used to identify histograms for different systematics/event weights

      0 :: Top      (lepton Q > 0)
      1 :: Anti-top (lepton Q < 0)
    */
    cma::DEBUG("HISTOGRAMMER : Init. histograms: "+m_name);

    for (const auto& target : m_targets){
        // Non-features just to compare consistency between top/anti-top
        histogrammerBase::init_hist("ljet_BEST_t-"+target+"_"+m_name,  100,  0.0,    1.0);
        histogrammerBase::init_hist("ljet_BEST_w-"+target+"_"+m_name,  100,  0.0,    1.0);
        histogrammerBase::init_hist("ljet_BEST_z-"+target+"_"+m_name,  100,  0.0,    1.0);
        histogrammerBase::init_hist("ljet_BEST_h-"+target+"_"+m_name,  100,  0.0,    1.0);
        histogrammerBase::init_hist("ljet_BEST_j-"+target+"_"+m_name,  100,  0.0,    1.0);
        histogrammerBase::init_hist("ljet_SDmass-"+target+"_"+m_name,  500,  0.0,  500.0);
        histogrammerBase::init_hist("ljet_tau1-"+target+"_"+m_name,    200,  0.0,    2.0);
        histogrammerBase::init_hist("ljet_tau2-"+target+"_"+m_name,    200,  0.0,    2.0);
        histogrammerBase::init_hist("ljet_tau3-"+target+"_"+m_name,    200,  0.0,    2.0);
        histogrammerBase::init_hist("ljet_tau21-"+target+"_"+m_name,   100,  0.0,    1.0);
        histogrammerBase::init_hist("ljet_tau32-"+target+"_"+m_name,   100,  0.0,    1.0);

        // Features
        histogrammerBase::init_hist("ljet_charge-"+target+"_"+m_name,  100,-5, 5);
        histogrammerBase::init_hist("ljet_subjet0_bdisc-"+target+"_"+m_name, 100, 0.0, 1.0);
        histogrammerBase::init_hist("ljet_subjet0_mass-"+target+"_"+m_name,  200, 0.0, 200.0);
        histogrammerBase::init_hist("ljet_subjet0_ptrel-"+target+"_"+m_name, 100, 0.0, 1.0);
        histogrammerBase::init_hist("ljet_subjet0_charge-"+target+"_"+m_name,100,  -5,   5);
        histogrammerBase::init_hist("ljet_subjet1_bdisc-"+target+"_"+m_name, 100, 0.0, 1.0);
        histogrammerBase::init_hist("ljet_subjet1_mass-"+target+"_"+m_name,  200, 0.0, 200.0);
        histogrammerBase::init_hist("ljet_subjet1_ptrel-"+target+"_"+m_name, 100, 0.0, 1.0);
        histogrammerBase::init_hist("ljet_subjet1_charge-"+target+"_"+m_name,100,  -5,   5);
    }

    return;
}


/**** FILL HISTOGRAMS ****/
void histogrammer::fill( const std::map<std::string,double> features, double weight ){
    /* Fill histograms -- 
       Fill information from single top object (inputs to deep learning)
    */
    std::string target = std::to_string( int(features.at("target")) );

    cma::DEBUG("HISTOGRAMMER : Fill histograms: "+m_name+"; target = "+target);

    histogrammerBase::fill("ljet_BEST_t-"+target+"_"+m_name, features.at("ljet_BEST_t"), weight);
    histogrammerBase::fill("ljet_BEST_w-"+target+"_"+m_name, features.at("ljet_BEST_w"), weight);
    histogrammerBase::fill("ljet_BEST_z-"+target+"_"+m_name, features.at("ljet_BEST_z"), weight);
    histogrammerBase::fill("ljet_BEST_h-"+target+"_"+m_name, features.at("ljet_BEST_h"), weight);
    histogrammerBase::fill("ljet_BEST_j-"+target+"_"+m_name, features.at("ljet_BEST_j"), weight);
    histogrammerBase::fill("ljet_SDmass-"+target+"_"+m_name, features.at("ljet_SDmass"), weight);
    histogrammerBase::fill("ljet_tau1-"+target+"_"+m_name,   features.at("ljet_tau1"),  weight);
    histogrammerBase::fill("ljet_tau2-"+target+"_"+m_name,   features.at("ljet_tau2"),  weight);
    histogrammerBase::fill("ljet_tau3-"+target+"_"+m_name,   features.at("ljet_tau3"),  weight);
    histogrammerBase::fill("ljet_tau21-"+target+"_"+m_name,  features.at("ljet_tau21"), weight);
    histogrammerBase::fill("ljet_tau32-"+target+"_"+m_name,  features.at("ljet_tau32"), weight);

    histogrammerBase::fill("ljet_charge-"+target+"_"+m_name, features.at("ljet_charge"), weight);
    histogrammerBase::fill("ljet_subjet0_bdisc-"+target+"_"+m_name,  features.at("ljet_subjet0_bdisc"),  weight);
    histogrammerBase::fill("ljet_subjet0_mass-"+target+"_"+m_name,   features.at("ljet_subjet0_mass"),   weight);
    histogrammerBase::fill("ljet_subjet0_ptrel-"+target+"_"+m_name,  features.at("ljet_subjet0_ptrel"),  weight);
    histogrammerBase::fill("ljet_subjet0_charge-"+target+"_"+m_name, features.at("ljet_subjet0_charge"), weight);
    histogrammerBase::fill("ljet_subjet1_bdisc-"+target+"_"+m_name,  features.at("ljet_subjet1_bdisc"),  weight);
    histogrammerBase::fill("ljet_subjet1_mass-"+target+"_"+m_name,   features.at("ljet_subjet1_mass"),   weight);
    histogrammerBase::fill("ljet_subjet1_ptrel-"+target+"_"+m_name,  features.at("ljet_subjet1_ptrel"),  weight);
    histogrammerBase::fill("ljet_subjet1_charge-"+target+"_"+m_name, features.at("ljet_subjet1_charge"), weight);

    cma::DEBUG("HISTOGRAMMER : End histograms");

    return;
}

// THE END

