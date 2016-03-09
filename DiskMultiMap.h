#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator();
        // You may add additional constructors
        // Iterator( any parameters here I like );
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        // Your private member declarations will go here
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    // REMOVE THIS SHIT BEFORE SUBMITTING:
    void print();
    // PLEASE REMOVE OR YOU GET A ZERO!!!!!!!!!!!!!!!!!!!!!!
    
private:
    // Your private member declarations will go here
    BinaryFile bf;
    BinaryFile::Offset r_head;
    
    struct Bucket {
        Bucket(BinaryFile::Offset bucketoff) : m_bucketOff(bucketoff) {}
        BinaryFile::Offset m_bucketOff; // represents each bucket's list location!
    };
    
    struct HashNode {  // for the linked list inside every bucket!
        HashNode(const std::string key, const std::string value, const std::string context, BinaryFile::Offset next) : m_next(next) {
            
            strcpy(m_key, key.c_str());
            strcpy(m_value, value.c_str());
            strcpy(m_context, context.c_str());
            
        }
        char m_key[121];
        char m_value[121];
        char m_context[121];
        BinaryFile::Offset m_next; // pointer to next
    };
    
    int m_sizeOffset = sizeof(BinaryFile::Offset);
    int m_sizeNode = sizeof(HashNode);
    int m_sizeBucket = sizeof(Bucket); 
    int m_numBuckets; // stores number of buckets available
    int m_sizeOfHeader = sizeof(m_numBuckets) + m_sizeOffset;
    int m_sizeOfBuckArray;
    
};

#endif // DISKMULTIMAP_H_
