/*
Created:        14 May       2016
Last Updated:   20 August    2016

Dan Marley
daniel.edison.marley@cernSPAMNOT.ch
Texas A&M University
-----

Common tools needed

*/
#include "Analysis/CyMiniAna/interface/tools.h"

namespace cma{


void check_file(const std::string & filename) {
    /* Check that file exists */
    std::ifstream f = open_file(filename);
    return;
}

void check_file(const std::ifstream& file, const std::string& fname){
    /* Check that file exists */
    if (!file) {
        cma::ERROR("TOOLS : File does not exist:       "+fname);
        cma::ERROR("TOOLS : Exiting. ");
        assert(file);
    }
    return;
}

std::ifstream open_file(const std::string &filename) {
    /* Open file */
    std::ifstream ifile(filename.c_str());
    check_file(ifile,filename);
    return ifile;
}


void read_file( const std::string &file_name, std::vector<std::string> &values, const std::string &comment ) {
    /* Read in a generic file and put it into a vector of strings */
    std::ifstream tmp_name = open_file(file_name);

    // open the file and put the data into a vector
    std::string line("");
    if (tmp_name.is_open()){

        while (std::getline(tmp_name, line)) {
            std::string newstring(line);

            // allow for comments
            std::size_t lineComment = line.find(comment);
            if (lineComment != std::string::npos) newstring = line.substr(0, lineComment);

            // remove all white spaces at the end of the string
            std::size_t space_pos = newstring.rfind(" ");
            while ( space_pos != std::string::npos && space_pos == newstring.size()-1 ) {
                newstring = newstring.substr(0, newstring.rfind(" "));
                space_pos = newstring.rfind(" ");
            }

            // ignore empty lines
            if(newstring.length()==0) continue;

            values.push_back(newstring); // put values into vector
        }

        tmp_name.close();
    }

    return;
}



void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }

    return;
}


void getListOfBranches( TTree* tree, std::vector<std::string>& treeBranches ){
    /* Find the list of branches in the TTree */
    treeBranches.clear();

    TList* list = (TList*)tree->GetListOfBranches();
    TIter iter(list->MakeIterator());
    while(TObject* obj = iter()){
        TKey* key = (TKey*)obj;
        std::string branchname( key->GetName() );
        treeBranches.push_back( branchname );
    }

    return;
}


void getListOfKeys( TFile* file, std::vector<std::string>& fileKeys ){
    /* Find the list of TTrees in a file */
    fileKeys.clear();

    TList* list = file->GetListOfKeys();
    TIter iter(list->MakeIterator());
    while(TObject* obj = iter()){
        TKey* key = (TKey*)obj;
        std::string keyname( key->GetName() );
        std::string classname( key->GetClassName() );
        if (classname.find("TH1")!=std::string::npos || classname.find("TH2")!=std::string::npos){
            fileKeys.push_back(keyname);  // just a histogram
        }
        else{
            // Check if this is a directory that contains TTrees
            try {
                TDirectory* dir = (TDirectory*)file->Get(keyname.c_str());
                TList* sublist  = dir->GetListOfKeys();
                TIter subiter(sublist->MakeIterator());
                while (TObject* subobj = subiter()){
                    TKey* key = (TKey*)subobj;
                    std::string subkeyname( key->GetName() );
                    fileKeys.push_back(keyname+"/"+subkeyname);
                }
            }
            catch (...){
                fileKeys.push_back(keyname);
            }
        }
    }

    return;
}


bool str2bool( const std::string value ){
    /* Turn string into boolean */
    bool valueBoolean(false);

    if (value.compare("True")==0 || value.compare("true")==0 || value.compare("1")==0){
        valueBoolean = true;
    }
    else{
        valueBoolean = false;
    }

    return valueBoolean;
}


std::string vectorToStr( const std::vector<std::string> &vec ){
    std::string str_list;
    for( const std::string &str : vec)
        str_list += str + std::string(",");
    str_list.pop_back(); // remove last comma
    return str_list;
}


bool deltaRMatch( const TLorentzVector &particle1, const TLorentzVector &particle2, const double deltaR ){
    /* Do the deltaR calculation (in one place) */
    return (particle1.DeltaR(particle2)<deltaR);
}


float ptrel( const TLorentzVector& a, const TLorentzVector& b){
    /* pTrel between two objects 
       - https://github.com/UHH2/UHH2/blob/master/common/src/Utils.cxx#L34
    */
    TVector3 a3 = a.Vect();
    TVector3 b3 = b.Vect();

    float pt_rel = a3.Cross(b3).Mag()/b3.Mag();

    return pt_rel;
}


std::string m_debugLevel = "SetMe";
void setVerboseLevel( const std::string& verboseLevel ) {
    m_debugLevel = verboseLevel;
    return;
}

void DEBUG(const std::string& message){
    /* Debug level (verbosity of output) */
    verbose("DEBUG",message);
    return;
}
void INFO(const std::string& message){
    /* Info level (verbosity of output) */
    verbose("INFO",message);
    return;
}
void WARNING(const std::string& message){
    /* Warning level (verbosity of output) */
    verbose("WARNING",message);
    return;
}
void ERROR(const std::string& message){
    /* Error level (verbosity of output) */
    verbose("ERROR",message);
    return;
}

void verbose(const std::string level, const std::string& message){
    /* 
       Printing output to console (debug,warning,error messages)
         if the level is "DEBUG", then all messages should be printed (DEBUG/INFO/WARNING/ERROR)
         if the level is "INFO", then only INFO/WARNING/ERROR messages should be printed
         if the level is "WARNING", then only WARNING/ERROR messages should be printed
         if the level is "ERROR", then only ERROR messages should be printed
    */
    std::map<std::string,unsigned int> debugMap = {
            {"DEBUG",   0},
            {"INFO",    1},
            {"WARNING", 2},
            {"ERROR",   3} };

    if ( debugMap.at( level ) >= debugMap.at( m_debugLevel ))
        std::cout << " " << level << " :: " << message << std::endl;

    return;
}

std::map<std::string,unsigned int> verboseMap() {
    /* mapping of verbose level to integer */
    std::map<std::string,unsigned int> verbose_map = {
            {"DEBUG",   0},
            {"INFO",    1},
            {"WARNING", 2},
            {"ERROR",   3} };
    
    return verbose_map;
}

void HELP(const std::string& runExecutable){
    /* HELP message (pass 'runExecutable' in case you are running from some 
       script like 'skim', 'run', or a custom macro)
    */
    std::cout << "\n   ** CyMiniAna ** " << std::endl;
    std::cout << "   --------------- " << std::endl;
    std::cout << "   Framework to perform event selection, write-out" << std::endl;
    std::cout << "   a few histograms or efficiencies, and make plots.\n" << std::endl;

    std::cout << "   To run:" << std::endl;
    std::cout << "      ./" << runExecutable << " share/cmaConfig.txt \n" << std::endl;
    std::cout << "    where 'share/cmaConfig.txt' is the configuration file \n" << std::endl;

    return;
}

} // end namespace

// THE END
