#include "HellmanTable.h"
#include <limits>
#include <sstream>
#include <iostream>
#include <unistd.h>
#define MIN_FILE_INDEX 0
#define MAX_FILE_INDEX 15241

using namespace RaaS;

void HellmanTable::load(const string FileName, const LoadType loadType){
    if(loadType == LoadType::Override)
        m_tables.clear();

	ifstream in(FileName, ios::binary | ios::in);

//    ifstream in("/home/elihu/Development/Tables/Table_0_2000000.bin", ios::binary | ios::in);
	if (in.good())
	{
        hash_t flavor = 0;
        len_t len = 0;
        value_type data[SubTable::BLOCK_SIZE];

		while (in.read((char*)&flavor, sizeof(flavor))){
            in.read((char*)&len, sizeof(len));
            in.read((char*)data, sizeof(data));
            m_tables.emplace_back(len, flavor);
            m_tables.back().load(data);
		}

		m_chainLength = len;
	}

	in.close();
}

void HellmanTable::save(const string FileName){
	int index = 0;
	cout << "# Of Entries written to file: " << m_tables.size() << endl;
	for (auto& val : m_tables) {
		ostringstream os;
		os << "/home/elihu/Development/Tables/New/Table_" << index << ".bin";
		ofstream out(os.str().c_str(), ios::binary | ios::out | ios::trunc);
		out << val;
		out.close();
//		cout << "Wrote index: " << os.str();
		index++;
	}



}

void HellmanTable::addChainBlocksFromFS(count_t amount){

	cout << "Updated number of entries in table before adding: " << m_tables.size() << endl;
	cout << "Loading " << amount << " of tables from the FS" << endl;
	cout << "-- addChainBlocksFromFS 1 --" << endl;
	for (count_t var = 0; var < amount; ++var) {
		ostringstream os;
		os << sFilesPath << "/Table_";
		if(nCurrentFileIndex < 10) {
			os << "0";
		}

		os << nCurrentFileIndex;
		os << ".bin";

		nCurrentFileIndex++;

//		cout << "Selected file: " << os.str() << " to load from the FS" << endl;
		cout << "-- addChainBlocksFromFS 2 --" << endl;
		load(os.str(),LoadType::Append);
		cout << "-- addChainBlocksFromFS 3 --" << endl;
	}

	cout << "Updated number of entries in table: " << m_tables.size() << endl;
}

void HellmanTable::addChainBlocks(count_t amount){
    vector<future<void>> results;

    amount = min(amount, numeric_limits<count_t>::max() - this->getSubTablesCount());

	for (count_t i = 0; i < amount; i++)
		m_tables.emplace_back(SubTable(m_chainLength, i));

    for(auto &tbl : m_tables)
		results.emplace_back(m_threads.enqueue(&SubTable::Init, &tbl));

	for (auto& t : results) t.get();
}

void HellmanTable::removeChainBlocks(count_t amount){
	SubTable* tbl;
	cout << "-- removeChainBlocks 1 --" << endl;
	amount = min(amount, this->getSubTablesCount());


	cout << "Removing blocks to " << m_tables.size() - amount << " from " << m_tables.size() << endl;
	cout << "-- removeChainBlocks 2 --" << endl;
	cout << "BEFORE SLEEP..!" << endl;
	sleep(3);
//	m_tables.erase(m_tables.end() - amount, m_tables.end()-1);

//	m_tables.erase(m_tables.end() - amount, m_tables.end());
	for(int i=0; i < amount; i++) {
		cout << "Popping " << i << endl;
		m_tables.pop_back();
	}

//	sleep(1);
	cout << "AFTER SLEEP..!" << endl;
	nCurrentFileIndex -= amount;
    cout << "-- removeChainBlocks 3 --" << endl;
    cout << "Updated File Index is now: " << nCurrentFileIndex << endl;


}

void HellmanTable::extend(len_t amount){
    vector<future<void>> results;

    amount = min(amount, (len_t)(numeric_limits<len_t>::max() - this->getChainLen()) );

	for (auto& val : m_tables)
		results.emplace_back(m_threads.enqueue(&SubTable::extend, &val, amount));
	for (auto& t : results) t.get();

	m_chainLength += amount;
}

void HellmanTable::shorten(len_t amount){
    vector<future<void>> results;

    amount = min(amount, this->getChainLen());

    for (auto& val : m_tables)
        results.emplace_back(m_threads.enqueue(&SubTable::shorten, &val, amount));
    for (auto& t : results) t.get();

    m_chainLength -= amount;
}

bool HellmanTable::getPreimage(hash_t query, hash_t& preimage){
    vector<future<hash_t>> results;

    cout << "-- getPreimage 1 --" << endl;
	for (auto& val : m_tables)
	{
		cout << "-- getPreimage 2 --" << endl;
		results.emplace_back(m_threads.enqueue(&SubTable::resolve, &val, query));
		cout << "-- getPreimage 3 --" << endl;
	}
	cout << "-- getPreimage 4 --" << endl;
	for (auto& t : results){
		hash_t ret = t.get();
		if (ret != SubTable::FAILURE){
			preimage = ret;
			cout << "-- getPreimage 5 --" << endl;
			return true;
		}
	}
	cout << "-- getPreimage 6 --" << endl;
	return false;
}

