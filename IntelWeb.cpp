#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <unordered_map>
#include <set>
#include "IntelWeb.h"
#include "InteractionTuple.h"

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
    bool suc3 = m_Prev.createNew(filePrefix + "prevalence.dat", 2*num_buck);
    
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
    ifstream inf(telemetryFile);
		  
    string line;
    while (getline(inf, line))
    {
        istringstream iss(line);
        string to_input;
        string from_input;
        string cont_input;
    
        if(!(iss >> cont_input >> from_input >> to_input))
            continue;
        
        if(to_input.size() > 120 || from_input.size() > 120 || cont_input.size() > 120)
            continue;
        
        m_From.insert(from_input, to_input, cont_input);
        m_To.insert(to_input, from_input, cont_input);
        m_Prev.insert(from_input, "", "");
        m_Prev.insert(to_input, "", "");
    }
    
    return true;
}

// ----------------------------------------------- CRAWL ------------------------------------------------------------------

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood, std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& badInteractions) {
    
    // clear badentities and interactions vectors
    badEntitiesFound.clear();
    badInteractions.clear();
    
    queue<std::string> badqueue;
    unordered_map<string, DiskMultiMap::Iterator> toIter;
    unordered_map<string, DiskMultiMap::Iterator> fromIter;
    unordered_map<string, bool> prevCache;
    set<string> bad_entities;
    set<InteractionTuple> interact_entities;
    
    // push all the current indicators ones onto a LOCAL QUEUE
    
      for(int i = 0; i < indicators.size(); i++) {
        badqueue.push(indicators.at(i));
      }
    
    // take front of the queue and search m_From hashmap
    while(!badqueue.empty()) {
        string temp_front = badqueue.front();
        badqueue.pop();
        DiskMultiMap::Iterator Iter_From = m_From.search(temp_front);
        DiskMultiMap::Iterator Iter_To = m_To.search(temp_front);
        
        if(!Iter_From.isValid() && !Iter_To.isValid())
            continue;
        
    // de-reference the iterator and check the value in the tuple
        MultiMapTuple m_from = *Iter_From;
        // check if value is in the map
        bool checkPrev = countPrev(m_from.value, minPrevalenceToBeGood);
        
        
    
    // 2 more unordered maps to track iterators for from and to
    
    
    // LOCAL UNORDERED MAP (HASH TABLE) HOLDS VALIDITY OF YOUR PREVALANCE (VALID IF BELOW THRESHOLD, INVALID IF ABOVE)
    
    // check the prevalance of the value taken (write function in private to do this)
            // if its low enough, move the iterator, store value onto queue, store into LOCAL unordered map (checkprev does)
            // if its high enough, move the iterator along but dont push onto queue
    
    } // end if crawl while loop
    
    // copy the bad entities set to the vector (increasing order for the vector)
    for(auto p = bad_entities.begin(); p != bad_entities.end(); p++) {
        badEntitiesFound.push_back(*p);
    }
    
    // copy the interaction tuples set to the vector (increasing order for the vector)
    for(auto j = interact_entities.begin(); j != interact_entities.end(); j++) {
        badInteractions.push_back(*j);
    }
    
    // type cast and return
    return (unsigned int)badEntitiesFound.size(); // PLACEHOLDER CODE
}

// ----------------------------------------------- PURGE ------------------------------------------------------------------

bool IntelWeb::purge(const std::string& entity) {  // MIGHT not be in the correct time, TEST THIS
    
    bool purged = 0;
    DiskMultiMap::Iterator it = m_From.search(entity);
    
    while (it.isValid())
    {
        MultiMapTuple m = *it;
        string key = m.key;
        string value = m.value;
        string cont = m.context;
        
        ++it;
        m_From.erase(key, value, cont);
        m_To.erase(value, key, cont);
        //++it;
        purged = 1;
        
    }
    
    DiskMultiMap::Iterator it2 = m_To.search(entity);
    
    while (it2.isValid())
    {
        MultiMapTuple m = *it2;
        string key = m.key;
        string value = m.value;
        string cont = m.context;
        
        ++it2;
        m_To.erase(key, value, cont);
        m_From.erase(value, key, cont);
        //++it2;
        purged = 1;
        
    }
    
    m_Prev.erase(entity, "", "");
    return purged;
    
}
