#define NBT_COMPILE
#define NBT_COMPILE_FULL_ARRAYS
#define NBT_IGNORE_MUTF
#define NBT_INCLUDE
#include "../nbt/nbt.hpp"
#define NBT_GZNBT_INCLUDE
#include "../loadgz/gznbt.h"
#include <fstream>
#include <iostream>
using namespace nbt;
using namespace std;
int main(int argc, char* argv[]) {
	compound out = compound("out");

	list test = list("ListTest");
	
	test << new stringtag("Hi", "");
	test << new stringtag("Goodbye", "");
	test << new stringtag("I never thought it had to end like this", "");
	test << new stringtag("\\r", "");
	
	out << &test;
	cout << out["ListTest"][0] << " ~ " << out["ListTest"][0]._string() << " ~ " << out["ListTest"][0]._stringtag() << endl;
	
	// Write nbt file data to buffer
	vector<char> bytes = vector<char>();
	out.write(bytes);
	
	// Deflate file data using GZip + ZLIB
	vector<char> deflated = vector<char>();
	nbt::deflate(&bytes[0], bytes.size(), &deflated, 9);
	
	// Write deflated data to file
	ofstream outf = ofstream("out.nbt", ios::binary);
	outf.write(&deflated[0], deflated.size());
	outf.close();

	// Re-read data from file
	ifstream inf = ifstream("out.nbt", ios::binary);

	// Get size of file, and create buffer with that size
	inf.seekg(0, ios::end);
	vector<char> inBytes = vector<char>(inf.tellg());
	inf.seekg(0, ios::beg);

	// Read data
	inf.read(&inBytes[0], inBytes.size());

	// Inflate data
	vector<char> inflated = vector<char>();
	nbt::inflate(&inBytes[0], inBytes.size(), &inflated);
	
	// Load NBT compound tag from file data
	compound in = compound();
	in.load(&inflated[0], 0);

	// Print the contents of 'in', proving that the data writing, deflation, file writing, file reading, inflation, and data reading were all successful.
	cout << in << endl;

}