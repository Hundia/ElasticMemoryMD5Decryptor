#ifndef SUB_TABLE_H
#define SUB_TABLE_H

#include <unordered_map>
#include <fstream>
#include <ext/malloc_allocator.h>

#include "MD5.h"

using namespace std;

namespace RaaS {

    //Type Definitions:

    //Statistics about single chain
    typedef struct chain_stat{
        hash_t SP;      //Start Point
        hash_t EP;      //End point
        len_t length;   //Length of chain
        hash_t flavor;  //Function flavor
    } chainStat;

    //Chain representative type : chain end-point, chain start-point
    typedef pair<hash_t, hash_t> value_type;

    //Memory allocator for Hash Table.
    typedef __gnu_cxx::malloc_allocator<value_type> allocator_type;

    /*
    Hash table
    Key :     chain end-point (EP).
    Value :   chain start-point (SP).
    */
    typedef unordered_map<hash_t, hash_t, hash<hash_t>, equal_to<hash_t>, allocator_type> chain_map;


    //////////////////////////////////////////////////////////////////////////////////////////////////////

    //Hellman Table class implementation with constant size and unique flavor
    class SubTable
    {
        chain_map m_map;        //Hash Table : end-point -> start-point
        len_t m_chainLength;    //Expected length of chains
        hash_t m_flavor;        //Flavor to distinguish the function from the other subtables

        //Size on Disk

    public:
        static const hash_t FAILURE = 0;        //Preimage not found (equivalent to NULL)
                                                //Note : Since FAILURE value is 0, zero could not be a preimage value

        static const count_t BLOCK_SIZE = 2048; //Number of chains per block
        static const uint64_t m_DISK_SIZE = sizeof(m_chainLength) + sizeof(m_flavor) + BLOCK_SIZE * sizeof(value_type);

        /*
        Default Constructor : Allocate buckets for hash table.
        @param length : The desired length of the chains.
        @param flavor : Distinguish falvor value.
        */
        SubTable(const len_t length, const hash_t flavor);

        /*
        Generates BLOCK_SIZE random start-points and corresponding end-points, and build data structure.
        */
        void Init();

        /*
        Read BLOCK_SIZE chains from given data array, and build data structure.
        @param data : Array of chains, representred as pair <End-point, Start-point>
        */
        void load(value_type data[BLOCK_SIZE]);

        /*
        Extend all chains by a certain amount
        @param amount : The desired amount to be extended.
        */
        void extend(len_t amount);

        /*
        Shorten all chains by a certain amount
        @param amount : The desired amount to be shortened.
        */
        void shorten(len_t amount);

        /*
        Compute the preimage of a given query.
        @param query : The query value to be resolved.
        Returns value of preimage on success, otherwise returns FAILURE.
        */
        hash_t resolve(hash_t query);

        //Returns representative chain statistics
        const chain_stat getStats() const { return {m_map.begin()->second, m_map.begin()->first, m_chainLength, m_flavor}; }

        friend inline std::ofstream& operator<< (std::ofstream& out, const SubTable& table){
            value_type data[BLOCK_SIZE];

            out.write((char*)&(table.m_flavor), sizeof(table.m_flavor));
            out.write((char*)&(table.m_chainLength), sizeof(table.m_chainLength));

            uint32_t i = 0;
            for(auto& val : table.m_map)
                data[i++] = val;

            out.write((char*)data, sizeof(data));

            return out;
        }

    };
}

#endif //SUB_TABLE_H

