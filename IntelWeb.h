#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>

class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& badInteractions
                       );
    bool purge(const std::string& entity);
    
private:
    // Your private member declarations will go here
    DiskMultiMap m_To;          // Multi Map to Map "To" Telemetry lines, Target to Initiatior 
    DiskMultiMap m_From;        // Multi Map to Map "From" Telemetry lines, Initiator To Target
    DiskMultiMap m_Prev;        // Multi Map to Map Prevalences Telemetry lines
    
    bool countPrev(const std::string& value, int prevThresh) { // false if bad (below threshold)
        
        int count = 0;
        DiskMultiMap::Iterator iter = m_Prev.search(value);
        while(iter.isValid()) {
            count++;
            if(count >= prevThresh)
                return true;
            ++iter;
        }
        return false;
    }
    
};


inline
bool operator<(const InteractionTuple& lhs, const InteractionTuple& rhs) {
    if(lhs.context < rhs.context)
        return true;
    if(lhs.context > rhs.context)
        return false;
    if(lhs.context == rhs.context) {
        if(lhs.from < rhs.from)
            return true;
        if(lhs.from > rhs.from)
            return false;
        if(lhs.from == rhs.from) {
            if(lhs.to < rhs.to)
                return true;
            if(lhs.to > rhs.to)
                return false;
            else
                return false;
        }
    }
    
    return false;
}


#endif // INTELWEB_H_
