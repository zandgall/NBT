/*

THE CURRENT CODE IS NOT FULLY COMMENTED. There are a few comments here and there, but it is very much a work in progress. Enter at your own risk of confusion!

This is a header only library, that defines the NBT data system. 
Each tag type is defined in a class, as a child of the nbt::tag super class. 

Every tag type holds an ID, a name, and a unique data type, which is given by the the name of the tag. (Byte tags store bytes, String tags store strings, etc.)
You may provide a tag with NBT file data in order to be interpretted into the proper data types. You may also push NBT file data into a buffer.

Strings in NBT use the Java Modified-UTF-8 codec. Don't ask me why, NBT was originally developed in Java, so they must use it and make life suck for the rest of us I suppose.

An NBT Compound tag may be used, just like a JSON object, to store various primitive types in the form of other NBT tags.
The Compound tag is the first place to start.

 - Zander

- Video essay on the specifics of NBT - https://youtu.be/12PAtF2Ih_c
- Wiki.vg documentation on NBT - https://wiki.vg/NBT
- Original NBT specification (archive) - https://web.archive.org/web/20110723210920/http://www.minecraft.net/docs/NBT.txt


Use NBT_INCLUDE only ONCE in your project, in order to define otherwise undefined functions.

Preprocessor options:
NBT_COMPILE - Allows access to 'compilation' functions, that return a string detailing the contents of a tag. Helpful for debugging.
NBT_COMPILE_FULL_ARRAYS - By default, any array tags will say "ArrayTag(100 elements)" instead of listing each element individually. If you wish to see each element, define this.
NBT_LITTLE_ENDIAN - Uses Little Endian to represent data, rather than the default Big Endian
NBT_SHORTHAND - In order to interact with different forms of data, tag_p will dynamic_cast from a tag pointer, to a specific tag reference. Shorthand adds extra shorter functions to allow you to call "tag_p.i()" or "tag_p.it()" instead of "tag_p._int()" or "tag_p._inttag()"
NBT_THROW_ENDLESS - Enables an exception to be thrown whenever a compound tag attempts to write it's data when it doesn't have an end tag.
NBT_IGNORE_MUTF - Ignores the "Modified UTF-8" specification, and instead only deals in the base UTF-8 standard, default C++ string.
NBT_INCLUDE - Required on first include.
*/

#pragma once

#include <vector>
#include <map>
#include <exception>
#include <typeinfo>
#include <string>
#include <iostream>

// Codecvt is a deprecated standard header. However, there was no equivalant given in the standard library that can convert a 8-bit string (const char*), to a 32-bit string (char32_t*). 
#ifndef NBT_IGNORE_MUTF
	#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
	#error NBT Makes use of <codecvt> in order to properly decode and encode Java Modified UTF-8. In order to make use of this, you will need to define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING before any STL includes, or in the debug declarations of your project. You may also #define NBT_IGNORE_MUTF instead to use default UTF-8 Text encodings.
	#endif
	#include <codecvt>
#endif

/*
All NBT tags are required to contain a name in order to be stored by a Compound Tag.
End Tags do not read a given file's contents for a name, so instead they use this placeholder name.
4 '255' character values are provided, in order to sort compound tags properly so that end tags appear at the end of the given list.
*/
constexpr const char* NBT_END_TAG_NAME = "\\NBT_END_TAG_NAME_CONSTANT";

/* 
The NBT_BYPASS_ID is a flag that always passes any ID checks. 
A tag will check that the ID it is loading, is the correct ID for it's type.

(i.e, bytetags will check for ID '1' in the file, and if it isn't 1, throws invalid_tag_id_exception)
*/
constexpr int8_t NBT_BYPASS_ID = 127;

namespace nbt {
	// Called if a tag reads data, and the ID it reads is incorrect
	// i.e: bytetag, which has an ID of '1', reading data that returns the ID 2, will throw invalid_tag_id_exception
	class invalid_tag_id_exception : public std::exception {
	public:
		char id, required_type;
		invalid_tag_id_exception(char id, char required_type) : exception() {
			this->id = id;
			this->required_type = required_type;
			error = ("Invalid tag ID exception. Tag ID read (" + std::to_string((int)id) + ") isn't the same as the Tag that's reading it! (" + std::to_string((int)required_type) + ")");
		}
		const char* what() const throw() {
			return error.c_str();
		}
	private:
		std::string error;
	};
	// Called if a list tries to load a tag that doesn't have the ID that the list tag uses.
	// i.e: (list of byte tags) bytelist.add(new inttag(10));
	class illegal_list_tag_type : public std::exception {
	public:
		char type;
		illegal_list_tag_type(char type) {
			this->type = type;
			this->error = ("List tag cannot contain tags of this type! (" + std::to_string((int)type) + ")");
		}
		const char* what() {
			return error.c_str();
		}
	private:
		std::string error;
	};
	// Called if a compound or list tag attempts to load a tag with an ID that isn't present.
	// i.e: the default tag ids (of this library) range from -12 to positive 12
	class missing_tag_id_exception : public std::exception {
	public:
		char id;
		missing_tag_id_exception(char id) {
			this->id = id;
			this->error = ("Tried to load tag, but didn't find a valid tag, intead got " + std::to_string((int)id));
		}
		const char* what() {
			return error.c_str();
		}
	private:
		std::string error;
	};
	// Only enabled through #define NBT_THROW_ENDLESS
#ifdef NBT_THROW_ENDLESS
	// Called if a compound tag attempts to write its data, when it does not have a comppound tag.
	// i.e: compound tag = compound(); tag.add(new inttag(0)); tag.write(...); (Tag has no end tag, thus is 'endless')
	class endless_compound_exception : public std::exception {
	public:
		endless_compound_exception() {
			error = "No end associated with this compound tag!";
		}
		const char* what() {
			return error.c_str();
		}
	private:
		std::string error;
	};
#endif
	// Thrown when interacting with a tag_p, and attempting to convert its data to a type it is not.
	// i.e: tag_p t = tag_p(new inttag(10)); t._byte(); (attempted to cast to byte tag, invalid tag operator)
	class invalid_tag_operator : public std::exception {
	public:
		char id, required_type;
		invalid_tag_operator(char id, char required_type) : exception() {
			this->id = id;
			this->required_type = required_type;
			error = ("Invalid tag operation exception. Attempted to perform operator on type (" + std::to_string((int)id) + ") when the required type for this operator is (" + std::to_string((int)required_type) + ") Check your tag_p usages!");
		}
		const char* what() const throw() {
			return error.c_str();
		}
	private:
		std::string error;
	};

	// Used to grab the byte-data of any T element. Defaults to Big Endian, however can be configured to use little endian
	template <typename T>
	int toBytes(const T in, char* const out) {
		try {
			memcpy_s(out, sizeof(T), (void*)&in, sizeof(T));

#ifndef NBT_LITTLE_ENDIAN
			for (char i = 0; i < sizeof(T) / 2; i++)
				std::swap(out[i], out[sizeof(T) - 1 - i]);
#endif
		}
		catch (std::exception e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}
		return 0;
	}

	// Used to cast the binary data of any T object, into a T object. 
	template <typename T>
	int fromBytes(const char* const in, T* const out) {
		try {
			memcpy_s(out, sizeof(T), in, sizeof(T));

#ifndef NBT_LITTLE_ENDIAN
			for (char i = 0; i < sizeof(T) / 2; i++)
				std::swap(((char*)(out))[i], ((char*)(out))[sizeof(T) - 1 - i]);
#endif
		}
		catch (std::exception e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}
		return 0;
	}

	// Convert a regular utf-8 string into a Java Modified-UTF-8 string
	extern std::string utfToMutf(std::string utf);
	// Convert a Java Modified-UTF-8 string into a regular utf-8 string
	extern std::string mutfToUtf(std::string mutf);

	// Finally, the class that specifies functions and data used by all types of tags.
	class tag {
	public:
		// The NBT id of the tag
		int8_t id = -1;

		// The name of the tag
		std::string name = "";

		/// <summary>
		/// Load data to this tag from the list of bytes, starting at the given offset. Used majorly by nbt::compound tags to load sub-tags
		/// </summary>
		/// <param name="bytes">- List of NBT data. Equivelent to a decompressed .dat file.</param>
		/// <param name="offset">- The position to start reading data from</param>
		/// <returns>Where the current tag's data ends, and the next tag's data begins.</returns>
		virtual size_t load(const char* const bytes, size_t offset = 0) = 0;
		/// <summary>
		/// Writes tag's data to an output buffer. Buffer is able to then be saved to a file or loaded by other tags.
		/// </summary>
		/// <param name="buffer">- Where the tag's data will be written to, make sure it has appropriate space. If given nullptr, this function can be used to get the exact length required for a buffer (FASTER+SAFER TO USE WRITE WITH A LIST INSTEAD OF CHAR ARRAY)</param>
		/// <param name="offset">- Where the tag should start writing data</param>
		/// <returns>Where the current tag's data ends, and the next tag's data should begin</returns>
		virtual size_t write(char* const buffer, size_t offset) = 0;
		/// <summary>
		/// Writes tag's data to an extendable output buffer. Buffer is able to then be saved to a file or loaded by other tags.
		/// </summary>
		/// <param name="buffer">- Where the tag's data will be written to, pushing back the end of the buffer.</param>
		/// <returns>Where the current tag's data ends, and the next tag's data should begin</returns>
		virtual size_t write(std::vector<char>& buffer) = 0;
		/// <summary>
		/// Get a vector of chars that represent the data held by this specific tag.
		/// i.e: An inttag will return chars that represent an int.
		/// </summary>
		virtual std::vector<char> value_bytes() = 0;
		/// <summary>
		/// Clear the tag's data, always called by the destructor
		/// </summary>
		virtual void discard() = 0;
		/// <summary>
		/// Returns the correct ID for any tag subclass, used in default write/load functions
		/// </summary>
		virtual const int8_t correct_tag() = 0;
#ifdef NBT_COMPILE
		virtual std::string compilation(std::string regex = "") = 0;
		friend std::ostream& operator<< (std::ostream& left, tag* right) {
			left << right->compilation();
			return left;
		}
#endif

	protected:
		// Writes the default header to a buffer at a given offset. Returns the index for the end of a header. Every tag (except for end tags!) use the default header.
		size_t writeDefault(char* const buffer, size_t offset) {
			if (buffer == nullptr)
				return offset + 3 + name.length();
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			uint16_t namelength = (uint16_t)name.length();
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			memcpy_s(&buffer[offset + 3], namelength, name.data(), namelength);
			return offset + 3 + namelength;
		}
		// Writes the default header, to a vector.
		size_t writeDefault(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			std::string mutf = utfToMutf(name);
			uint16_t namelength = (uint16_t)mutf.length();
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			buffer.insert(buffer.end(), mutf.begin(), mutf.end());
			return (int32_t)buffer.size();
		}
		// Loads the default header, returning an index to the end of the header. Used by all tags (except for end tags!).
		size_t loadDefault(const char const* bytes, size_t offset) {
			// Grabs the id of the tag
			id = bytes[offset];

			// Checks id against what the id of this tag type should be, and throw an exception if it's invalid
			if (id != correct_tag() && id != NBT_BYPASS_ID && correct_tag() != NBT_BYPASS_ID) {
				throw invalid_tag_id_exception(id, correct_tag());
			}
			id = correct_tag();

			// Get the length of the name
			uint16_t namelength = 0;
			fromBytes(&bytes[offset + 1], &namelength);

			// Copy the name out of the bytes nbt data array
			name = mutfToUtf(std::string((const char*)&bytes[offset + 3], namelength));
			return (int32_t)offset + 3 + namelength;
		}
	};

	/*
	A list of constructor functions, mapped to the ID of the tag the constructor creates. This list is used by list and compound tags, in order to quickly call
	the correct tag's constructor based on a given ID. (Read compound::load or list::load for examples)

	This both makes the code a little simpler (lookup table instead of, say, a switch statement spanning 50 lines) while also making room for custom tags.

	For example, if the user creates a tag class (child of nbt::tag,) with a unique ID, they may make it a recognized and valid tag that will be created by 
	list tags and compounds.

	Example:
	class pointertag : public tag {
		- tag class data -
	}
	// Then, before any compound::load calls are made,
	registerTag<pointertag>(15); // Registers a "pointertag" with the id 15.

	This is made even simpler with primitivetags and primitivearraytags, see comments near those classes for details.
	*/
	extern std::map<int8_t, tag* (*)()> tagConstructors;

	// Used to essentially reference a class's default constructor
	template <class T>
	T* create() {
		return new T;
	}

	// Register's a tag class (T) to 'tagConstructors' with the given ID.
	template <typename T>
	void registerTag(int8_t id) {
		tagConstructors.insert(std::make_pair(id, (tag * (*)()) create<T>));
	}

	// A function and state variable called by compound and list tags in order to register the default set of tags.
	static bool registeredDefaultTags = false;
	static void registerDefaultTags();

	// The end class stores no data, but signals a compound tag when it is time to stop reading data. Like a null-terminated string. Very self explanitory in all it does.
	class end : public tag {
	public:
		end() {
			id = 0;
			name = "";
		}
		size_t load(const char* const bytes, size_t offset) {
			return offset + 1;
		}
		size_t write(std::vector<char>& buffer) {
			buffer.push_back(0);
			return buffer.size();
		}
		size_t write(char* buffer, size_t offset) {
			if (buffer == nullptr)
				return offset + 1;
			buffer[offset] = 0;
			return offset + 1;
		}
		std::vector<char> value_bytes() {
			return { 0 };
		}
		void discard() {
			name.clear();
		}

		const int8_t correct_tag() { return 0; }

#ifdef NBT_COMPILE
		std::string compilation(std::string regex = "") {
			return regex + "END\n";
		};
		friend std::ostream& operator<< (std::ostream& left, end& right) {
			left << right.compilation();
			return left;
		}
#endif
	};

	/*
	Abstract class used for primive tags such as : IntTag, FloatTag, ByteTag, DoubleTag, etc.

	All "primitive data tags" use essentially the same code. 
	They read data the same way, write it the same way, and store just one single instance of a given type,
	just with different data lengths and IDs.

	So the class is templated, and all default primitive data tags are just type definitions of it.
	*/
	template <typename T, int8_t ID>
	class primitivetag : public tag {
	public:
		T data;

		primitivetag() {
			data = T();
			id = correct_tag();
		}
		primitivetag(std::string name) : primitivetag() {
			this->name = name;
		}
		primitivetag(T data) : primitivetag() {
			this->data = data;
		}
		primitivetag(T data, std::string name) : primitivetag() {
			this->name = name;
			this->data = data;
		}
		primitivetag(std::string name, T data) : primitivetag() {
			this->name = name;
			this->data = data;
		}
		primitivetag(tag* tag) {
			primitivetag<T>* t = dynamic_cast<primitivetag<T>*>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}

		size_t load(const char* const bytes, size_t offset) {
			size_t off = loadDefault(bytes, offset);

			// Use primitive casting to convert the data (in bytes) to the data (as a primitive)
			// This allows us to use floating point types with primitive tag. The other way to convert to primitive (by using bit shifting "<<") will only convert to integer types
			fromBytes<T>(&bytes[off], &data);

			//Return where the next tag will start in bytes[]
			return off + sizeof(T);
		}
		size_t write(char* const buffer, size_t offset) {
			size_t off = writeDefault(buffer, offset);
			if (buffer == nullptr)
				return off + sizeof(T);
			// Convert the data into byte form, and copy it to the buffer
			toBytes(data, &buffer[off]);

			//Return where the next tag will start in buffer[]
			return off + sizeof(T);
		}
		size_t write(std::vector<char>& buffer) {
			writeDefault(buffer);

			// Make room for the data
			buffer.insert(buffer.end(), sizeof(T), 0);
			// Convert the data into byte form, and copy it to the buffer
			toBytes(data, &buffer[buffer.size() - sizeof(T)]);

			//Return length of vector
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> out = std::vector<char>();
			// Make room for the data
			out.insert(out.end(), sizeof(T), 0);
			// Convert the data into byte form, and copy it to the buffer
			toBytes(data, &out[0]);
			return out;
		}
		void discard() {
			// data = (T)0; Redundant
			name.clear();
		}

		// CONVERSION AND SYNTAX SIMPLIFICATION
		operator T() {
			return data;
		}
		operator T* () {
			return &data;
		}
		operator primitivetag<T, ID>* () {
			return this;
		}

		const int8_t correct_tag() {
			return ID;
		}

#ifdef NBT_COMPILE
		// The only problem with a primitive tag is that you cannot easily specify the name of each tag in compilation. Thus we just define default names and call everything else "custom"
		std::string compilation(std::string regex = "") {
			std::string type = "";
			if (typeid(T) == typeid(int8_t) || typeid(T) == typeid(uint8_t))
				type = "Byte";
			else if (typeid(T) == typeid(int16_t) || typeid(T) == typeid(uint16_t))
				type = "Short";
			else if (typeid(T) == typeid(int32_t) || typeid(T) == typeid(uint32_t))
				type = "Int";
			else if (typeid(T) == typeid(int64_t) || typeid(T) == typeid(uint64_t))
				type = "Long";
			else if (typeid(T) == typeid(float))
				type = "Float";
			else if (typeid(T) == typeid(double))
				type = "Double";
			else
				type = "Custom";
			std::string out = regex + type + "Tag(" + std::string(name) + "): " + std::to_string(data) + "\n";
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, primitivetag<T, ID>& right) {
			left << right.compilation();
			return left;
		}
#endif
	};

	// Similar to primitivetag, but storing arrays (vectors) of primitive types instead of a single instance.
	template <typename T, int8_t ID>
	class primitivearraytag : public tag {
	public:
		std::vector<T> data;
		primitivearraytag() {
			this->data = std::vector<T>();
			id = correct_tag();
		}
		primitivearraytag(std::vector<T> data) : primitivearraytag() {
			this->data = data;
		}
		primitivearraytag(std::string name) : primitivearraytag() {
			this->data = std::vector<T>();
			this->name = name;
		}
		primitivearraytag(std::string name, std::vector<T> data) : primitivearraytag() {
			this->data = data;
			this->name = name;
		}
		primitivearraytag(std::vector<T> data, std::string name) : primitivearraytag() {
			this->data = data;
			this->name = name;
		}
		primitivearraytag(const tag* const tag) : primitivearraytag() {
			const primitivearraytag<T>* const t = dynamic_cast<const primitivearraytag<T>* const>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}

		size_t load(const char* const bytes, size_t offset) {
			size_t off = loadDefault(bytes, offset);

			// Get the amount of elements in the array
			uint32_t length = 0;
			fromBytes(&bytes[off], &length);
			// Initiate the array as empty
			data = std::vector<T>();
			// Edit offset for simplicity
			off += 4;

			// Loop through each element and push them into data
			for (uint32_t i = 0; i < length; i++) {
				// Add the current data into the list, and change the offset for the next loop
				data.push_back(*new T(0));
				fromBytes<T>(&bytes[off], &data[i]);
				off += sizeof(T);
			}
			return off;
		}
		size_t write(char* const buffer, size_t offset) {
			size_t off = writeDefault(buffer, offset);
			if (buffer == nullptr)
				return off + 4 * sizeof(T) * data.size();

			// Convert the length into byte form, and copy it to the buffer
			toBytes((uint32_t)data.size(), &buffer[off]);
			off += 4;
			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				toBytes(data[i], &buffer[off]);
				// Change offset for future use
				off += sizeof(T);
			}

			//Return where the next tag will start in buffer[]
			return off;
		}
		size_t write(std::vector<char>& buffer) {
			size_t off = writeDefault(buffer);
			// Free space for the output
			buffer.insert(buffer.end(), sizeof(T) * data.size() + 4, 0x00);
			// Convert the length into byte form, and copy it to the buffer
			toBytes((uint32_t)data.size(), &buffer[off]);
			off += 4;
			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				toBytes(data[i], &buffer[off]);
				// Change offset for future use
				off += sizeof(T);
			}

			//Return where the next tag will start in buffer[]
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> out = std::vector<char>();
			// Free space for the output
			out.insert(out.end(), sizeof(T) * data.size() + 4, 0x00);
			// Convert the length into byte form, and copy it to the buffer
			toBytes((uint32_t)data.size(), &out[0]);
			int off = 4;
			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				toBytes(data[i], &out[off]);
				// Change offset for future use
				off += sizeof(T);
			}
			return out;
		}
		void discard() {
			data.clear();
			name.clear();
		}
		// CONVERSIONS AND SYNTAX SIMPLIFICATION

		// primitivearraytag<int, 3> intarray; intarray << 10; intarray << 12;
		// Not sure why this operator isn't used by vectors and stack-like data structures in general, tbh.
		void operator<<(T t) {
			data.push_back(t);		
		}
		T operator[](size_t i) {
			return data[i];
		}
		operator std::vector<T>() {
			return data;
		}

		const int8_t correct_tag() {
			return ID;
		}

#ifdef NBT_COMPILE
		// See primitivetag for complaints on nbt-compilation with template classes!
		std::string compilation(std::string regex = "") {
			std::string type = "";
			if (typeid(T) == typeid(int8_t) || typeid(T) == typeid(uint8_t))
				type = "Byte";
			else if (typeid(T) == typeid(int16_t) || typeid(T) == typeid(uint16_t))
				type = "Short";
			else if (typeid(T) == typeid(int32_t) || typeid(T) == typeid(uint32_t))
				type = "Int";
			else if (typeid(T) == typeid(int64_t) || typeid(T) == typeid(uint64_t))
				type = "Long";
			else
				type = "Custom";
			std::string out = regex + type + "ArrayTag(" + std::string(name) + "): " + std::to_string(data.size()) + " " + type + "s \n";
#ifdef NBT_COMPILE_FULL_ARRAYS
			// By default, arrays don't spew out their contents when being printed, but it is an option you can toggle on.
			for (unsigned int i = 0; i < data.size(); i++) {
				out += regex + "\t" + std::to_string(data[i]) + "\n";
			}
#endif
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, primitivearraytag<T, ID>& right) {
			left << right.compilation();
			return left;
		}
#endif
	};

	// All of the primitive data tags that are present by default with this library.
	// Unsigned tags are not defined by the NBT standard, they are present here by my choice, as it just makes sense to be able to store unsigned data types with NBT
	typedef primitivetag<int8_t, 1> bytetag;			//1
	typedef primitivetag<uint8_t, -1> ubytetag;

	typedef primitivetag<int16_t, 2> shorttag;			//2
	typedef primitivetag<uint16_t, -2> ushorttag;

	typedef primitivetag<int32_t, 3> inttag;			//3
	typedef primitivetag<uint32_t, -3> uinttag;

	typedef primitivetag<int64_t, 4> longtag;			//4
	typedef primitivetag<uint64_t, -4> ulongtag;

	typedef primitivetag<float, 5> floattag;			//5

	typedef primitivetag<double, 6> doubletag;

	typedef primitivearraytag<int8_t, 7> bytearray;	//7
	typedef primitivearraytag<uint8_t, -7> ubytearray;

	typedef primitivearraytag<int32_t, 11> intarray;	//11
	typedef primitivearraytag<uint32_t, -11> uintarray;

	typedef primitivearraytag<int64_t, 12> longarray;	//12
	typedef primitivearraytag<uint64_t, -12> ulongarray;

	// Honestly not much to say here. Strings in NBT files are lenght-based rather than null-terminated, so this class reads the length of the string before the string itself.
	class stringtag : public tag {
	public:
		std::string data;
		stringtag() {
			data = std::string();
			id = 8;
		}
		stringtag(std::string name) : stringtag() {
			this->name = name;
		}
		stringtag(std::string data, std::string name) : stringtag() {
			this->name = name;
			this->data = data;
		}
		stringtag(const tag* const tag) {
			const stringtag* const t = dynamic_cast<const stringtag* const>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}
		size_t load(const char* const bytes, size_t offset) {
			size_t off = loadDefault(bytes, offset);

			// Load the size of the string
			uint16_t datlength = 0;
			fromBytes(&bytes[off], &datlength);
			// Copy the string out of the bytes nbt data array
			data = mutfToUtf(std::string(&bytes[off + 2], datlength));

			//Return where the next tag will start in bytes[]
			return off + 2 + datlength;
		}
		size_t write(char* const buffer, size_t offset) {
			size_t off = writeDefault(buffer, offset);
			std::string mutf = utfToMutf(data);
			if (buffer == nullptr)
				return off + 2 + mutf.length();
			// Grab the name length and store it in the buffer
			uint16_t datlength = (uint16_t)mutf.length();
			buffer[offset + 1] = (datlength >> 8) & 0xff;
			buffer[offset + 2] = (datlength) & 0xff;

			// Copy the name into the buffer
			memcpy_s(&buffer[off + 2], datlength, mutf.data(), datlength);

			//Return where the next tag will start in buffer[]
			return off + 2 + mutf.length();
		}
		size_t write(std::vector<char>& buffer) {
			int off = writeDefault(buffer);

			// Grab the data length and store it in the buffer
			uint16_t datlength = (uint16_t)data.length();
			buffer.push_back((datlength >> 8) & 0xff);
			buffer.push_back((datlength) & 0xff);

			std::string mutf = utfToMutf(data);
			// Copy the data into the buffer
			buffer.insert(buffer.end(), mutf.begin(), mutf.end());
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			//return (char*)data.c_str();
			std::vector<char> out = std::vector<char>();
			// Grab the data length and store it in the buffer
			uint16_t datlength = (uint16_t)data.length();
			out.push_back((datlength >> 8) & 0xff);
			out.push_back((datlength) & 0xff);

			// Copy the data into the buffer
			out.insert(out.end(), data.begin(), data.end());
			return out;
		}
		void discard() {
			data.clear();
			name.clear();
		}
		// CONVERSION AND SYNTAX SIMPLIFICATION
		operator std::string() {
			return data;
		}
		operator std::string* () {
			return &data;
		}
		operator stringtag* () {
			return this;
		}

		const int8_t correct_tag() {
			return 8;
		}
#ifdef NBT_COMPILE
		std::string compilation(std::string regex = "") {
			std::string out = regex + "StringTag(" + std::string(name) + "): " + data + "\n";
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, stringtag& right) {
			left << right.compilation();
			return left;
		}
#endif
	};

	// Forward declaration, compounds and lists are used in tag_p class, and tag_p is used in compound and list classes
	class compound;
	class list;

	/*
	Interface class that lets you cast between types without unsafe casting, or having to write dynamic_cast<tagtype> every line
	
	For instance, any subclass of nbt::tag can be referenced and stored as a (tag*)
	The tag_p class takes a (tag*) and gives you functions and operators that allow you to easily interact with the data of the actual tag,
	without having to specify that you are trying to talk to an inttag instead of a generic tag*

	Example:
	std::vector<tag*> tag_list = std::vector<tag*>();
	tag_list.push(new inttag("int tag one", 10));
	tag_list.push(new inttag("int tag two", 20));

	dynamic_cast<inttag*>(tag_list[0]).value; // 10, clunky bad way!
	tag_p(tag_list[0])._int(); // 20, made even better if tag_list was a std::vector<tag_p>();

	See compound and list tag classes for the usages of tag_p.
	*/
	class tag_p {
	public:
		tag* value;
		tag_p() {
			value = nullptr;
		}
		tag_p(tag* value) : value(value) {}
		/*
		* When you grab a compound tag from a parent compound tag, you may then instantly grab contents from it without having to specify that it is a compound tag.
		* For example, in this dataset (given in JSON) you'll see the data we want, and where it is stored
		*	"data": {
		*		"data compound": {
		*			"data we want": ":)"
		*		}
		*	}
		* 
		* This operator lets us write:
		* data["data compound"]["data we want"] without any errors.
		*/
		tag_p& operator [](std::string key);
		/*
		* Similar to the above operation, but for list tags.
		*	"data": {
		*		"data list": ["data1", "data2", "data3"]
		*	}
		*
		* data["data list"][1]
		*/
		tag_p& operator [](size_t index);
		/*
		* Lets you directly check if a compound has a given key
		*/
		bool has(std::string key);
		/* 
		* Allows pointer operations, like
		* tag_p data = tag_p(new inttag("name", 10));
		* data->name; // Valid
		* data->id; // Valid
		*/
		tag* operator->() {
			return value;
		}
		void discard() {
			if (value) {
				value->discard();
				delete value;
			}
			value = nullptr;
		}

		/*
		* The big mess of conversions. 
		* The syntax is temporary, but what it does is self explanitory.
		* 
		* The _byte() function, attempts to cast the tag* value to a bytetag* value, and return the int8_t that is stored within it.
		* The _bytetag() function, attempts to cast the tag* value to a bytetag* value, and return that casted value.
		* (Throws invalid_tag_operator if the value was not actually the requested tag type)
		*/
		int8_t& _byte() { if (value->id != 1) throw invalid_tag_operator(value->id, 1); return dynamic_cast<bytetag*>(value)->data; }
		uint8_t& _ubyte() { if (value->id != -1) throw invalid_tag_operator(value->id, -1); return dynamic_cast<ubytetag*>(value)->data; }
		int16_t& _short() { if (value->id != 2) throw invalid_tag_operator(value->id, 2); return dynamic_cast<shorttag*>(value)->data; }
		uint16_t& _ushort() { if (value->id != -2) throw invalid_tag_operator(value->id, -2); return dynamic_cast<ushorttag*>(value)->data; }
		int32_t& _int() { if (value->id != 3) throw invalid_tag_operator(value->id, 3); return dynamic_cast<inttag*>(value)->data; }
		uint32_t& _uint() { if (value->id != -3) throw invalid_tag_operator(value->id, -3); return dynamic_cast<uinttag*>(value)->data; }
		int64_t& _long() { if (value->id != 4) throw invalid_tag_operator(value->id, 4); return dynamic_cast<longtag*>(value)->data; }
		uint64_t& _ulong() { if (value->id != -4) throw invalid_tag_operator(value->id, -4); return dynamic_cast<ulongtag*>(value)->data; }
		float& _float() { if (value->id != 5) throw invalid_tag_operator(value->id, 5); return dynamic_cast<floattag*>(value)->data; }
		double& _double() { if (value->id != 6) throw invalid_tag_operator(value->id, 6); return dynamic_cast<doubletag*>(value)->data; }
		std::vector<int8_t>& _bytearray() { if (value->id != 7) throw invalid_tag_operator(value->id, 7); return dynamic_cast<bytearray*>(value)->data; }
		std::vector<uint8_t>& _ubytearray() { if (value->id != -7) throw invalid_tag_operator(value->id, -7); return dynamic_cast<ubytearray*>(value)->data; }
		std::vector<int32_t>& _intarray() { if (value->id != 11) throw invalid_tag_operator(value->id, 11); return dynamic_cast<intarray*>(value)->data; }
		std::vector<uint32_t>& _uintarray() { if (value->id != -11) throw invalid_tag_operator(value->id, -11); return dynamic_cast<uintarray*>(value)->data; }
		std::vector<int64_t>& _longarray() { if (value->id != 12) throw invalid_tag_operator(value->id, 12); return dynamic_cast<longarray*>(value)->data; }
		std::vector<uint64_t>& _ulongarray() { if (value->id != -12) throw invalid_tag_operator(value->id, -12); return dynamic_cast<ulongarray*>(value)->data; }
		std::string& _string() { if (value->id != 8) throw invalid_tag_operator(value->id, 8); return dynamic_cast<stringtag*>(value)->data; }

		bytetag& _bytetag() { if (value->id != 1) throw invalid_tag_operator(value->id, 1); return *dynamic_cast<bytetag*>(value); }
		ubytetag& _ubytetag() { if (value->id != -1) throw invalid_tag_operator(value->id, -1); return *dynamic_cast<ubytetag*>(value); }
		shorttag& _shorttag() { if (value->id != 2) throw invalid_tag_operator(value->id, 2); return *dynamic_cast<shorttag*>(value); }
		ushorttag& _ushorttag() { if (value->id != -2) throw invalid_tag_operator(value->id, -2); return *dynamic_cast<ushorttag*>(value); }
		inttag& _inttag() { if (value->id != 3) throw invalid_tag_operator(value->id, 3); return *dynamic_cast<inttag*>(value); }
		uinttag& _uinttag() { if (value->id != -3) throw invalid_tag_operator(value->id, -3); return *dynamic_cast<uinttag*>(value); }
		longtag& _longtag() { if (value->id != 4) throw invalid_tag_operator(value->id, 4); return *dynamic_cast<longtag*>(value); }
		ulongtag& _ulongtag() { if (value->id != -4) throw invalid_tag_operator(value->id, -4); return *dynamic_cast<ulongtag*>(value); }
		floattag& _floattag() { if (value->id != 5) throw invalid_tag_operator(value->id, 5); return *dynamic_cast<floattag*>(value); }
		doubletag& _doubletag() { if (value->id != 6) throw invalid_tag_operator(value->id, 6); return *dynamic_cast<doubletag*>(value); }
		bytearray& _bytearraytag() { if (value->id != 7) throw invalid_tag_operator(value->id, 7); return *dynamic_cast<bytearray*>(value); }
		ubytearray& _ubytearraytag() { if (value->id != -7) throw invalid_tag_operator(value->id, -7); return *dynamic_cast<ubytearray*>(value); }
		intarray& _intarraytag() { if (value->id != 11) throw invalid_tag_operator(value->id, 11); return *dynamic_cast<intarray*>(value); }
		uintarray& _uintarraytag() { if (value->id != -11) throw invalid_tag_operator(value->id, -11); return *dynamic_cast<uintarray*>(value); }
		longarray& _longarraytag() { if (value->id != 12) throw invalid_tag_operator(value->id, 12); return *dynamic_cast<longarray*>(value); }
		ulongarray& _ulongarraytag() { if (value->id != -12) throw invalid_tag_operator(value->id, -12); return *dynamic_cast<ulongarray*>(value); }
		stringtag& _stringtag() { if (value->id != 8) throw invalid_tag_operator(value->id, 8); return *dynamic_cast<stringtag*>(value); }
		// Forward declared because the bodies of compound and list classes have not been defined yet
		compound& _compound();
		list& _list();
#ifdef NBT_SHORTHAND
		/*
		* As you understand the convestion functions, you may use the shorthand versions instead to shorten your code, and potentially make it more readable.
		* As long as you understand which functions are which, and what does what 
		* 
		* Every function is the initials of the previous set of functions returned, with only a few exceptions
		* - str() and strt() are used to return string and stringtag. As 's' and 'st' are taken by shorts and short tags
		* - li() is used to return a list, as 'l' is taken by long and long tags.
		*/
		int8_t& b() { return _byte(); }
		uint8_t& ub() { return _ubyte(); }
		int16_t& s() { return _short(); }
		uint16_t& us() { return _ushort(); }
		int32_t& i() { return _int(); }
		uint32_t& ui() { return _uint(); }
		int64_t& l() { return _long(); }
		uint64_t& ul() { return _ulong(); }
		float& f() { return _float(); }
		double& d() { return _double(); }
		std::vector<int8_t>& ba() { return _bytearray(); }
		std::vector<uint8_t>& uba() { return _ubytearray(); }
		std::vector<int32_t>& ia() { return _intarray(); }
		std::vector<uint32_t>& uia() { return _uintarray(); }
		std::vector<int64_t>& la() { return _longarray(); }
		std::vector<uint64_t>& ula() { return _ulongarray(); }
		std::string& str() { return _string(); }

		bytetag& bt() { return _bytetag(); }
		ubytetag& ubt() { return _ubytetag(); }
		shorttag& st() { return _shorttag(); }
		ushorttag& ust() { return _ushorttag(); }
		inttag& it() { return _inttag(); }
		uinttag& uit() { return _uinttag(); }
		longtag& lt() { return _longtag(); }
		ulongtag& ult() { return _ulongtag(); }
		floattag& ft() { return _floattag(); }
		doubletag& dt() { return _doubletag(); }
		bytearray& bat() { return _bytearraytag(); }
		ubytearray& ubat() { return _ubytearraytag(); }
		intarray& iat() { return _intarraytag(); }
		uintarray& uiat() { return _uintarraytag(); }
		longarray& lat() { return _longarraytag(); }
		ulongarray& ulat() { return _ulongarraytag(); }
		stringtag& strt() { return _stringtag(); }
		compound& c() { return _compound(); }
		list& li() { return _list(); }
#endif
#ifdef NBT_COMPILE
		friend std::ostream& operator<< (std::ostream& left, tag_p& right) {
			left << right->compilation();
			return left;
		}
#endif
	};


	/*
	* A list tag stores a list of same-type tags.
	* Each tag read will not have a name, and will only be referenced by their index.
	* Despite the fact that it will only store a single type of tag in its data, it still uses tag_p instead of being a template.
	* As template<typename T> class list ... requires that you specify the typename of T whenever you reference the list class, even when casting to a list.
	*/
	class list : public tag {
	public:
		std::vector<tag_p> tags;
		char tag_type = NBT_BYPASS_ID;
		list() {
			tags = std::vector<tag_p>();
			id = 9;
		}
		list(std::vector<tag_p> tags) {
			this->tags = tags;
			if (tags.size() > 0)
				tag_type = tags[0]->id;
			id = 9;
		}
		list(std::vector<tag_p> tags, std::string name) {
			this->tags = tags;
			this->name = name;
			if (tags.size() > 0)
				tag_type = tags[0]->id;
			id = 9;
		}
		list(std::string name) {
			this->name = name;
			id = 9;
		}
		list(std::string name, std::vector<tag_p> tags) {
			this->tags = tags;
			this->name = name;
			if (tags.size() > 0)
				tag_type = tags[0]->id;
			id = 9;
		}
		list(const tag* const tag) {
			// IS NOT A CAST FUNCTION!
			// This is a copy constructor, and assumes that the input tag* is a list*.
			const list* const t = dynamic_cast<const list* const>(tag);
			this->tags = t->tags;
			this->id = 9;
			this->tag_type = t->tag_type;
			this->name = t->name;
		}
		// Passthrough function
		void clear() {
			this->tags.clear();
		}
		void discard() {
			name.clear();
			for (auto it = tags.begin(); it != tags.end();) {
				it->discard();
				it = tags.erase(it);
			}
		}

		size_t load(const char* const bytes, size_t offset) {
			// Make sure all tags are registered
			registerDefaultTags();

			size_t off = loadDefault(bytes, offset);
			tag_type = bytes[off];

			if (tagConstructors.find(tag_type) == tagConstructors.end())
				throw missing_tag_id_exception(tag_type);

			uint32_t length = 0;
			fromBytes(&bytes[++off], &length);
			off += 4;
			tag* tag;
			for (uint32_t i = 0; i < length; i++) {
				tag = (*tagConstructors[tag_type])();
				// SHHHH don't tell anyone that I casted away the const!
				const_cast<char* const>(bytes)[off - 3] = NBT_BYPASS_ID;
				const_cast<char* const>(bytes)[off - 2] = 0x00;
				const_cast<char* const>(bytes)[off - 1] = 0x00;
				off = tag->load(bytes, off - 3);
				tags.push_back(tag);
			}
			return off;
		}
		size_t write(char* const buffer, size_t offset) {
			// Push the id of the tag
			size_t off = writeDefault(buffer, offset);

			buffer[off++] = tag_type;
			toBytes((uint32_t)tags.size(), &buffer[off]);
			off += 4;

			for (auto i = tags.begin(); i != tags.end(); i++) {
				auto tdat = i->value->value_bytes();
				memcpy_s(&buffer[off], tdat.size(), &tdat[0], tdat.size());
				off += tdat.size();
				//buffer.insert(buffer.end(), tdat.begin(), tdat.end());
			}
			return off;
		}
		size_t write(std::vector<char>& buffer) {
			int off = writeDefault(buffer);

			// Write tag type and number of tags to buffer
			buffer.insert(buffer.end(), 5, tag_type);
			toBytes((uint32_t)tags.size(), &buffer[off + 1]);

			for (auto i = tags.begin(); i != tags.end(); i++) {
				auto tdat = i->value->value_bytes();
				buffer.insert(buffer.end(), tdat.begin(), tdat.end());
			}
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> buffer = std::vector<char>();
			// Write tag type and number of tags to buffer
			buffer.insert(buffer.end(), 5, tag_type);
			toBytes((uint32_t)tags.size(), &buffer[1]);

			for (int i = 0; i < tags.size(); i++) {
				auto tdat = tags[i]->value_bytes();
				buffer.insert(buffer.end(), tdat.begin(), tdat.end());
			}
			return buffer;
		}

		void add(tag_p t) {
			if (tag_type == NBT_BYPASS_ID)
				tag_type = t->correct_tag();
			if (t->correct_tag() != tag_type)
				throw illegal_list_tag_type(t->correct_tag());
			else
				tags.push_back(t);
		}

		// SYNTAX AND CODE SIMPLIFICATION

		// list[1]
		tag_p& operator[](size_t i) {
			// vector::at throws std::out_of_range if input is, well, out of range.
			return tags.at(i);
		}
		operator std::vector<tag_p>() {
			return tags;
		}
		void operator<<(tag_p t) {
			add(t);
		}

		const int8_t correct_tag() {
			return 9;
		}

#ifdef NBT_COMPILE
		std::string compilation(std::string regex = "") {
			std::string out = regex + "ListTag(" + std::string(name) + "): " + std::to_string(tags.size()) + " tags {\n";
			for (int i = 0; i < tags.size(); i++) {
				std::string tag = tags[i]->compilation(regex + "\t");
				out += tag;
			}
			out += regex + "}\n";
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, list& right) {
			left << right.compilation();
			return left;
		}
#endif
	};

	/*
	The NBT Compound Tag is the starting point of all NBT datasets. It stores a list of Tags, mapped to their names. Therefore, they store tags using an std::map.
	Because all of the tags are unique classes, the Compound Tag instead stores pointers to the nbt::tag superclass in the form of the tag_p interface class.

	tag_p is how you would mostly be interacting with the API, that would be where to read next.
	*/

	class compound : public tag {
	public:
		std::map<std::string, tag_p> tags;
		compound() {
			tags = std::map<std::string, tag_p>();
			id = 10;
		}
		compound(std::map<std::string, tag_p> tags) {
			this->tags = tags;
			id = 10;
		}
		compound(std::string name) {
			this->name = name;
			id = 10;
		}
		compound(std::map<std::string, tag_p> tags, std::string name) {
			this->tags = tags;
			this->name = name;
			id = 10;
		}
		compound(std::string name, std::map<std::string, tag_p> tags) {
			this->tags = tags;
			this->name = name;
			id = 10;
		}
		compound(const tag* const tag) {
			// Note: Creates a new compound tag, and is not the same thing as a cast. Instead, use tag_p in order to interact between types.
			const compound* const t = dynamic_cast<const compound* const>(tag);
			this->tags = t->tags;
			this->id = 10;
			this->name = t->name;
		}

		void discard() {
			name.clear();
			for (auto it = tags.begin(); it != tags.end();) {
				it->second.discard();
				it = tags.erase(it);
			}
		}

		// Loads compound tag data from a list of bytes
		size_t load(const char* const bytes, size_t offset) {
			// Make sure all tags are registered, required in order to use the provided default tag types.
			registerDefaultTags();

			size_t off = loadDefault(bytes, offset);
			char t;
			tag* tag;
			while (true) {
				t = bytes[off];
				if (t == 0) {
					tags.insert(std::make_pair(NBT_END_TAG_NAME, new end()));
					return off + 1;
				}
				if (tagConstructors.find(t) == tagConstructors.end())
					throw missing_tag_id_exception(t);
				tag = (*tagConstructors[t])();
				off = tag->load(bytes, off);
				tags.insert(std::make_pair(tag->name, tag));
			}
			return off + 1;
		}
		size_t write(char* const buffer, size_t offset) {
			// Writes default tag header.
			size_t off = writeDefault(buffer, offset);

			// Loops through every stored tag, and call it's write function, unless it is an end tag, as that is written at the end of the compound tag.
			for (auto i = tags.begin(); i != tags.end(); i++)
				if (i->second->id!=0)
					off = i->second->write(buffer, off);
			// Compound Tags are required to provide an End Tag at the end of their definition, like a closing '}' for code bodies.
			// If the option is given, then Compound tags will throw an endless_compound_exception if it does not contain an end tag.
#ifdef NBT_THROW_ENDLESS
			if (tags.count(NBT_END_TAG_NAME) == 0)
				throw endless_compound_exception();
#endif
			buffer[off++] = 0x00; // Write end tag
			return off;
		}
		size_t write(std::vector<char>& buffer) {
			// See write(char*, size_t) comments
			writeDefault(buffer);

			for (auto i = tags.begin(); i != tags.end(); i++)
				if(i->second->id!=0)
					i->second->write(buffer);

#ifdef NBT_THROW_ENDLESS
			if (tags.count(NBT_END_TAG_NAME) == 0)
				throw endless_compound_exception();
#endif
			buffer.push_back(0x00);

			return buffer.size();
		}
		std::vector<char> value_bytes() {
			// Provides the data of all contained tags, essentially a write without the default header.
			std::vector<char> buffer = std::vector<char>();
			for (auto i = tags.begin(); i != tags.end(); i++)
				if (i->second->id != 0)
					i->second->write(buffer);
			buffer.push_back(0x00);
			return buffer;
		}

		tag_p& get(const char* name) {
			// Could check for missing keys, however std::map::at does that already (compared to operator[] which returns a default constructor if key not found)
			// if (tags.count(name) == 0)
			return tags.at(name);
		}

		bool has(const char* name) {
			return tags.count(name);
		}

		void add(tag_p tag) {
			tags.insert(std::make_pair(tag->name, tag));
		}

		// SYNTAX AND CODE SIMPLIFICATION

		// compound["sub tag"]
		tag_p& operator[](const char* name) {
			return get(name);
		}
		// compound().begin()
		operator std::map<std::string, tag_p>() {
			return tags;
		}
		// compound << new inttag()
		void operator<<(tag_p t) {
			tags.insert(std::make_pair(t->name, t));
		}
		size_t size() {
			return tags.size();
		}

		const int8_t correct_tag() {
			return 10;
		}
#ifdef NBT_COMPILE
		// Returns a string that can be useful for debugging
		std::string compilation(std::string regex = "") {
			std::string out = regex + "CompoundTag(" + std::string(name) + "): " + std::to_string(tags.size()) + " tags {\n";
			for (auto i = tags.begin(); i != tags.end(); i++) {
				if (i->second.value == nullptr || i->second->id == 0)
					continue;
				std::string tag = i->second->compilation(regex + "\t");
				out += tag;
			}
			out += regex + "}\n";
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, compound& right) {
			return (left << right.compilation());
		}
#endif
	};

#ifdef NBT_SHORTHAND
	typedef bytetag bt;
	typedef ubytetag ubt;
	typedef shorttag st;
	typedef ushorttag ust;
	typedef inttag it;
	typedef uinttag uit;
	typedef longtag lt;
	typedef ulongtag ult;
	typedef floattag ft;
	typedef doubletag dt;
	typedef bytearray ba;
	typedef ubytearray uba;
	typedef list li;
	typedef compound c;
	typedef stringtag str;
	typedef intarray ia;
	typedef uintarray uia;
	typedef longarray la;
	typedef ulongarray ula;
#endif

	static void registerDefaultTags() {
		if (registeredDefaultTags)
			return;

		registerTag<end>(0);
		registerTag<bytetag>(1);
		registerTag<ubytetag>(-1);
		registerTag<shorttag>(2);
		registerTag<ushorttag>(-2);
		registerTag<inttag>(3);
		registerTag<uinttag>(-3);
		registerTag<longtag>(4);
		registerTag<ulongtag>(-4);
		registerTag<floattag>(5);
		registerTag<doubletag>(6);
		registerTag<bytearray>(7);
		registerTag<ubytearray>(-7);
		registerTag<stringtag>(8);
		registerTag<list>(9);
		registerTag<compound>(10);
		registerTag<intarray>(11);
		registerTag<uintarray>(-11);
		registerTag<longarray>(12);
		registerTag<ulongarray>(-12);

		nbt::registeredDefaultTags = true;
	}
}
//#define NBT_INCLUDE
// Again, define on first include! Defines a few necessary functions
#ifdef NBT_INCLUDE
#undef NBT_INCLUDE

/*
* This function really deserves it's own smaller library, honestly.
* Converts from the normal global UTF-8 to the "Modified UTF-8" used in Java.
* https://docs.oracle.com/javase/6/docs/api/java/io/DataInput.html#modified-utf-8 is the only official documentation of Modified UTF-8
* Pretty simple conversion, but annoying, and uses <codecvt> to make things a little simpler.
* 
* Essentially goes from UTF-8, to 32-bit Unicode, to MUTF-8.
*/
std::string nbt::utfToMutf(std::string utf) {
#ifdef NBT_IGNORE_MUTF
	return utf;
#else
	std::string out = "";
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	std::u32string uni = converter.from_bytes(utf);

	for (int i = 0; i < uni.size(); i++) {
		uint32_t cur = uni[i];
		if (cur <= 0x7F)
			out += (char)cur;
		else if (cur <= 0x7ff) {
			unsigned char y = (cur & 0b00111111);
			unsigned char x = (cur >> 6) & 0b00011111;
			out += char(x + 0b11000000);
			out += char(y + 0b10000000);
		}
		else if (cur <= 0xFFFF) {
			unsigned char z = cur & 0b00111111;
			unsigned char y = (cur >> 6) & 0b00111111;
			unsigned char x = (cur >> 12) & 0b00001111;
			out += char(x + 0b11100000);
			out += char(y + 0b10000000);
			out += char(z + 0b10000000);
		}
		else {
			cur -= 0x10000;
			unsigned char z = cur & 0b00111111;
			unsigned char y = (cur >> 6) & 0b00001111;
			unsigned char w = (cur >> 10) & 0b00111111;
			unsigned char v = (cur >> 16) & 0b00001111;
			out += char(0b11101101);
			out += char(v + 0b10100000);
			out += char(w + 0b10000000);
			out += char(0b11101101);
			out += char(y + 0b10110000);
			out += char(z + 0b10000000);
		}
	}
	return out;
#endif
}

/*
* See previous function! Goes from MUTF-8 to 32-bit Unicode to UTF-8
*/
std::string nbt::mutfToUtf(std::string mutf) {
#ifdef NBT_IGNORE_MUTF
	return mutf;
#else
	std::u32string out = U"";

	for (int i = 0; i < mutf.size(); i++) {
		unsigned char x, y;
		if (uint8_t(mutf[i]) <= 0x7F)
			out += mutf[i];
		else if ((mutf[i] & 0b11100000) == 0b11000000) {
			out += ((mutf[i] & 0b00011111) << 6) + (mutf[i + 1] & 0b00111111);
			i++;
		}
		else if (mutf[i] == 0b11101101) {
			out += 0x10000 + (uint32_t(mutf[i + 1] & 0x0f) << 16) + (uint32_t(mutf[i + 2] & 0x3f) << 10) + (uint32_t(mutf[i + 4] & 0x0f) << 6) + uint32_t(mutf[i + 5] & 0x3f);
			i += 5;
		}
		else if ((mutf[i] & 0b11110000) == 0b11100000) {
			out += uint32_t(uint32_t(mutf[i] & 0x0f) << 12) + uint32_t(uint32_t(mutf[i + 1] & 0x3f) << 6) + uint32_t(mutf[i + 2] & 0x3f);
			i += 2;
		}
	}

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	return converter.to_bytes(out);
#endif
}

std::map<int8_t, nbt::tag* (*)()> nbt::tagConstructors = std::map<int8_t, nbt::tag* (*)()>();


// Define the forwarded operators and functions from tag_p.
nbt::tag_p& nbt::tag_p::operator[](std::string key) {
	return _compound()[key.c_str()];
}
nbt::tag_p& nbt::tag_p::operator[](size_t index) {
	return _list()[index];
}
bool nbt::tag_p::has(std::string key) {
	return _compound().has(key.c_str());
}
nbt::compound& nbt::tag_p::_compound() { if (value->id != 10) throw invalid_tag_operator(value->id, 10); return *dynamic_cast<compound*>(value); }
nbt::list& nbt::tag_p::_list() { if (value->id != 9) throw invalid_tag_operator(value->id, 9); return *dynamic_cast<list*>(value); }

//size_t nbt::list::load(const char* const bytes, size_t offset) 
#endif