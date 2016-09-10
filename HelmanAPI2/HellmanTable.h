#ifndef HELLMAN_TABLE_H
#define HELLMAN_TABLE_H

#include "SubTable.h"
#include "ThreadPool.h"

using namespace std;

namespace RaaS {

    enum LoadType : uint8_t {
        Append = 1,
        Override = 2
    };



    //Dynemic size Hellman Table
    class HellmanTable
    {
        const uint16_t m_THREAD_COUNT = 32; //Maximum number of threads (size of ThreadPool)
        len_t m_chainLength;                //Expected length of chains
        vector<SubTable> m_tables;          //Vector of smaller Hellman Tables with fixed size
        ThreadPool m_threads;               //Pool of threads that can be used to execute tasks

    public:
        int nCurrentFileIndex;
        std::string sFilesPath;
        /*
        Default Constructor : Initlizes the chain length
        @param chainLength : Initlize chain length value
        */
        HellmanTable(len_t chainLength, std::string filesPath) : m_threads(m_THREAD_COUNT), m_chainLength(chainLength), sFilesPath(filesPath), nCurrentFileIndex(0){}
        HellmanTable(len_t chainLength) : m_threads(m_THREAD_COUNT), m_chainLength(chainLength), nCurrentFileIndex(0){}

        /*
         * Add a Random Table from the FS
         */
        void addChainBlocksFromFS(count_t amount);

        /*
        Restore hellman table from file
        @param amount : Name of file containing the binary data
        */
        void load(const string FileName, const LoadType loadType);

        /*
        Save hellman table to file
        @param amount : Name of file containing the binary data
        */
        void save(const string FileName);

        /*
        Add a certain amount of Sub-Tables (multithreaded)
        @param amount : Desired amount of Sub-Tables to be added.
        */
        void addChainBlocks(count_t amount);

        /*
        Remove a certain amount of Sub-Tables (multithreaded)
        @param amount : Desired amount of Sub-Tables to be removed.
        */
        void removeChainBlocks(count_t amount);

        /*
        Extend all chains by a certain amount (multithreaded)
        @param amount : Desired amount to be extended.
        */
        void extend(len_t amount);

        /*
        Shorten all chains by a certain amount (multithreaded)
        @param amount : Desired amount to be shortened.
        */
        void shorten(len_t amount);

        /*
        Compute MD5 preimage for a given query (multithreaded)
        @param query        : The search query value.
        @param ref preimage : Refrence to preimage value on success, otherwise null value.
        @Return value       : True if preimage is found, False otherwise.
        */
        bool getPreimage(hash_t query, hash_t& preimage);

        void clear() { m_tables.clear(); }

        //Returns current amount of Sub-Tables
        const count_t getSubTablesCount() const { return m_tables.size(); }

        //Returns current total amount of chains
        const count_t getChainCount() const { return m_tables.size() * SubTable::BLOCK_SIZE; }

        //Returns expected length of chains
        const len_t getChainLen() const { return m_chainLength; }

        //Returns representative chain statistics
        const chain_stat getStats() const { return m_tables.front().getStats(); }


    };

}

#endif //HELLMAN_TABLE_H
