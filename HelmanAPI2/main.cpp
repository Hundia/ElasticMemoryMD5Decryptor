#include "HellmanTable.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <malloc.h>
#include <sstream>
#include <stdexcept>

using namespace std::chrono;
using namespace RaaS;

//Global Variables (bad programing, but more readable and undertandable)
const string INVALID_PARAM_MSG = "Invalid parameter, try \"help\" for more information";
const len_t DEFAULT_CHAIN_LENGTH = 1000; //Default length of chains.
const uint64_t SAFTY_GAP = 2048;         // = 2MB
const uint64_t BASE_MEMORY = 20240;      // = 20MB
uint64_t CURRENT_MEMORY = BASE_MEMORY;   //Current available memory
double RATIO = 0.00747881;                            //[Blocks per KB]
const uint32_t SIZE_OF_TABLE_IN_KB = 16;      // = 16390B

////////////////////////////////////////////////////////////////////////
//HellmanTable ourTable(DEFAULT_CHAIN_LENGTH,"./Tables");

HellmanTable* ourTable = NULL;

void init(char* sPathToTables) {
	std::string* s = new std::string(sPathToTables);
	ourTable = new HellmanTable(DEFAULT_CHAIN_LENGTH, *s);
}

//Returns resident set size of current process
uint64_t memusage(){
    char line[128];
    /* Report memory usage from /proc/self/status interface. */
    FILE* status = fopen("/proc/self/status", "r");
    if (!status)
        return 0;

    long long unsigned VmRSS = 0;

    while (!feof(status)) {
        fgets(line, 128, status);
        if (strncmp(line, "VmRSS:", 6) == 0){
            if (sscanf(line, "VmRSS: ""%llu", &VmRSS) != 1) {
                fclose(status);
                return 0;
            }

            break;
        }
    }

    fclose(status);
    return VmRSS;
}




//Couputes preimage for a given query.
hash_t HandleTest(hash_t query){
	//	Agreed value for primage not found
	hash_t preimage = 0;
	cout << "-- HandleTest 1 --" << endl;
	cout << "Starting preimage computation for : " << query << endl;
	if (ourTable->getPreimage(query, preimage))
		cout << "Preimage Found! : " << preimage << endl;
	else
		cout << "Preimage was not found :(" << endl;
	cout << "-- HandleTest 2 --" << endl;
	return preimage;
}

//Clibrate the memory to chain block ratio (Extremely slow, pretty accurate).
void Calibrate(HellmanTable &table){
	const count_t repeat        = 2;            //Number of tries each step
	const int step_count        = 3;            //Number of overall calibration steps.
	int step_sizes[step_count]  = {20, 30, 40}; //Number of blocks each step
    const count_t base          = 0;            //Base number of blocks.

    uint64_t sum = 0, temp = 0, total_size = 0;

    for(int i = 0; i < step_count; i++)
        total_size += step_sizes[i];
    total_size *= repeat;

    //Add base amount of blocks
    table.addChainBlocks(base);
    malloc_trim(0);

    //Step calibrattion
    for(int step = 0; step < step_count; step++)
    {
        for(count_t i = 0; i < repeat; i++){
            malloc_trim(0);
            temp = memusage();
            table.addChainBlocks(step_sizes[step]);
            sum += (memusage() - temp);
            malloc_trim(0);
        }
    }

	//Remove added blocks.
    table.removeChainBlocks(base + total_size);
    malloc_trim(0);

    //Use the avrage ratio.
    RATIO = (double)total_size / (double)sum;
    cout << "Calibrate successfully, Ratio = " << RATIO << "[blocks/KB]" << endl;
}

//Shows information about Hellman Table and system performance.
void HandleMonitor(HellmanTable &table){
	cout << "Table Information:" << endl;
	cout << "Chains Count           : " << table.getChainCount() << endl;
	cout << "Chains Length          : " << table.getChainLen() << endl;
	cout << "Sub Tables Count       : " << table.getSubTablesCount() << endl;
	cout << "Current Memory Avai.   : " << CURRENT_MEMORY << "(KB)" << endl;
    cout << "Memmory Usage in Use   : " << memusage() << "(KB)" << endl;
	cout << "Memmory Usage          : " << (double)memusage() / CURRENT_MEMORY * 100 << "%" << endl;
    cout << "Blocks per KB ratio     : " <<  RATIO << endl;
	cout << endl;
}

//Add chains in order to fill the current available memory.
void HandleFill(HellmanTable &table){
	cout << "Filling Table..." << endl;
    uint64_t temp = memusage();
    count_t amount = max(0.0 , floor(RATIO * (int64_t)(CURRENT_MEMORY - temp - SAFTY_GAP)));
	table.addChainBlocks(amount);
    malloc_trim(0);
}

//Computes preimage for a random value.
void HandleRand(HellmanTable &table){
	int succCount = 0;
	int failCount = 0;
	for(int i=0;i<60;i++) {
		hash_t preimage, query;
			query = gen_input(rand());

			cout << "Starting preimage computation for : " << query << "\n";
			if (table.getPreimage(query, preimage)) {
//				cout << "Preimage Found! : " << preimage << endl;

				succCount++;
			}
			else {
				failCount++;
			}
				//cout << "Preimage was not found :(" << endl;
	}

	cout << "# of Success: " << succCount << " # of Fail: " << failCount << endl;
	cout << "Success rate: " << (float)succCount / 100.0;

}

//Couputes preimage for a given query.
void HandleTest(HellmanTable &table, istringstream& params){
	hash_t query, preimage;
	if(!(params >> query))
        throw invalid_argument(INVALID_PARAM_MSG);

	cout << "Starting preimage computation for : " << query << endl;
	if (table.getPreimage(query, preimage))
		cout << "Preimage Found! : " << preimage << endl;
	else
		cout << "Preimage was not found :(" << endl;
}

//Calculate success rate for a given range of tries.
void HandleRate(HellmanTable &table, istringstream& params){
    uint32_t range;
    if(!(params >> range))
        throw invalid_argument(INVALID_PARAM_MSG);

	hash_t preimage, query;
	uint32_t succ = 0;
	cout << "Starting success rate computation <" << range << ">" << endl;
	for (int i = 0; i < range; i++){
		query = gen_input(rand());
		if (table.getPreimage(query, preimage))
			succ++;
	}
	cout << "Success Rate : " << succ << "/" << range << "(" << (double)succ / range * 100.0 << "%" << ")" << endl;
}

//Extends all of the chains by a  specific amount.
void HandleExtend(HellmanTable &table, istringstream& params){
    len_t amount;
    if(!(params >> amount))
        throw invalid_argument(INVALID_PARAM_MSG);

	amount = min<len_t>(amount, 65536 - table.getChainLen());
	cout << "Starting extension process, amount = " << amount << endl;
	table.extend(amount);
	cout << "Extension finished successfully" << endl;
}

void Load(HellmanTable &table) {
	//
	table.load("./CurrentTables.bin",LoadType::Append);
}
//Shortens all of the chains by a specific amount.
void HandleShorten(HellmanTable &table, istringstream& params){
    len_t amount;
    if(!(params >> amount))
        throw invalid_argument(INVALID_PARAM_MSG);

	amount = min<len_t>(amount, table.getChainLen());
	cout << "Starting shotening process, amount = " << amount << endl;
	table.shorten(amount);
	cout << "Shortening finished successfully" << endl;
}

void HandleRec(HellmanTable &table, string str) {
	table.save(str.c_str());
}
//Prints representative chain information.
void HandleChain(HellmanTable &table){
    if(table.getChainCount() == 0){
        cerr << "No chains available yet, try \"help\" for more information";
        return;
    }
	cout << "chain example : " << endl;
	chain_stat stats;
	hash_t flavor;
	stats = table.getStats();

	hash_t ptr = stats.SP;
	cout << "Body : ";
	while (ptr != stats.EP){
		cout << ptr << " -> ";
		ptr = n_hash(ptr, flavor, 1);
	}
	cout << stats.EP << endl;
	cout << "Flavor = " << stats.flavor << endl;
	cout << "Length = " << stats.length << endl;
}

//Adds a certain amount of memory (KB) and fill it with chains accordingly.
void HandleAddFromFS(uint64_t mem_amount){
//    uint64_t mem_amount;
//    if(!(params >> mem_amount))
//        throw invalid_argument(INVALID_PARAM_MSG);
//
//    cout << "Handling loading from FS:" << endl;
//    CURRENT_MEMORY += mem_amount;
//    //uint64_t temp = memusage();
//    count_t amount;
//    if(mem_amount < SIZE_OF_TABLE_IN_KB)
//    	amount = SIZE_OF_TABLE_IN_KB;
//    else
//    	amount = floor(mem_amount / SIZE_OF_TABLE_IN_KB);

//    uint64_t mem_amount;
//    if(!(unMemoryInKb >> mem_amount))
//        throw invalid_argument(INVALID_PARAM_MSG);

    CURRENT_MEMORY += mem_amount;
    cout << "-- HandleAddFromFS 1 --" << endl;
    uint64_t temp = memusage();
    cout << "memusage: " << temp << endl;
    count_t amount = max(0.0 , floor(RATIO * (int64_t)(CURRENT_MEMORY - temp - SAFTY_GAP)));
    cout << "-- HandleAddFromFS 2 --" << endl;
    //cout << " Calculated: " << mem_amount << " / " << SIZE_OF_TABLE_IN_KB << " = " << amount << endl;
//    cout << "Adding chain blocks from FS, amount = " << amount << endl;
    ourTable->addChainBlocksFromFS(amount);
    cout << "-- HandleAddFromFS 3 --" << endl;
	malloc_trim(0);
	cout << "-- HandleAddFromFS 4 --" << endl;
	RATIO = (RATIO +((double)amount / (memusage() - temp))) / 2;
	cout << "-- HandleAddFromFS 3 --" << endl;

}

//Adds a certain amount of memory (KB) and fill it with chains accordingly.
void HandleAddFromFS(HellmanTable &table, istringstream& params){
//    uint64_t mem_amount;
//    if(!(params >> mem_amount))
//        throw invalid_argument(INVALID_PARAM_MSG);
//
//    cout << "Handling loading from FS:" << endl;
//    CURRENT_MEMORY += mem_amount;
//    //uint64_t temp = memusage();
//    count_t amount;
//    if(mem_amount < SIZE_OF_TABLE_IN_KB)
//    	amount = SIZE_OF_TABLE_IN_KB;
//    else
//    	amount = floor(mem_amount / SIZE_OF_TABLE_IN_KB);

    uint64_t mem_amount;
    if(!(params >> mem_amount))
        throw invalid_argument(INVALID_PARAM_MSG);

    CURRENT_MEMORY += mem_amount;
    uint64_t temp = memusage();
    cout << "memusage: " << temp << endl;
    count_t amount = max(0.0 , floor(RATIO * (int64_t)(CURRENT_MEMORY - temp - SAFTY_GAP)));

    //cout << " Calculated: " << mem_amount << " / " << SIZE_OF_TABLE_IN_KB << " = " << amount << endl;
//    cout << "Adding chain blocks from FS, amount = " << amount << endl;
	table.addChainBlocksFromFS(amount);
	malloc_trim(0);
	RATIO = (RATIO +((double)amount / (memusage() - temp))) / 2;

}

//Adds a certain amount of memory (KB) and fill it with chains accordingly.
void HandleAdd(HellmanTable &table, istringstream& params){
    uint64_t mem_amount;
    if(!(params >> mem_amount))
        throw invalid_argument(INVALID_PARAM_MSG);

    CURRENT_MEMORY += mem_amount;
    uint64_t temp = memusage();
    count_t amount = max(0.0 , floor(RATIO * (int64_t)(CURRENT_MEMORY - temp - SAFTY_GAP)));
    cout << "Adding chain blocks, amount = " << amount << endl;
	table.addChainBlocks(amount);
	malloc_trim(0);
    //RATIO = (RATIO +((double)amount / (memusage() - temp))) / 2;
}

void PrepareTablesToDisk(HellmanTable &table) {
	static int recIndex = 0;

	std::ostringstream os;
	os << "./Tables/Table_" << recIndex << ".bin";
	HandleRec(table,os.str());
	return;
    for (int var = 0; var < 50; ++var) {
    	HellmanTable table2(DEFAULT_CHAIN_LENGTH);
    	table2.addChainBlocks(100);
    	cout << "Recording table index: " << recIndex << " Length: " << table2.getStats().length << endl;
    	//malloc_trim(0);
    	std::ostringstream os;
    	os << "./Tables/Table_" << recIndex << ".bin";
    	HandleRec(table2,os.str());
    	//table.removeChainBlocks(100);
    	recIndex++;
    	malloc_trim(0);
	}
}

//Removes a certain amount of memory (KB) and delete chains accordingly.
void HandleRemove(HellmanTable &table, istringstream& params){
    uint64_t mem_amount;

    if(!(params >> mem_amount))
        throw invalid_argument(INVALID_PARAM_MSG);

    cout << " TEST..!" << endl;
	//cin >> mem_amount;
    CURRENT_MEMORY -= mem_amount;
	uint64_t amount = ceil(RATIO * mem_amount);
	cout << "Removing chain blocks, amount = " << amount << endl;
	table.removeChainBlocks(amount);
	malloc_trim(0);
}

//Removes a certain amount of memory (KB) and delete chains accordingly.
void HandleRemove(uint64_t mem_amount){
//    uint64_t mem_amount;
//    if(!(params >> mem_amount))
//        throw invalid_argument(INVALID_PARAM_MSG);

	cout << "-- HandleRemove 1 --" << endl;
	if(CURRENT_MEMORY - mem_amount < BASE_MEMORY) {
		mem_amount = CURRENT_MEMORY - BASE_MEMORY;
	}

	CURRENT_MEMORY -= mem_amount;
	//cin >> mem_amount;

	uint64_t amount = ceil(RATIO * mem_amount);
	cout << "Removing chain blocks, amount = " << amount << endl;
	cout << "-- HandleRemove 2 --" << endl;
	ourTable->removeChainBlocks(amount);
	cout << "-- HandleRemove 3 --" << endl;
	malloc_trim(0);
	cout << "-- HandleRemove 4 --" << endl;
}

//Show UI instructions
void HandleHelp(){
    cout << "cal                : Clibrate the memory to chain block ratio." << endl;
    cout << "monitor            : Shows information about Hellman Table and system performance." << endl;
    cout << "fill               : Add chains in order to fill the current available memory." << endl;
    cout << "rand               : Computes preimage for a random value." << endl;
    cout << "test <query>       : Couputes preimage for a given query." << endl;
    cout << "rate <range>       : Calculate success rate for a given range of tries." << endl;
    cout << "extend <amount>    : Extends all of the chains by a  specific amount." << endl;
    cout << "shorten <amount>   : Shortens all of the chains by a specific amount." << endl;
    cout << "chain              : Prints representative chain information." << endl;
    cout << "add <amount>       : Adds a certain amount of memory (KB) and fill it with chains accordingly." << endl;
    cout << "addfs <amount>       : Adds a certain amount of memory (KB) from FS and fill it with chains accordingly." << endl;
    cout << "remove <amount>    : Removes a certain amount of memory (KB) and delete chains accordingly." << endl;
    cout << "exit               : Closes the application" << endl;
}


int main(){
    srand(0);
    HellmanTable table(DEFAULT_CHAIN_LENGTH,"/home/elihu/Development/Tables/New");

    cout << "Calibrating..." << endl;
//    Calibrate(table);
	cout << endl << "Welcome!" << endl << "Please insert command" << endl << endl;

	bool active = true;
	string line, cmd;

	while (active){
		getline(cin, line);

		if (line == "")
			continue;

        istringstream iss(line);

		cout << "//////////////////////////////////////" << endl;
        iss >> cmd;

		auto start = steady_clock::now();

		try{

            if(cmd == "?" || cmd == "help"){HandleHelp();
            } else if (cmd == "cal"){       Calibrate(table);
            } else if (cmd == "monitor"){   HandleMonitor(table);
            } else if (cmd == "fill"){      HandleFill(table);
            } else if (cmd == "add"){       HandleAdd(table, iss);
            } else if (cmd== "addfs"){      HandleAddFromFS(table, iss);
            } else if (cmd == "remove"){    HandleRemove(table, iss);
            } else if (cmd == "extend"){    HandleExtend(table, iss);
            } else if (cmd == "shorten"){   HandleShorten(table, iss);
            } else if (cmd == "test"){      HandleTest(table, iss);
            } else if (cmd == "rand"){      HandleRand(table);
            } else if (cmd == "rate"){      HandleRate(table, iss);
            } else if (cmd == "chain"){     HandleChain(table);
            } else if (cmd == "rec"){     	HandleRec(table,"path");
            } else if (cmd== "load"){       Load(table);
            } else if (cmd== "prep"){       PrepareTablesToDisk(table);
            } else if (cmd == "exit"){      active = false;
            } else{
                cout << "Unsupporeted command..." << endl;
            }

		}
		catch (const exception& e){
            cerr << e.what() << endl;
		}

		auto elapsed = steady_clock::now() - start;
		cout << "Duration (seconds) : " << double(elapsed.count()) * steady_clock::period::num / steady_clock::period::den << endl;
		cout << "//////////////////////////////////////" << endl << endl;
	}

}

