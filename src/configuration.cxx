/*
Created:        20 August    2016
Last Updated:   25 March     2018

Dan Marley
daniel.edison.marley@cernSPAMNOT.ch
Texas A&M University
-----

Configuration class
  -- Read config file and use functions
     to return configurations later

*/
#include "Analysis/CyMiniAna/interface/configuration.h"


configuration::configuration(const std::string &configFile) : 
  m_configFile(configFile),
  m_isMC(false),
  m_treename("SetMe"),
  m_filename("SetMe"),
  m_verboseLevel("SetMe"),
  m_nEventsToProcess(0),
  m_firstEvent(0),
  m_outputFilePath("SetMe"),
  m_customDirectory("SetMe"),
  m_cma_absPath("SetMe"),
  m_metadataFile("SetMe"),
  m_DNNinference(false),
  m_DNNtraining(false),
  m_dnnFile("SetMe"),
  m_dnnKey("SetMe"),
  m_jet_btag_wkpt("SetMe"){
    m_selections.clear();
    m_cutsfiles.clear();
    m_primaryDataset = "";
    m_NTotalEvents   = 0;
  }

configuration::~configuration() {}

configuration &configuration::operator=(const configuration &rhs) { return *this; }

void configuration::initialize() {
    /* Initialize the configurations */
    std::vector<std::string> configurations; 
    cma::read_file( m_configFile, configurations ); // read config file into vector

    // fill map with values from configuration file
    for (const auto& config : configurations){
        // split config items by space
        std::istringstream cfg(config);
        std::istream_iterator<std::string> start(cfg), stop;
        std::vector<std::string> tokens(start, stop);

        m_map_config.insert( std::pair<std::string,std::string>(tokens.at(0),tokens.at(1)) );
    }

    // Protection against default settings missing in custom configuration
    // -- map of defaultConfigs defined in header (can't use 'verbose' tools, not defined yet!)
    for (const auto& defaultConfig : m_defaultConfigs){
        if ( m_map_config.find(defaultConfig.first) == m_map_config.end() ){ // item isn't in config file
            std::cout << " WARNING :: CONFIG : Configuration " << defaultConfig.first << 
                         " not defined, set to default '" << defaultConfig.second << "'" << std::endl;
            m_map_config[defaultConfig.first] = defaultConfig.second;
        }
    }


    // Set the verbosity level (the amount of output to the console)
    std::map<std::string,unsigned int> verboseMap = cma::verboseMap(); // load mapping of string to integer
    m_verboseLevel = getConfigOption("verboseLevel");
    if (verboseMap.find(m_verboseLevel)==verboseMap.end()){
        m_verboseLevel = "INFO";
        cma::setVerboseLevel(m_verboseLevel);

        cma::WARNING( "CONFIG : Verbose level selected, "+m_verboseLevel+", is not supported " );
        cma::WARNING( "CONFIG : Please select one of the following: " );
        for (const auto& dm : verboseMap)
            cma::WARNING( "CONFIG :          "+dm.first);
        cma::WARNING( "CONFIG : Continuing; setting verbose level to "+m_verboseLevel);
    }
    else{
        cma::setVerboseLevel(m_verboseLevel);
    }


    // Get the absolute path to CyMiniAna for loading
    char* cma_path = getenv("CYMINIANADIR");
    if (cma_path==NULL){
        cma::WARNING("CONFIG : environment variable " );
        cma::WARNING("CONFIG :    'CYMINIANADIR' " );
        cma::WARNING("CONFIG : is not set.  Using PWD to set path." );
        cma::WARNING("CONFIG : This may cause problems submitting batch jobs." );
        cma_path = getenv("PWD");
    }
    m_cma_absPath = cma_path;
    cma::DEBUG("CONFIG : path set to: "+m_cma_absPath );

    // Assign values
    m_nEventsToProcess = std::stoi(getConfigOption("NEvents"));
    m_firstEvent       = std::stoi(getConfigOption("firstEvent"));
    m_input_selection  = getConfigOption("input_selection"); // "grid", "pre", etc.
    cma::split( m_map_config.at("selection"), ',', m_selections );  // different event selections
    cma::split( m_map_config.at("cutsfile"), ',', m_cutsfiles );  // different event selections

    // check that b-tag and top-tag WPs are recognized as one of supported values
    check_btag_WP(getConfigOption("jet_btag_wkpt"));

    m_jet_btag_wkpt    = getConfigOption("jet_btag_wkpt");
    m_outputFilePath   = getConfigOption("output_path");
    m_customDirectory  = getConfigOption("customDirectory");
    m_dnnFile          = getConfigOption("dnnFile");
    m_dnnKey           = getConfigOption("dnnKey");
    m_DNNinference     = cma::str2bool( getConfigOption("DNNinference") );
    m_DNNtraining      = cma::str2bool( getConfigOption("DNNtraining") );

    cma::read_file( getConfigOption("inputfile"), m_filesToProcess );
    cma::read_file( getConfigOption("treenames"), m_treeNames );
    m_treename = getConfigOption("treename");

    return;
}


void configuration::print(){
    // -- Print the configuration
    std::cout << " ** CyMiniAna ** " << std::endl;
    std::cout << " --------------- " << std::endl;
    std::cout << " CONFIGURATION :: Printing configuration " << std::endl;
    std::cout << " " << std::endl;
    for (const auto& config : m_map_config){
        std::cout << " " << config.first << "\t\t\t" << config.second << std::endl;
    }
    std::cout << " --------------- " << std::endl;

    return;
}


std::string configuration::getConfigOption( std::string item ){
    /* Check that the item exists in the map & return it; otherwise throw exception  */
    std::string value("");

    try{
        value = m_map_config.at(item);
    }
    catch(const std::exception&){
        cma::ERROR("CONFIG : Option "+item+" does not exist in configuration.");
        cma::ERROR("CONFIG : This does not exist in the default configuration either.");
        cma::ERROR("CONFIG : Returing an empty string.");
    }

    return value;
}


void configuration::readMetadata(TFile& file,const std::string& metadataTreeName){
    /* Read metadata TTree */
    m_primaryDataset = "";
    m_isMC = false;

    if (metadataTreeName.size()<1) return;  // no metadata tree to read

    TTreeReader metadata(metadataTreeName.c_str(), &file);

    TTreeReaderValue<std::string> primaryDataset(metadata, "primaryDataset");
    metadata.Next();

    std::string pd  = *primaryDataset;
    for (const auto& x : m_mapOfPrimaryDatasets){   // only contains MC samples
        if (x.second==pd){
            m_isMC = true;
            break;
        }
    }

    m_primaryDataset   = pd;

    cma::DEBUG("CONFIGURATION : Primary dataset = "+m_primaryDataset);

    return;
}


void configuration::inspectFile( TFile& file, const std::string& metadataTreeName ){
    /* Compare filenames to determine file type */
    m_NTotalEvents   = 0;
    m_primaryDataset = "";
    m_isMC = false;
    readMetadata(file,metadataTreeName);                 // access metadata

    return;
}


void configuration::setTreename(std::string treeName){
    m_treename = treeName;
    return;
}

void configuration::setFilename(std::string fileName){
    m_filename = fileName;
    return;
}

bool configuration::isNominalTree(){
    return isNominalTree( m_treename );
}

bool configuration::isNominalTree( const std::string &tree_name ){
    /* Check if tree is a nominal one */
    bool isNominal(false);
    if (tree_name.compare("tree/eventVars")==0 || tree_name.compare("eventVars")==0)
        isNominal = true;
    else
        isNominal = false;

    return isNominal;
}

bool configuration::isMC( TFile& file ){
    /* Check the sum of weights tree DSIDs (to determine Data || MC) */
    inspectFile( file );
    return m_isMC;
}

void configuration::check_btag_WP(const std::string &wkpt){
    /* Check the b-tagging working point */
    if(! std::any_of(m_btag_WPs.begin(), m_btag_WPs.end(), [&](const std::string& s){return (s.compare(wkpt) == 0);} ) ) {
        cma::ERROR("CONFIG : Unknown b-tagging WP: "+wkpt+". Aborting!");
        cma::ERROR("CONFIG : Available b-tagging WPs: "+cma::vectorToStr(m_btag_WPs));
        exit(EXIT_FAILURE);
    }

    return;
}

// THE END
