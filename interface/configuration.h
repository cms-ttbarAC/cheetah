#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "TROOT.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

#include <iostream>
#include <sstream>

#include "Analysis/cheetah/interface/tools.h"


class configuration {
  public:
    // Default - so root can load based on a name;
    configuration( const std::string &configFile );
    //configuration( const configuration& );
    configuration& operator=( const configuration& rhs );

    // Default - so we can clean up;
    virtual ~configuration();

    // Run once at the start of the job;
    virtual void initialize();
    std::string getConfigOption( std::string item );

    // Print configuration
    virtual void print();

    // Type of File(s) being processed
    virtual bool isMC() {return m_isMC;}              // must call "checkFileType(file)" or "isMC(file)" first!
    virtual bool isMC( TFile& file );

    // Type of analysis (all-hadronic, semi-leptonic, or di-leptonic)
    virtual bool isOneLeptonAnalysis() {return m_isOneLeptonAnalysis;}

    // object declarations
    virtual bool useJets() {return m_useJets;}
    virtual bool useNeutrinos() {return m_useNeutrinos;}
    virtual bool useLeptons() {return m_useLeptons;}
    virtual bool useLargeRJets() {return m_useLargeRJets;}
    virtual bool useTruth() {return m_useTruth;}
    bool kinematicReco() {return m_kinematicReco;}
    bool neutrinoReco() {return m_neutrinoReco;}

    std::string jet_btagWkpt() {return m_jet_btag_wkpt;}
    std::vector<std::string> btagWkpts() {return m_btag_WPs;}
    float cMVAv2L() {return m_cMVAv2L;}
    float cMVAv2M() {return m_cMVAv2M;}
    float cMVAv2T() {return m_cMVAv2T;}
    float CSVv2L() {return m_CSVv2L;}
    float CSVv2M() {return m_CSVv2M;}
    float CSVv2T() {return m_CSVv2T;}

    std::vector<std::string> zeroLeptonTriggers() {return m_zeroLeptonTriggers;}
    std::vector<std::string> ejetsTriggers() {return m_ejetsTriggers;}
    std::vector<std::string> mujetsTriggers() {return m_mujetsTriggers;}
    std::vector<std::string> dileptonTriggers() {return m_dileptonTriggers;}

    // functions about the TTree
    virtual bool isNominalTree();
    virtual bool isNominalTree( const std::string &tree_name );
    std::vector<std::string> treeNames() {return m_treeNames;}
    void setTreename(std::string treeName);
    std::string treename() {return m_treename;}

    // functions about the file
    void readMetadata(TFile& file, const std::string& metadataTreeName);
    virtual void inspectFile( TFile& file, const std::string& metadataTreeName="" );
    std::vector<std::string> filesToProcess() {return m_filesToProcess;}
    void setFilename(std::string fileName);
    std::string filename(){ return m_filename;}
    std::string primaryDataset(){ return m_primaryDataset;}

    // return some values from config file
    std::string verboseLevel() {return m_verboseLevel;}
    std::vector<std::string> selections() {return m_selections;}
    std::vector<std::string> cutsfiles() {return m_cutsfiles;}
    std::string outputFilePath() {return m_outputFilePath;}
    std::string customDirectory() {return m_customDirectory;}
    std::string configFileName() {return m_configFile;}
    std::string getAbsolutePath() {return m_cma_absPath;}
    int nEventsToProcess() {return m_nEventsToProcess;}
    unsigned long long firstEvent() {return m_firstEvent;}

    // DNN
    std::string dnnFile() {return m_dnnFile;}
    bool useDNN() {return m_useDNN;}
    bool DNNinference() {return m_DNNinference;}
    bool DNNtraining() {return m_DNNtraining;}
    std::string dnnKey() {return m_dnnKey;}   // key for lwtnn

    // truth-reco matching
    std::map<std::string,int> mapOfPartonContainment() {return m_containmentMap;}

  protected:

    void check_btag_WP(const std::string &wkpt);

    std::map<std::string,std::string> m_map_config;
    const std::string m_configFile;

    // type of file(s)
    bool m_isMC;

    // type of analysis
    bool m_isOneLeptonAnalysis;

    // object declarations
    bool m_useTruth;
    bool m_useJets;
    bool m_useLeptons;
    bool m_useLargeRJets;
    bool m_useNeutrinos;
    bool m_neutrinoReco;

    // return some values from config file
    std::string m_input_selection;
    std::vector<std::string> m_selections;
    std::vector<std::string> m_cutsfiles;
    std::string m_treename;
    std::string m_filename;
    std::string m_primaryDataset;

    std::string m_verboseLevel;
    int m_nEventsToProcess;
    unsigned long long m_firstEvent;
    std::string m_outputFilePath;
    std::string m_customDirectory;
    bool m_makeTTree;
    bool m_makeHistograms;
    bool m_makeEfficiencies;
    std::string m_cma_absPath;
    std::string m_metadataFile;
    bool m_useDNN;
    bool m_DNNinference;
    bool m_DNNtraining;
    std::string m_dnnFile;
    std::string m_dnnKey;
    bool m_doRecoEventLoop;
    bool m_matchTruthToReco;
    bool m_kinematicReco;

    std::string m_jet_btag_wkpt;   // "L","M","T"

    // b-tagging (https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation80XReReco)
    // isBTagged = (jet.cMVAv2 > wkpt)
    std::vector<std::string> m_btag_WPs = {"L","M","T"};
    float m_cMVAv2L=-0.5884;
    float m_cMVAv2M=0.4432;
    float m_cMVAv2T=0.9432;
    float m_CSVv2L=0.5426;
    float m_CSVv2M=0.8484;
    float m_CSVv2T=0.9535;

    std::vector<std::string> m_filters = {"goodVertices",
        "eeBadScFilter",
        "HBHENoiseFilter",
        "HBHENoiseIsoFilter",
        "globalTightHalo2016Filter",
        "EcalDeadCellTriggerPrimitiveFilter"};

    std::vector<std::string> m_zeroLeptonTriggers = {"HLT_PFHT800","HLT_PFHT900","HLT_AK8PFJet450","HLT_PFHT700TrimMass50","HLT_PFJet360TrimMass30"};
    std::vector<std::string> m_ejetsTriggers  = {"HLT_Ele45_CaloIdVT_GsfTrkIdT_PFJet200_PFJet50","HLT_Ele50_CaloIdVT_GsfTrkIdT_PFJet165","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
    std::vector<std::string> m_mujetsTriggers = {/*"HLT_Mu40_Eta2P1_PFJet200_PFJet50",*/"HLT_Mu50","HLT_TkMu50"};
    std::vector<std::string> m_dileptonTriggers = {};

    bool m_recalculateMetadata;
    std::vector<std::string> m_filesToProcess;
    std::vector<std::string> m_treeNames;

    // Primary dataset names for different samples in analysis
    std::map<std::string,std::string> m_mapOfPrimaryDatasets = {
        {"ttbar","TT_TuneCUETP8M2T4_13TeV-powheg-pythia8"},
        {"ttbar-ext","TT_TuneCUETP8M2T4_13TeV-powheg-pythia8-ext"},
        {"ttbarGOOD","TT_TuneCUETP8M1_13TeV-powheg-pythia8"},
        {"ttbarGEN","TT_TuneCUETP8M1_13TeV-powheg-pythia8"},
    };

    // Degrees of 'containment' for parton matching to jets
    std::map<std::string,int> m_containmentMap = {
                    {"NONE",  0},
                    {"BONLY", 1},
                    {"QONLY", 2},
                    {"BQ",    3},   // B+Q
                    {"W",     4},   // Q+Q
                    {"FULL",  5}};  // (BQ)+Q || (W+B) || Q+Q+B


    std::map<std::string,std::string> m_defaultConfigs = {
             {"jet_btag_wkpt",         "M"},
             {"NEvents",               "-1"},
             {"firstEvent",            "0"},
             {"selection",             "example"},
             {"output_path",           "./"},
             {"customDirectory",       ""},
             {"cutsfile",              "examples/config/cuts_example.txt"},
             {"inputfile",             "examples/config/miniSL_ALLfiles.txt"},
             {"treenames",             "examples/config/treenames_nominal"},
             {"treename",              "tree/eventVars"},
             {"metadataFile",          "config/sampleMetaData.txt"},
             {"verboseLevel",          "INFO"},
             {"dnnFile",               "config/keras_ttbar_DNN.json"},
             {"dnnKey",                "dnn"},
             {"DNNinference",          "false"},
             {"DNNtraining",           "false"},
             {"kinematicReco",         "false"} };
};

#endif
