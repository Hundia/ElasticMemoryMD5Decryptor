#include "SubTable.h"
#include <limits>

using namespace RaaS;

SubTable::SubTable(const len_t length, const hash_t flavor) : m_chainLength(length), m_flavor(flavor){
    m_map.reserve(BLOCK_SIZE);
}

void SubTable::Init(){
    m_map.clear();

    hash_t SP;
    hash_t EP;
	for (size_t i = 0; i < BLOCK_SIZE; i++){
        SP = gen_input(m_flavor ^ i);
        EP = n_hash(SP, m_flavor, m_chainLength);
        m_map.emplace(EP, SP);
    }
}

void SubTable::load(value_type data[BLOCK_SIZE]){
    m_map.clear();

    for(count_t i = 0; i < BLOCK_SIZE; i++)
        m_map.insert(data[i]);
}

void SubTable::extend(len_t amount){
    amount = min(amount, (len_t)(numeric_limits<len_t>::max() - m_chainLength));

    chain_map newMap;
    for (auto& val : m_map){
        newMap.emplace(n_hash(val.first, m_flavor, amount), val.second);
    }
    m_map = newMap;
    m_chainLength += amount;
}

void SubTable::shorten(len_t amount){
    amount = min(amount, m_chainLength);

    for (auto& val : m_map)
        val.second = n_hash(val.second, m_flavor, amount);

    m_chainLength -= amount;
}

hash_t SubTable::resolve(hash_t query){
	query = query ^ m_flavor;
	chain_map::iterator it;

	hash_t preimage;
	hash_t pointer = query;

	for (uint32_t i = 0; i < this->m_chainLength; i++){
        it = m_map.find(pointer);
		if (it != m_map.end())
		{
			preimage = it->second;
			preimage = n_hash(preimage, m_flavor, m_chainLength - i - 1);

			hash_t temp = preimage;
			temp = n_hash(temp, m_flavor, 1);

			if (temp == query)
				return (preimage ^ m_flavor);
		}
		pointer = n_hash(pointer, m_flavor, 1);
	}
	return FAILURE;
}



