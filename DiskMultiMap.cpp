#include <iostream>
#include <cstring>
#include <string>
#include <functional>
#include "DiskMultiMap.h"
#include "MultiMapTuple.h"
#include "InteractionTuple.h"

using namespace std;

// ------------------------------------------- DISK MULTI MAP CONSTRUCTOR -------------------------------------------------

DiskMultiMap::DiskMultiMap() {}

// ------------------------------------------- DISK MULTI MAP DESTRUCTOR -------------------------------------------------

DiskMultiMap::~DiskMultiMap() {
    close();
}

// -------------------------------------------------- CREATE NEW ---------------------------------------------------------

bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets) {
    if(bf.isOpen())
        bf.close();
    
    bool create = bf.createNew(filename);
    
    if(!create) {
        cout << "Error Creating New Binary File!" << endl;
        return false;
    }
    
    m_numBuckets = numBuckets;
    bf.write(m_numBuckets, 0);
    
    r_head = 0;
    bool reuse = bf.write(r_head, sizeof(m_numBuckets));
    
    if(!reuse) {
        cout << "Error Creating Reuse Head!" << endl;
        return false;
    }
    
    for(int i = 0; i < m_numBuckets; i++) {  // set all the num buckets
        Bucket new_buck(0);
        bf.write(new_buck, sizeof(m_numBuckets) + m_sizeOffset + m_sizeBucket*i);
    }
    
    m_sizeOfBuckArray = m_sizeBucket*m_numBuckets;
    
    return true;
}

// -------------------------------------------------- OPEN EXISTING -------------------------------------------------------

bool DiskMultiMap::openExisting(const std::string& filename) {
    if(bf.isOpen())
        bf.close();
    return bf.openExisting(filename);
}

// ---------------------------------------------------- CLOSE -------------------------------------------------------------

void DiskMultiMap::close() {
    if(bf.isOpen())
        bf.close();
}

// ---------------------------------------------------- INSERT -------------------------------------------------------------

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context) {
    
    if(key.size() > 120 || value.size() > 120 || context.size() > 120) {
        return false;
    }
    
    hash<string> str_hash;
    unsigned long hashValue = str_hash(key);
    unsigned int bucket = hashValue % m_numBuckets;
    BinaryFile::Offset bucket_addr = m_sizeOfHeader + bucket*m_sizeBucket;
    
    Bucket buck(0);
    bool buck_read = bf.read(buck, bucket_addr);
    if(!buck_read) {
        cout << "Error Reading Bucket!" << endl;
        return false;
    }
    
    HashNode new_hash(key, value, context, 0);
    
    
    if(r_head == 0) {  // no reusable nodes available I have to expand the memory
        if(buck.m_bucketOff == 0) { // if the bucket is not pointing to anything right now
            buck.m_bucketOff = bf.fileLength();
            bf.write(buck, bucket_addr);
            bf.write(new_hash, buck.m_bucketOff);
        }
        
        else {
            new_hash.m_next = buck.m_bucketOff;
            buck.m_bucketOff = bf.fileLength();
            bf.write(buck, bucket_addr);
            bf.write(new_hash, buck.m_bucketOff);
        }
    }
    
    else {  // we have a reusable node and must use it!
        if(buck.m_bucketOff == 0) {
            HashNode temp(" ", " ", " ", 0);
            bf.read(temp, r_head);
            buck.m_bucketOff = r_head;
            bf.write(buck, bucket_addr);
            bf.write(new_hash, buck.m_bucketOff);
            r_head = temp.m_next;
        }
        
        else {
            HashNode temp(" ", " ", " ", 0);
            bf.read(temp, r_head);
            new_hash.m_next = buck.m_bucketOff;
            buck.m_bucketOff = r_head;
            bf.write(buck, bucket_addr);
            bf.write(new_hash, buck.m_bucketOff);
            r_head = temp.m_next;
        }
    }
    
    return true;
}

// ------------------------------------------------- ITERATOR SEARCH ----------------------------------------------------------

DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key) {  // BE SURE TO CHECK FOR COLLISIONS
  
    hash<string> str_hash;
    unsigned long hashValue = str_hash(key);
    unsigned int bucket = hashValue % m_numBuckets;
    BinaryFile::Offset bucket_addr = m_sizeOfHeader + bucket*m_sizeBucket;

    Bucket b(0);
    bf.read(b, bucket_addr);
    
    if(b.m_bucketOff == 0)
        return DiskMultiMap::Iterator::Iterator();
    
    BinaryFile::Offset temp = b.m_bucketOff;
    HashNode temp_hash(" ", " ", " ", 0);
    bf.read(temp_hash, temp);
   
    while(temp != 0) {
        if(strcmp(key.c_str(), temp_hash.m_key) == 0) {
            return DiskMultiMap::Iterator::Iterator(temp, temp_hash.m_next, &bf, key);
        }
        temp = temp_hash.m_next;
        bf.read(temp_hash, temp);
    }
    
    // if I didn't find the key in my buckets, return the invalid iterator
    return DiskMultiMap::Iterator::Iterator();
}

// ---------------------------------------------------- ERASE -------------------------------------------------------------

int DiskMultiMap::erase(const std::string &key, const std::string &value, const std::string &context) {
    
    hash<string> str_hash;
    unsigned long hashValue = str_hash(key);
    unsigned int bucket = hashValue % m_numBuckets;
    BinaryFile::Offset bucket_addr = m_sizeOfHeader + bucket*m_sizeBucket;
    
    int removed = 0;
    
    Bucket buck(0);
    bf.read(buck, bucket_addr);
    //HashNode temp(" ", " ", " ", 0);
    //bf.read(temp, buck.m_bucketOff);
    
    BinaryFile::Offset temp_bucketOff = buck.m_bucketOff;
    BinaryFile::Offset prev_off = bucket_addr;
    
    while(temp_bucketOff != 0) {
        
        HashNode temp(" ", " ", " ", 0);
        bf.read(temp, temp_bucketOff);
        
        if(strcmp(temp.m_key, key.c_str()) == 0 && strcmp(temp.m_value, value.c_str()) == 0 && strcmp(temp.m_context, context.c_str()) == 0) {
            
            if(temp_bucketOff == buck.m_bucketOff) {  // we are removing the head node
            
                buck.m_bucketOff = temp.m_next;
                bf.write(buck, bucket_addr);
                BinaryFile::Offset holder = temp_bucketOff;
                temp.m_next = r_head;
                
                if(r_head == 0)
                    r_head = holder;
                else {
                    BinaryFile::Offset point = r_head;
                    r_head = holder;
                    temp.m_next = point;
                }
                bf.write(r_head, sizeof(m_numBuckets));
                bf.write(temp, temp_bucketOff);
                temp_bucketOff = buck.m_bucketOff;
                removed++;
                continue;
            }
            
            else {  // we are removing some middle or end node  ERROR WITHIN THIS ELSE SOMEWHERE!!!
                
                HashNode prev(" ", " ", " ", 0);
                bf.read(prev, prev_off);
                BinaryFile::Offset holder = temp_bucketOff;
                prev.m_next = temp.m_next;
                temp.m_next = r_head;
                bf.write(temp, temp_bucketOff);
                bf.write(prev, prev_off);
                
                
                if(r_head == 0) {
                    r_head = holder;
                }
                else {
                    BinaryFile::Offset point = r_head;
                    r_head = holder;
                    temp.m_next = point;
                }
                
                temp_bucketOff = prev.m_next;
                removed++;
                continue;
            }
            
        }
        
        prev_off = temp_bucketOff;
        temp_bucketOff = temp.m_next;
        
    }
    
    
    return removed;
}

// ------------------------------------ ITERATOR FUNCTIONS BEGIN -------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------

DiskMultiMap::Iterator::Iterator() {
    m_isValid = 0;
}

// ------------------------------------ PERSONAL CONSTRUCTOR --------------------------------------------------------

DiskMultiMap::Iterator::Iterator(BinaryFile::Offset curr, BinaryFile::Offset next, BinaryFile* bf, const std::string &key) : m_curr(curr), m_next(next), m_iter_bf(bf) {
    strcpy(m_iter_key, key.c_str());
    m_isValid = 1;
}

// ------------------------------------ ITERATOR IS VALID -----------------------------------------------------------

bool DiskMultiMap::Iterator::isValid() const {
    return m_isValid;
}

// --------------------------------------- ITERATOR OPERATOR ++ ------------------------------------------------------

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {  //TEST THIS! CHECK THIS FOR COLLISIONS!
    if(isValid()) {

        m_curr = m_next;
        
        while(m_curr != 0) {
            HashNode temp_node(" ", " ", " ", 0);
            m_iter_bf->read(temp_node, m_curr);
            
            if(strcmp(temp_node.m_key, m_iter_key) == 0) { // found the association that maps to the correct key
                m_next = temp_node.m_next;
                return *this;  // get out because m_curr now holds the correct offset iterator should move to
            }
            
            m_curr = temp_node.m_next;
        } // end of while loop
        
        if(m_curr == 0) {
            m_isValid = 0;
        }
    }
    
    return *this;
    
}


// ---------------------------------------- ITERATOR OPERATOR * ------------------------------------------------------

MultiMapTuple DiskMultiMap::Iterator::operator*() {
    
    MultiMapTuple m;
    
    if(!isValid()) {
        m.key = "";
        m.value = "";
        m.context = "";
        return m;
    }
    
    HashNode new_hash(" ", " ", " ", 0);
    m_iter_bf->read(new_hash, m_curr);
    
    m.key = new_hash.m_key;
    m.value = new_hash.m_value;
    m.context = new_hash.m_context;
    return m;
    
}


// ------------------------------------- DELETE THIS -------------------------------------------
/*
void DiskMultiMap::print()
{
    cout << "------------------" << endl;
    cout << "NumBuckets: " << m_numBuckets << endl;
    cout << "r_head: " << r_head << endl;
    cout << "Filelength: " << bf.fileLength() << endl;
    cout << "Size of offset: " << m_sizeOffset << endl;
    cout << "Size of bucket: " << m_sizeBucket << endl;
    cout << "Size of node: " << m_sizeNode << endl;
    cout << "-------------------" << endl;
    for (int i = 0; i < m_numBuckets; i++)
    {
        Bucket b(0);
        bf.read(b, sizeof(m_numBuckets) + m_sizeOffset + (i*m_sizeBucket));
        cout << "bucket address: " << sizeof(m_numBuckets) + m_sizeOffset + (i*m_sizeBucket) << endl;
        cout << "bucket's node address: " << b.m_bucketOff << endl;
    }
    cout << "-----------------" << endl;
    for (int i = 0; i < 10; i++)
    {
        HashNode n(" ", " ", " ", 0);
        bf.read(n, m_sizeOfHeader + m_sizeOfBuckArray + i*m_sizeNode);					// CHECK VALUE
        cout << "Address: " << m_sizeOfHeader + m_sizeOfBuckArray + i * m_sizeNode << endl;
        cout << "Key: " << n.m_key << endl;
        cout << "Value: " << n.m_value << endl;
        cout << "Context: "<< n.m_context << endl;
        cout << "Next: " << n.m_next << endl;
        cout << endl;
    }

}
 */

