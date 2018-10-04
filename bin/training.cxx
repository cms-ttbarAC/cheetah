/*
Created:        --
Last Updated:   17 February 2018

Dan Marley
daniel.edison.marley@cernSPAMNOT.ch
Texas A&M University
-----

Basic steering macro for running cheetah
 - Make flat ntuples for machine learning
 - Make histograms to compare features
*/
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1.h"
#include "TSystem.h"
#include "TMath.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <map>
#include <fstream>
#include <string>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Analysis/cheetah/interface/tools.h"
#include "Analysis/cheetah/interface/configuration.h"
#include "Analysis/cheetah/interface/Event.h"
#include "Analysis/cheetah/interface/eventSelection.h"
#include "Analysis/cheetah/interface/miniTree.h"
#include "Analysis/cheetah/interface/histogrammer.h"


int main(int argc, char** argv) {
    /* Steering macro for cheetah */
    if (argc < 2) {
        cma::HELP();
        return -1;
    }

    // configuration
    configuration config(argv[1]);                         // configuration file
    config.initialize();

    unsigned long long maxEntriesToRun(0);     // maximum number of entries in TTree
    unsigned int numberOfEventsToRun(0);       // number of events to run
    bool passEvent(false);                     // event passed selection

    int nEvents = config.nEventsToProcess();                      // requested number of events to run
    std::string outpathBase = config.outputFilePath();            // directory for output files
    unsigned long long firstEvent      = config.firstEvent();     // first event to begin running over
    std::vector<std::string> filenames = config.filesToProcess(); // list of files to process
    std::string selection(config.selections().at(0));             // selection to apply (only assuming 1 at the moment)
    std::string treename(config.treename());

    std::string customDirectory( config.customDirectory() );
    if (customDirectory.length()>0  && customDirectory.substr(0,1).compare("_")!=0){
        customDirectory = "_"+customDirectory; // add '_' to beginning of string, if needed
    }

    // event selection
    eventSelection evtSel( config );
    evtSel.initialize(selection, config.cutsfiles().at(0) ); // need event selection and cutsfiles names


    // --------------- //
    // -- File loop -- //
    // --------------- //
    unsigned int numberOfFiles(filenames.size());
    unsigned int currentFileNumber(0);
    cma::INFO("TRAIN : *** Starting file loop *** ");
    for (const auto& filename : filenames) {

        ++currentFileNumber;
        cma::INFO("TRAIN :   Opening "+filename+"   ("+std::to_string(currentFileNumber)+"/"+std::to_string(numberOfFiles)+")");

        auto file = TFile::Open(filename.c_str());
        if (!file || file->IsZombie()){
            cma::WARNING("TRAIN :  -- File: "+filename);
            cma::WARNING("TRAIN :     does not exist or it is a Zombie. ");
            cma::WARNING("TRAIN :     Continuing to next file. ");
            continue;
        }

        cma::DEBUG("TRAIN : set file name and inspect ");
        config.setFilename( filename );   // Use the filename to determine primary dataset and information about the sample
        config.inspectFile( *file );      // Determine information about the input file (metadata)

        cma::DEBUG("TRAIN : get list of keys ");
        std::vector<std::string> fileKeys;
        cma::getListOfKeys(file,fileKeys);      // keep track of ttrees in file


        // -- Output file -- //
        cma::DEBUG("TRAIN : setup output directory ");
        struct stat dirBuffer;
        std::string outpath = outpathBase+"/"+selection+customDirectory;
        if ( !(stat((outpath).c_str(),&dirBuffer)==0 && S_ISDIR(dirBuffer.st_mode)) ){
            cma::DEBUG("TRAIN : Creating directory for storing output: "+outpath);
            system( ("mkdir "+outpath).c_str() );  // make the directory so the files are grouped together
        }

        std::size_t pos   = filename.find_last_of(".");     // the last ".", i.e., ".root"
        std::size_t found = filename.find_last_of("/");     // the last "/"
        std::string outputFilename = filename.substr(found+1,pos-1-found); // betwee "/" and "."
        // hopefully this returns: "diboson_WW_361082" given something like:
        // "/some/path/to/file/diboson_WW_361082.root"

        std::string fullOutputFilename = outpath+"/"+outputFilename+".root";
        std::unique_ptr<TFile> outputFile(TFile::Open( fullOutputFilename.c_str(), "RECREATE"));
        cma::INFO("TRAIN :   >> Saving to "+fullOutputFilename);

        histogrammer histMaker(config,"ML");      // initialize histogrammer
        histMaker.initialize( *outputFile );

        evtSel.setCutflowHistograms( *outputFile );

        // check that the ttree exists in this file before proceeding
        if (std::find(fileKeys.begin(), fileKeys.end(), treename) == fileKeys.end()){
            cma::INFO("TRAIN : TTree "+treename+" is not present in this file, continuing to next TTree");
            continue;
        }

        // -- Load TTree to loop over
        cma::INFO("TRAIN :      TTree "+treename);
        TTreeReader myReader(treename.c_str(), file);

        // -- Make new Tree in Root file
        miniTree miniTTree(config);          // initialize TTree for new file
        miniTTree.initialize( *outputFile );

        // -- Number of Entries to Process -- //
        maxEntriesToRun = myReader.GetEntries(true);
        if (maxEntriesToRun<1) // skip files with no entries
            continue;

        if (nEvents < 0 || ((unsigned int)nEvents+firstEvent) > maxEntriesToRun)
            numberOfEventsToRun = maxEntriesToRun - firstEvent;
        else
            numberOfEventsToRun = nEvents;

        // ---------------- //
        // -- Event Loop -- //
        // ---------------- //
        Long64_t imod = 1;                     // print to the terminal
        Event event = Event(myReader, config);

        Long64_t eventCounter = 0;    // counting the events processed
        Long64_t entry = firstEvent;  // start at a different event!
        while (myReader.Next()) {

            if (eventCounter+1 > numberOfEventsToRun){
                cma::INFO("TRAIN : Processed the desired number of events: "+std::to_string(eventCounter)+"/"+std::to_string(numberOfEventsToRun));
                break;
            }

            if (entry%imod==0){
                cma::INFO("TRAIN :       Processing event "+std::to_string(entry) );
                if(imod<2e4) imod *=10;
            }

            // -- Build Event -- //
            cma::DEBUG("TRAIN : Execute event");
            event.execute(entry);
            // now we have event object that has the event-level objects in it
            // pass this to the selection tools

            // -- Event Selection -- //
            cma::DEBUG("TRAIN : Apply event selection");
            evtSel.setObjects(event);
            passEvent = evtSel.applySelection();

            if (passEvent){
                cma::DEBUG("TRAIN : Passed selection, now reconstruct ttbar & save information");
                event.ttbarReconstruction();

                // For ML, we are training on boosted top quarks in data!
                // Only save features of the AK8 to the output ntuple/histograms
                // didn't save metadata -- set these values to 1 for now
                std::map<std::string,double> features2save;
                features2save["xsection"] = 1.; //s.XSection;
                features2save["kfactor"]  = 1.; //s.KFactor;
                features2save["sumOfWeights"] = 1.; //s.sumOfWeights;
                features2save["nominal_weight"] = 1.; //event.nominal_weight();

                Ttbar1L tt = event.ttbar1L();             // setup for CWoLa (large-R jet from l+jets events)
                Ljet ljet = tt.ljet;

                // Quality cuts on the jets
                // positive CSVv2 values, and subjet charges that aren't really large
                if (ljet.features.at("ljet_subjet0_bdisc")>0 && ljet.features.at("ljet_subjet1_bdisc")>0 && 
                    std::abs(ljet.features.at("ljet_subjet0_charge"))<20 && std::abs(ljet.features.at("ljet_subjet1_charge"))<20){

                    for (const auto& x : ljet.features){
                        features2save[x.first] = x.second;
                    }
                    // extra features for plotting
                    features2save["ljet_BEST_t"] = ljet.BEST_t;
                    features2save["ljet_BEST_w"] = ljet.BEST_w;
                    features2save["ljet_BEST_z"] = ljet.BEST_z;
                    features2save["ljet_BEST_h"] = ljet.BEST_h;
                    features2save["ljet_BEST_j"] = ljet.BEST_j;
                    features2save["ljet_SDmass"] = ljet.softDropMass;
                    features2save["ljet_tau1"]   = ljet.tau1;
                    features2save["ljet_tau2"]   = ljet.tau2;
                    features2save["ljet_tau3"]   = ljet.tau3;
                    features2save["ljet_tau21"]  = ljet.tau21;
                    features2save["ljet_tau32"]  = ljet.tau32;
                    features2save["ljet_isHadTop"] = ljet.isHadTop*1.0;
                    features2save["ljet_contain"] = ljet.containment;

                    miniTTree.saveEvent(features2save);
                    histMaker.fill(features2save);
                } // end quality cut on AK8
            }

            // iterate the entry and number of events processed
            ++entry;
            ++eventCounter;
        } // end event loop

        event.finalize();
        miniTTree.finalize();

        // put overflow/underflow content into the first and last bins
        histMaker.overUnderFlow();

        cma::INFO("TRAIN :   END Running  "+filename);
        cma::INFO("TRAIN :   >> Output at "+fullOutputFilename);

        outputFile->Write();
        outputFile->Close();

        // -- Clean-up stuff
        delete file;          // free up some memory 
        file = ((TFile *)0);  // (no errors for too many root files open)
    } // end file loop

    cma::INFO("TRAIN : *** End of file loop *** ");
    cma::INFO("TRAIN : Program finished. ");
}

// THE END
