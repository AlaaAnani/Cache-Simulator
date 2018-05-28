#include <iostream>
#include  <iomanip>

using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(64*1024)
#define		looper			1000000


enum cacheResType { MISS = 0, HIT = 1 };
enum cachetype { direct = 0, setass = 1, fullyass = 2 };
enum WritePolicy { WriteThrough = 0, WriteBack = 1 };
WritePolicy writepolicy;

int capacitymiss = 0;
int conflictmiss = 0;
int compulsurymiss = 0;
int entrycounter = 0;
int RecentlyUsedRate = 0;
int FullBlocks = 0;
int wordsperblock = 0;
int MissPenalty = 10;
int HitTime = 1;
int writeThroughToMem = 0;
int writeBackToMem = 0;


unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */
unsigned int cache[3][CACHE_SIZE];

char *msg[2] = { "Miss", "Hit" };

unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}


unsigned int memGen1()
{
	static unsigned int addr = 0;
	return (addr++) % (128 * 1024);
}

unsigned int memGen2()
{
	static unsigned int addr = 0;
	return  rand() % (32 * 1024);
}

unsigned int memGen3()
{
	static unsigned int addr = 0;
	return (addr += (64 * 1024)) % (512 * 1024);
}

cacheResType HandleSetAssociative(int blocksperset, int indexValue, int tagValue, int referencetype);

cacheResType HandleDirectMapping(int offsetValue, int indexValue, int tagValue, int numberOfBlocks, int blocksize, int referencetype);

cacheResType HandleFullyAssociative(int blocksperset, int tagValue, int numberOfBlocks, int blocksize, int ReplacementType, int cachesize, int referencetype);

cacheResType CacheSimulator(unsigned int addr, int offsetsize, int indexsize, int blocksperset, cachetype mycache, int cachesize, int numberOfBlocks, int blockSize, int ReplacementType, int referencetype);


int readinfo(cachetype &mycache, int &blocksize, int &cachesize, int &ReplacementType);


int main()
{
	int counter[2] = { 0, 0 };	//0 misscounter, 1 hit counter
	int iter;
	cacheResType r;
	unsigned int addr;
	cachetype mycache;

	int blocksize, cachesize, ReplacementType;


	int blocksperset = readinfo(mycache, blocksize, cachesize, ReplacementType);
	int numberOfBlocks = (cachesize * 1024) / (blocksize);
	int nofsets = numberOfBlocks / blocksperset;
	int indexsize = log2(nofsets);
	int offsetsize = log2(blocksize);
	int tagsize = 32 - offsetsize - indexsize;
	wordsperblock = blocksize / 4;
	int referencetype;
	double AMAT = 0;

	cout << "Cache Simulator\n";

	for (int i = 0; i < 3; i++)
	for (int j = 0; j < CACHE_SIZE; j++)	//initialize everything to empty (-1)
		cache[i][j] = -1;
	// change the number of iterations into a minimum of 1,000,000
	cout << "MEMGEN1\n";
	for (iter = 0; iter<looper; iter++)
	{
		referencetype = iter % 3; //0--> write. 1 & 2 --> Read
		addr = memGen1();
		r = CacheSimulator(addr, offsetsize, indexsize, blocksperset, mycache, cachesize, numberOfBlocks, blocksize, ReplacementType, referencetype);
		//cout << "0x" << setfill('0') << setw(8) << hex << addr << " (" << msg[r] << ")\n";
		counter[r]++;
	}

	cout << dec << counter[0] << " Misses " << dec << counter[1] << " Hits " << endl;
	cout << dec << "Hit Ratio = " << double(counter[1]) / double((counter[0] + counter[1])) << endl;
	cout << compulsurymiss << " compulsory " << capacitymiss << " capacity " << conflictmiss << " conflict miss " << endl;
	AMAT = DBG + double(counter[0] + writeBackToMem + writeThroughToMem) / double(looper) * MissPenalty;
	cout << "AMAT = " << AMAT << endl;
	cout << "---------------------------------------------------" << endl;
	counter[0] = 0; counter[1] = 0; compulsurymiss = 0; capacitymiss = 0; conflictmiss = 0; writeBackToMem = 0; writeThroughToMem = 0;
	addr = 0; RecentlyUsedRate = 0; entrycounter = 0;
	for (int i = 0; i < 3; i++)
	for (int j = 0; j < CACHE_SIZE; j++)	//initialize everything to empty (-1)
		cache[i][j] = -1;
	// change the number of iterations into a minimum of 1,000,000
	cout << "MEMGEN2\n";
	for (iter = 0; iter<looper; iter++)
	{
		referencetype = iter % 3; //0--> write. 1 & 2 --> Read
		addr = memGen2();
		r = CacheSimulator(addr, offsetsize, indexsize, blocksperset, mycache, cachesize, numberOfBlocks, blocksize, ReplacementType, referencetype);
		//cout << "0x" << setfill('0') << setw(8) << hex << addr << " (" << msg[r] << ")\n";
		counter[r]++;
	}

	cout << dec << counter[0] << " Misses " << dec << counter[1] << " Hits " << endl;
	cout << dec << "Hit Ratio = " << double(counter[1]) / double((counter[0] + counter[1])) << endl;

	cout << compulsurymiss << " compulsory " << capacitymiss << " capacity " << conflictmiss << " conflict miss " << endl;
	AMAT = DBG + double(counter[0] + writeBackToMem + writeThroughToMem) / double(looper) * MissPenalty;
	cout << "AMAT = " << AMAT << endl;
	cout << "---------------------------------------------------" << endl;

	counter[0] = 0; counter[1] = 0; compulsurymiss = 0; capacitymiss = 0; conflictmiss = 0; writeBackToMem = 0; writeThroughToMem = 0;
	addr = 0;  RecentlyUsedRate = 0; entrycounter = 0;
	for (int i = 0; i < 3; i++)
	for (int j = 0; j < CACHE_SIZE; j++)	//initialize everything to empty (-1)
		cache[i][j] = -1;
	// change the number of iterations into a minimum of 1,000,000
	cout << "MEMGEN3\n";
	for (iter = 0; iter<looper; iter++)
	{
		referencetype = iter % 3; //0--> write. 1 & 2 --> Read
		addr = memGen3();
		r = CacheSimulator(addr, offsetsize, indexsize, blocksperset, mycache, cachesize, numberOfBlocks, blocksize, ReplacementType, referencetype);
		//cout << "0x" << setfill('0') << setw(8) << hex << addr << " (" << msg[r] << ")\n";
		counter[r]++;
	}

	cout << dec << counter[0] << " Misses " << dec << counter[1] << " Hits " << endl;
	cout << dec << "Hit Ratio = " << double(counter[1]) / double((counter[0] + counter[1])) << endl;

	cout << compulsurymiss << " compulsory " << capacitymiss << " capacity " << conflictmiss << " conflict miss " << endl;
	AMAT = DBG + double(counter[0] + writeBackToMem + writeThroughToMem) / double(looper) * MissPenalty;
	cout << "AMAT = " << AMAT << endl;
	cout << "---------------------------------------------------" << endl;
	system("pause");
	return 0;

}
 

cacheResType HandleSetAssociative(int blocksperset, int indexValue, int tagValue, int referencetype)
{
	if (referencetype != 0)		// READ from cache
	{
		int k;
		for (k = 0; k < blocksperset; k++)								//check if hit
		if (cache[0][blocksperset*indexValue + k] == tagValue)
			return HIT;

		for (k = 0; k < blocksperset; k++)								//check if it's a compulsory miss			
		if (cache[1][blocksperset*indexValue + k] == -1)
		{
			compulsurymiss++;
			cache[0][blocksperset*indexValue + k] = tagValue;
			cache[1][blocksperset*indexValue + k] = entrycounter++;				//VALID
			k = blocksperset + 1;
			return MISS;

		}
																		// check if it's conflict and we need to replace
		if (k == blocksperset)
		{
			capacitymiss++;
			//FIFO
			k = blocksperset*indexValue;
			for (int i = 0; i < blocksperset; i++)
			if (cache[1][k] > cache[1][blocksperset*indexValue + i])
				k = blocksperset*indexValue + i;
			if (writepolicy == WriteBack)
			{
				if (cache[2][k] == 1) //dirty
				{
					writeBackToMem++;
					cache[0][k] = tagValue;
					cache[1][k] = entrycounter++;
					cache[2][k] = -1;				//not dirty anymore
				}
				else
				{
					cache[0][k] = tagValue;
					cache[1][k] = entrycounter++;
				}

			}
			else if (writepolicy == WriteThrough)
			{
				cache[0][k] = tagValue;
				cache[1][k] = entrycounter++;
			}

			return MISS;

		}									
	}
	else						//write to cache
	{
		int k;
		// first check if your write call hits :D		increment writethrough or change the dirty bit
		for (k = 0; k < blocksperset; k++)
		if (cache[0][blocksperset*indexValue + k] == tagValue)
		{
			//change the value of cache location
			if (writepolicy == WriteThrough)
			{
				//access memory at the same instance to change data there
				writeThroughToMem++;
			}
			else if (writepolicy == WriteBack)
			{
				cache[2][blocksperset*indexValue] = 1; //indicate dirty bit
			}
			return HIT;

		}

		// then check if the write call misses :D
		for (k = 0; k < blocksperset; k++)
		if (cache[1][blocksperset*indexValue + k] == -1)
		{
			compulsurymiss++;							//cold start
			cache[0][blocksperset*indexValue + k] = tagValue;
			cache[1][blocksperset*indexValue + k] = entrycounter++;				//VALID
			k = blocksperset + 1;
			return MISS;

		}

		if (k == blocksperset)
		{
			capacitymiss++;
			//FIFO
			k = blocksperset*indexValue;
			for (int i = 0; i < blocksperset; i++)
			if (cache[1][k] > cache[1][blocksperset*indexValue + i])
				k = blocksperset*indexValue + i;
			if (writepolicy == WriteBack)
			{
				if (cache[2][k] == 1) //dirty
				{
					writeBackToMem++;
					cache[0][k] = tagValue;
					cache[1][k] = entrycounter++;
					cache[2][k] = -1;				//not dirty anymore
				}
				else
				{
					cache[0][k] = tagValue;
					cache[1][k] = entrycounter++;
					
				}
			}
			else if (writepolicy == WriteThrough)
			{
				cache[0][k] = tagValue;
				cache[1][k] = entrycounter++;
			}

			
			return MISS;

		}
	}
}
cacheResType HandleDirectMapping(int offsetValue, int indexValue, int tagValue, int numberOfBlocks, int blocksize, int referencetype)
{
	if (referencetype != 0)					//READ DATA FROM CACHE
	{

		if (cache[0][indexValue] == tagValue) // check if it is a hit
			return HIT;

		else if (cache[1][indexValue] == -1)
		{
			compulsurymiss++;
			cache[0][indexValue] = tagValue;
			cache[1][indexValue] = 1;
		}
		// it is not a comp miss; therefore, replace it
		else
		{
			conflictmiss++;
			int k = indexValue;
			if (writepolicy == WriteBack)
			{
				if (cache[2][k] == 1)			//dirty	 bit
				{
					writeBackToMem++;
					cache[0][k] = tagValue;
					cache[2][k] = -1;				//not dirty anymore
				}
				else
				{
					cache[0][k] = tagValue;
				}

			}
			else if (writepolicy == WriteThrough)
			{
				cache[0][k] = tagValue;
			}


			/*											 NO CAPACITY MISSES EXIST IN THE DIRECT MAPPED
			//Check if it is a capacity or conflict miss
			//if there is one empty block in the cache, it is not a capacity miss
			for (int i = 0; i < numberOfBlocks && IsCapacityMiss; i++)
			{
			if (cache[1][i] == -1)
			{
			IsCapacityMiss = false;

			}
			}

			if (IsCapacityMiss == true)
			{
			capacitymiss++;
			}
			else
			{	*/


		}

		return MISS;

	}
	else									// WRITE DATA TO CACHE
	{
		if (cache[0][indexValue] == tagValue)		 //check for hit
		{
			//change the value of cache location
			if (writepolicy == WriteThrough)
			{
				//access memory at the same instance to change data there
				writeThroughToMem++;
			}
			else if (writepolicy == WriteBack)
			{
				cache[2][indexValue] = 1;	//indicate dirty bit
			}
			return HIT;
			
		}
		else if (cache[1][indexValue] == -1)		// check for compulsory miss
		{
			compulsurymiss++;
			cache[0][indexValue] = tagValue;
			cache[1][indexValue] = 1;
		}
		// it is not a comp miss; therefore, replace it
		else
		{
			conflictmiss++;
			int k = indexValue;
			if (writepolicy == WriteBack)
			{
				if (cache[2][k] == 1) //dirty
				{
					writeBackToMem++;
					cache[0][k] = tagValue;
					cache[2][k] = -1;				//not dirty anymore
				}
				else
				{
					cache[0][k] = tagValue;
				}
			}
			else if (writepolicy == WriteThrough)
			{
				cache[0][k] = tagValue;
			}

			return MISS;
		}
		return MISS;
	}
}
cacheResType HandleFullyAssociative(int blocksperset, int tagValue, int numberOfBlocks, int blocksize, int ReplacementType, int cachesize, int referencetype)
{
	int k; 
	int entrycounter2 = 0;

	bool IsCacheFull = (entrycounter >= numberOfBlocks);
	if (referencetype != 0) // READ from cache
	{
		
			for (k = 0; k < numberOfBlocks; k++)
				if (cache[0][k] == tagValue) //if tag is found, return hit
				{
					if (ReplacementType == 2) //MRU
						cache[1][k] = RecentlyUsedRate++;
					return HIT;
				}
			//tag is not found, load into the first empty block found
			if (!IsCacheFull)
			{
			compulsurymiss++;
			entrycounter2++;
			cache[0][entrycounter] = tagValue;
			if (ReplacementType == 2) //MRU
			{
				cache[1][k] = 0;//complusory miss, first time to load it. means not used before
				entrycounter++;
			}
			else
			cache[1][entrycounter] = entrycounter++;				//valid
			return MISS;

		}
		else //cache is full, replacement will occur
		{
			entrycounter2 = 0;
			if (ReplacementType == 0) // FIFO
			{
				capacitymiss++;

				k = 0;
				for (int i = 0; i < numberOfBlocks; i++)
				if (cache[1][k] > cache[1][i])//look for block with the minimum entrycounter (First loaded block into chache)
					k = i;
				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}
				cache[0][k] = tagValue; //replace
				cache[1][k] = entrycounter++;				//valid
				entrycounter2++;
				return MISS;



			} //FIFO END

			else if (ReplacementType == 1) // LIFO
			{
				capacitymiss++;

				k = 0;
				for (int i = 0; i < numberOfBlocks; i++)
				if (cache[1][k] < cache[1][i])//look for block with the minimum entrycounter (First loaded block into chache)
					k = i;
				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}

				cache[0][k] = tagValue; //replace
				cache[1][k] = entrycounter++;				//valid
				entrycounter2++;
				return MISS;



			}//LIFO END
			else if (ReplacementType == 2) // MRU
			{
				capacitymiss++;
				entrycounter++;
				entrycounter2++;

				k = 0;
				for (int i = 0; i < numberOfBlocks; i++)
				if (cache[1][k] < cache[1][i])
					k =  i;
				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}
				cache[0][k] = tagValue;
				cache[1][k] = 0;				//rate is zero since it is not recently used (first load)


				return MISS;

			} //MRU END
			else if (ReplacementType == 3) // RANDOM
			{
				capacitymiss++;

				unsigned int RandomAddress = rand_() % cachesize;



				if (writepolicy == WriteBack)
				{
					if (cache[2][RandomAddress] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][RandomAddress] = -1; //not dirty anymore
					}
				}

				cache[0][RandomAddress] = tagValue;
				cache[1][RandomAddress] = entrycounter++;				//valid
				entrycounter2++;
				return MISS;
			}

		}
	}
	else			//write to cache
	{
		for (k = 0; k < numberOfBlocks; k++)
		if (cache[0][k] == tagValue) //if tag is found, return hit
		{
			if (writepolicy == WriteThrough)
				writeThroughToMem++;
			else if (writepolicy == WriteBack)
				cache[2][k] = 1;		//dirty bit
			if (ReplacementType == 2) //MRU
				cache[1][k] = RecentlyUsedRate++;
			return HIT;
		}
		if (!IsCacheFull)
		{
			compulsurymiss++;
			cache[0][entrycounter] = tagValue;
			cache[1][entrycounter] = entrycounter++;	//valid
		}
		//cache is full
		else
		{
			if (ReplacementType == 0) // FIFO
			{
				capacitymiss++;

				k = 0;
				for (int i = 0; i < numberOfBlocks; i++)
				if (cache[1][k] > cache[1][i])//look for block with the minimum entrycounter (First loaded block into chache)
					k = i;
				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}
				cache[0][k] = tagValue; //replace
				cache[1][k] = entrycounter++;				//valid
				return MISS;



			} //FIFO END

			else if (ReplacementType == 1) // LIFO
			{
				capacitymiss++;

				k = 0;
				for (int i = 0; i < numberOfBlocks; i++)
				if (cache[1][k] < cache[1][i])//look for block with the minimum entrycounter (First loaded block into chache)
					k = i;
				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}

				cache[0][k] = tagValue; //replace
				cache[1][k] = entrycounter++;				//valid
				return MISS;



			}//LIFO END
			else if (ReplacementType == 2) // MRU
			{
				capacitymiss++;
				entrycounter++;

				k = 0;
				for (int i = 0; i < numberOfBlocks; i++)
				if (cache[1][k] < cache[1][i])
					k = i;
				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}
				cache[0][k] = tagValue;
				cache[1][k] = 0;				//rate is zero since it is not recently used (first load)


				return MISS;

			} //MRU END
			else if (ReplacementType == 3) // RANDOM
			{
				capacitymiss++;

				unsigned int RandomAddress = rand_() % cachesize;



				if (writepolicy == WriteBack)
				{
					if (cache[2][k] != -1) //dirty
					{
						writeBackToMem++;
						cache[2][k] = -1; //not dirty anymore
					}
				}

				cache[0][RandomAddress] = tagValue;
				cache[1][RandomAddress] = entrycounter++;				//valid




				return MISS;
			}

		}
		return MISS;
	}
}
cacheResType CacheSimulator(unsigned int addr, int offsetsize, int indexsize, int blocksperset, cachetype mycache, int cachesize, int numberOfBlocks, int blockSize, int ReplacementType, int referencetype)
{
	// This function accepts the memory address for the read/write and returns whether it caused a cache miss or a cache hit
	// The current implementation assumes there is no cache; so, every transaction is a miss

	cacheResType flag = MISS;
	unsigned int offsetValue = addr%int(pow(2,offsetsize));
	unsigned int indexValue;
	if (mycache != fullyass)
		indexValue = (addr >> offsetsize) % int(pow(2,indexsize));
	unsigned int tagValue = (addr >> (offsetsize + indexsize));
	if (mycache == direct)
	{
		flag = HandleDirectMapping(offsetValue, indexValue, tagValue, numberOfBlocks, blockSize, referencetype);
	}

	if (mycache == setass)
	{

		flag = HandleSetAssociative(blocksperset, indexValue, tagValue, referencetype);

	}


	if (mycache == fullyass)
	{
		//index value will be zero, but it's passed anyway. blocksperset will be all blocks
		flag = HandleFullyAssociative(blocksperset, tagValue, numberOfBlocks, blockSize, ReplacementType, cachesize, referencetype);
	}



	return flag;
}

int readinfo(cachetype &mycache, int &blocksize, int &cachesize, int& ReplacementType)
{
	int blocksperset;
	int c;
	do
	{
		cout << "Please enter the type of the cache ==> 0 for directmapped,1 for set-associative, 2 for fully associative:" << endl;
		cin >> c;
	} while ((c != 0) && (c != 1) && (c != 2));
	mycache = cachetype(c);
	do
	{
		cout << "Please enter the size of the block as a power of 2 between 4 bytes to 128 bytes" << endl;
		cin >> blocksize;
	} while ((blocksize<4) || (blocksize>128) || (log2(blocksize) - floor(log2(blocksize)) != 0));
	do
	{
		cout << "please enter the size of the cache as a power of 2 between 2KB to 64KB" << endl;
		cin >> cachesize;
	} while ((cachesize<2) || (cachesize>64) || (log2(cachesize) - floor(log2(cachesize)) != 0));
	do
	{
		cout << "Enter write policy. 0--> Write through. 1--> Write back." << endl;
		cin >> c;
	} while ((c != 0) && (c != 1));
	writepolicy = WritePolicy(c);
	if (mycache == setass)
	{
		do
		{
			cout << "Please enter the number of blocks per set as a power of 2 between 2 to 16" << endl;
			cin >> blocksperset;
		} while ((blocksperset<2) || (blocksperset>16) || (log2(blocksperset) - floor(log2(blocksperset)) != 0));

	}
	if (mycache == direct)
		blocksperset = 1;
	if (mycache == fullyass)
	{
		blocksperset = (cachesize * 1024) / blocksize;
		do
		{
			cout << "Please enter replacement policy: 0->FIFO, 1->LIFO, 2->MRU, 3->Random" << endl;
			cin >> ReplacementType;
		} while ((ReplacementType != 0) && (ReplacementType != 1) && (ReplacementType != 2) && (ReplacementType != 3));
	}
	return blocksperset;
}




