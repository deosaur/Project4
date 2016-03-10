#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include "IntelWeb.h"

using namespace std;

// ----------------------------------------------- CONSTRUCTOR ----------------------------------------------------------

IntelWeb::IntelWeb() { }

// ----------------------------------------------- DESTRUCTOR ----------------------------------------------------------

IntelWeb::~IntelWeb() {
    close();
    // potentially free dynamically allocated memory....
}

// ----------------------------------------------- CREATE NEW ----------------------------------------------------------

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems) {
    close();
    
    int num_buck = maxDataItems/0.7;
    
    bool suc1 = m_To.createNew(filePrefix + "to.dat", num_buck);
    bool suc2 = m_From.createNew(filePrefix + "from.dat", num_buck);
    bool suc3 = m_Prev.createNew(filePrefix + "prevalence.dat", num_buck);
    
    if(suc1 && suc2 && suc3)
        return true;
    close();
    return false;
    
}

// ----------------------------------------------- OPEN EXISTING ----------------------------------------------------------

bool IntelWeb::openExisting(const std::string& filePrefix) {
    close();
    
    bool suc1 = m_To.openExisting(filePrefix + "to.dat");
    bool suc2 = m_From.openExisting(filePrefix + "from.dat");
    bool suc3 = m_Prev.openExisting(filePrefix + "prevalence.dat");
    
    if(suc1 && suc2 && suc3)
        return true;
    close();
    return false;

}

// ----------------------------------------------- CLOSE ------------------------------------------------------------------

void IntelWeb::close() {
    m_To.close();
    m_From.close();
    m_Prev.close();
}

// ----------------------------------------------- INGEST ------------------------------------------------------------------

bool IntelWeb::ingest(const std::string& telemetryFile) {
    ifstream inf("telemetryFile");
		  
    return true;
}

// ----------------------------------------------- CRAWL ------------------------------------------------------------------

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood, std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& badInteractions) {
    return 0; // PLACEHOLDER CODE
}

// ----------------------------------------------- PURGE ------------------------------------------------------------------

bool IntelWeb::purge(const std::string& entity) {
    return false; // PLACEHOLDER CODE
}
