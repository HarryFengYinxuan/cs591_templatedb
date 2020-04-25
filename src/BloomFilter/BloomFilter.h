#include <vector>
#include <string>
#include <math.h>

using namespace std;

namespace BF{

class BloomFilter {
public:
	BloomFilter();
	BloomFilter( int numElement_, int bitsPerElement_ );
	// those parameters affect the filter vector
	int numElement;
	int bitsPerElement;

	// writing key
	void program(string key);
	// estimating membership of key
	bool query(string key);

	int getIndexNum();
	int getSize();
private:
	int numIndex;
	int size;
	vector< bool > bf_vec;

	void makeBloomFilter();
	void getIndex( string key, vector<int>* index );
};

} // namespace BF
