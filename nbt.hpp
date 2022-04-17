#pragma once
#include <vector>
#include <map>
#include <exception>
#include <typeinfo>
#include <string>
#include <iostream>
#define NBT_END_TAG_NAME "\\NBT_END_TAG_NAME_CONSTANT"
namespace nbt {
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

	template <typename T>
	int toBytes(T in, char* out) {
		try {
			//*out = ((char*)&in);
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

	template <typename T>
	int fromBytes(char* in, T* out) {
		try {
#ifndef NBT_LITTLE_ENDIAN
			for (char i = 0; i < sizeof(T) / 2; i++)
				std::swap(in[i], in[sizeof(T) - 1 - i]);
#endif
			//#ifdef NBT_LITTLE_ENDIAN
			memcpy_s(out, sizeof(T), in, sizeof(T));
			//#endif
		}
		catch (std::exception e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}
		return 0;
	}

	class tag {
	public:
		// The NBT id of the 
		char id = -1;
		// The name of the tag
		std::string name = "";
		/// <summary>
		/// Load data to this tag from the list of bytes, starting at the given offset. Used by nbt::compound tags to load sub-tags
		/// </summary>
		/// <param name="bytes">List of NBT data. Equivelent to a decompressed .dat file.</param>
		/// <param name="offset">The position to start reading data from</param>
		/// <returns>Where the current tag's data ends, and the next tag's data begins.</returns>
		virtual size_t load(char* bytes, size_t offset = 0) = 0;
		/// <summary>
		/// Writes tag's data to an output buffer. Buffer is able to then be saved to a file or loaded by other tags.
		/// </summary>
		/// <param name="buffer">Where the tag's data will be written to, make sure it has appropriate space. If given nullptr, this function can be used to get the exact length required for a buffer (FASTER+SAFER TO USE WRITE WITH A LIST INSTEAD OF CHAR ARRAY)</param>
		/// <param name="offset">Where the tag should start writing data</param>
		/// <returns>Where the current tag's data ends, and the next tag's data should begin</returns>
		virtual size_t write(char* buffer, size_t offset) = 0;
		/// <summary>
		/// Writes tag's data to an extendable output buffer. Buffer is able to then be saved to a file or loaded by other tags.
		/// </summary>
		/// <param name="buffer">Where the tag's data will be written to, pushing back the end of the buffer.</param>
		/// <returns>Where the current tag's data ends, and the next tag's data should begin</returns>
		virtual size_t write(std::vector<char>& buffer) = 0;
		virtual std::vector<char> value_bytes() = 0;
		virtual void discard() = 0;
#ifdef NBT_COMPILE
		virtual std::string compilation(std::string regex = "") = 0;
		friend std::ostream& operator<< (std::ostream& left, tag *right) {
			left << right->compilation();
			return left;
		}
#endif
	protected:
		virtual char correct_tag() = 0;
		int writeDefault(char* buffer, size_t offset) {
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			uint16_t namelength = (uint16_t)name.length();
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			memcpy_s(&buffer[offset + 3], namelength, name.data(), namelength);
			return (int32_t)offset + 3 + namelength;
		}
		int writeDefault(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			uint16_t namelength = (uint16_t)name.length();
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			buffer.insert(buffer.end(), name.begin(), name.end());
			return (int32_t)buffer.size();
		}
		int loadDefault(char* bytes, size_t offset) {
			// Grabs the id of the tag
			id = bytes[offset];

			// Checks id against what the id of this tag type should be, and throw an exception if it's invalid
			if (id!=correct_tag() && id!=-1 && correct_tag()!=-1) {
				throw invalid_tag_id_exception(id,correct_tag());
			}
			id = correct_tag();

			// Get the length of the name
			uint16_t namelength = 0;
			fromBytes(&bytes[offset+1], &namelength);

			// Copy the name out of the bytes nbt data array
			name = std::string((const char*)&bytes[offset + 3], namelength);
			return (int32_t)offset + 3 + namelength;
		}
	};

	class end : public tag {
	public:
		end() {
			id = 0;
			name = "";
		}

		size_t load(char* bytes, size_t offset) {
			return offset + 1;
		}
		size_t write(std::vector<char>& buffer) {
			buffer.push_back(0);
			return buffer.size();
		}
		size_t write(char* buffer, size_t offset) {
			buffer[offset] = 0;
			return offset + 1;
		}
		std::vector<char> value_bytes() {
			return { 0 };
		}
		void discard() {
			name.clear();
		}
#ifdef NBT_COMPILE
		std::string compilation(std::string regex="") {
			return "END\n";
		};
		friend std::ostream& operator<< (std::ostream& left, end& right) {
			left << right.compilation();
			return left;
		}
#endif
	private:
		char correct_tag() { return 0; }
	};

	// Abstract class used for primive tags such as: IntTag, FloatTag, ByteTag, DoubleTag, etc.
	template <typename T>
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
		size_t load(char* bytes, size_t offset) {

			int off = loadDefault(bytes, offset);

			// Use primitive casting to convert the data (in bytes) to the data (as a primitive)
			// This allows us to use floating point types with primitive tag. The other way to convert to primitive (by using byte shifting "<<") will only convert to integer types
			fromBytes<T>(&bytes[off], &data);

			//Return where the next tag will start in bytes[]
 			return off + sizeof(T);

		}
		size_t write(char* buffer, size_t offset) {
			int off = writeDefault(buffer, offset);

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
			toBytes(data, &buffer[buffer.size()-sizeof(T)]);

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
			data = (T)0;
			name.clear();
		}

		// CONVERSION AND SYNTAX SIMPLIFICATION
		operator T() {
			return data;
		}
		operator T* () {
			return &data;
		}
		operator primitivetag<T>* () {
			return this;
		}

#ifdef NBT_COMPILE
		std::string compilation(std::string regex="") {
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
		friend std::ostream& operator<<(std::ostream& left, primitivetag<T>& right) {
			left << right.compilation();
			return left;
		}
#endif

	private:
		// Compares the current tag-id to what the tag is
		char correct_tag() {
			if (typeid(T) == typeid(int8_t) || typeid(T) == typeid(uint8_t))
				return 1;
			else if (typeid(T) == typeid(int16_t) || typeid(T) == typeid(uint16_t))
				return 2;
			else if (typeid(T) == typeid(int32_t) || typeid(T) == typeid(uint32_t))
				return 3;
			else if (typeid(T) == typeid(int64_t) || typeid(T) == typeid(uint64_t))
				return 4;
			else if (typeid(T) == typeid(float))
				return 5;
			else if (typeid(T) == typeid(double))
				return 6;
			else
				return -1; // Return -1 to allow for custom primitive tags if T isn't default tag type
		}
	};

	template <typename T>
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
		primitivearraytag(tag* tag) : primitivearraytag() {
			primitivearraytag<T>* t = dynamic_cast<primitivearraytag<T>*>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}
		size_t load(char* bytes, size_t offset) {
			int off = loadDefault(bytes, offset);

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
				//char* fb = 
				fromBytes<T>(&bytes[off], &data[i]);
				off += sizeof(T);
			}
			return off;
		}
		size_t write(char* buffer, size_t offset) {
			int off = writeDefault(buffer, offset);

			// Convert the length into byte form, and copy it to the buffer
			toBytes((uint32_t)data.size(), &buffer[off]);
			off += 4;
			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				toBytes(data[i], &buffer[off]);
				// Change offset for future use
				off+=sizeof(T); 
			}

			//Return where the next tag will start in buffer[]
			return off;
		}
		size_t write(std::vector<char>& buffer) {
			int off = writeDefault(buffer);
			// Free space for the output
			buffer.insert(buffer.end(), sizeof(T) * data.size() + 4, 0x00);
			// Convert the length into byte form, and copy it to the buffer
			toBytes((uint32_t)data.size(), &buffer[off]);
			off += 4;
			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				toBytes(data[i], &buffer[off]);
				// Change offset for future use
				off+=sizeof(T);
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
		void operator<<(T t) {
			data.push_back(t);
		}
		T operator[](size_t i) {
			return data[i];
		}
		operator std::vector<T>() {
			return data;
		}
		//operator T* () {
			//return &data[0];
		//}
#ifdef NBT_COMPILE
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
			for (unsigned int i = 0; i < data.size(); i++) {
				out += regex + "\t" + std::to_string(data[i]) + "\n";
			}
#endif
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, primitivearraytag<T>& right) {
			left << right.compilation();
			return left;
		}
#endif

	private:
		// Compares the current tag-id to what the tag is
		char correct_tag() {
			if (typeid(T) == typeid(int8_t) || typeid(T)==typeid(uint8_t))
				return 7;
			else if (typeid(T) == typeid(int32_t) || typeid(T) == typeid(uint32_t))
				return 11;
			else if (typeid(T) == typeid(int64_t) || typeid(T) == typeid(uint64_t))
				return 12;
			else
				return -1; // Return true to allow for custom primitive tags
		}
	};

	// Unsigned tags are just for storage and data purposes, it does not change the reading and writing code
	typedef primitivetag<int8_t> bytetag;			//1
	typedef primitivetag<uint8_t> ubytetag;

	typedef primitivetag<int16_t> shorttag;			//2
	typedef primitivetag<uint16_t> ushorttag;

	typedef primitivetag<int32_t> inttag;			//3
	typedef primitivetag<uint32_t> uinttag;

	typedef primitivetag<int64_t> longtag;			//4
	typedef primitivetag<uint64_t> ulongtag;

	typedef primitivetag<float> floattag;			//5

	typedef primitivetag<double> doubletag;

	typedef primitivearraytag<int8_t> bytearray;	//7
	typedef primitivearraytag<uint8_t> ubytearray;

	typedef primitivearraytag<int32_t> intarray;	//11
	typedef primitivearraytag<uint32_t> uintarray;

	typedef primitivearraytag<int64_t> longarray;	//12
	typedef primitivearraytag<uint64_t> ulongarray;

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
		stringtag(tag* tag) {
			stringtag* t = dynamic_cast<stringtag*>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}
		size_t load(char* bytes, size_t offset) {

			int off = loadDefault(bytes, offset); 

			// Load the size of the string
			uint16_t datlength = 0;
			fromBytes(&bytes[off], &datlength);
			// Copy the string out of the bytes nbt data array
			data = std::string((const char*)&bytes[off + 2], datlength);

			//Return where the next tag will start in bytes[]
			return off + 2 + datlength;

		}
		size_t write(char* buffer, size_t offset) {
			int off = writeDefault(buffer, offset);

			// Grab the name length and store it in the buffer
			uint16_t datlength = (uint16_t)data.length();
			buffer[offset + 1] = (datlength >> 8) & 0xff;
			buffer[offset + 2] = (datlength) & 0xff;

			// Copy the name into the buffer
			memcpy_s(&buffer[off + 2], datlength, data.data(), datlength);

			//Return where the next tag will start in buffer[]
			return off + 2 + data.size();
		}
		size_t write(std::vector<char>& buffer) {
			int off = writeDefault(buffer);

			// Grab the data length and store it in the buffer
			uint16_t datlength = (uint16_t)data.length();
			buffer.push_back((datlength >> 8) & 0xff);
			buffer.push_back((datlength) & 0xff);

			// Copy the data into the buffer
			buffer.insert(buffer.end(), data.begin(), data.end());
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
	private:
		char correct_tag() {
			return 8;
		}
	};

	class compound;
	class list;

	class tag_p {
	public:
		tag* value;
		tag_p() {
			value = nullptr;
		}
		tag_p(tag* value) : value(value) {}
		tag_p& operator [](std::string key);
		tag_p& operator [](size_t index);
		tag* operator->() {
			return value;
		}
		void discard() {
			value->discard();
			delete value;
			value = nullptr;
		}
		
		int8_t& _byte() { if (value->id != 1) throw invalid_tag_operator(value->id, 1); return dynamic_cast<bytetag*>(value)->data; }
		uint8_t& _ubyte() { if (value->id != 1) throw invalid_tag_operator(value->id, 1); return dynamic_cast<ubytetag*>(value)->data; }
		int16_t& _short() { if (value->id != 2) throw invalid_tag_operator(value->id, 2); return dynamic_cast<shorttag*>(value)->data; }
		uint16_t& _ushort() { if (value->id != 2) throw invalid_tag_operator(value->id, 2); return dynamic_cast<ushorttag*>(value)->data; }
		int32_t& _int() { if (value->id != 3) throw invalid_tag_operator(value->id, 3); return dynamic_cast<inttag*>(value)->data; }
		uint32_t& _uint() { if (value->id != 3) throw invalid_tag_operator(value->id, 3); return dynamic_cast<uinttag*>(value)->data; }
		int64_t& _long() { if (value->id != 4) throw invalid_tag_operator(value->id, 4); return dynamic_cast<longtag*>(value)->data; }
		uint64_t& _ulong() { if (value->id != 4) throw invalid_tag_operator(value->id, 4); return dynamic_cast<ulongtag*>(value)->data; }
		float& _float() { if (value->id != 5) throw invalid_tag_operator(value->id, 5); return dynamic_cast<floattag*>(value)->data; }
		double& _double() { if (value->id != 6) throw invalid_tag_operator(value->id, 6); return dynamic_cast<doubletag*>(value)->data; }
		std::vector<int8_t>& _bytearray() { if (value->id != 7) throw invalid_tag_operator(value->id, 7); return dynamic_cast<bytearray*>(value)->data; }
		std::vector<uint8_t>& _ubytearray() { if (value->id != 7) throw invalid_tag_operator(value->id, 7); return dynamic_cast<ubytearray*>(value)->data; }
		std::vector<int32_t>& _intarray() { if (value->id != 11) throw invalid_tag_operator(value->id, 11); return dynamic_cast<intarray*>(value)->data; }
		std::vector<uint32_t>& _uintarray() { if (value->id != 11) throw invalid_tag_operator(value->id, 11); return dynamic_cast<uintarray*>(value)->data; }
		std::vector<int64_t>& _longarray() { if (value->id != 12) throw invalid_tag_operator(value->id, 12); return dynamic_cast<longarray*>(value)->data; }
		std::vector<uint64_t>& _ulongarray() { if (value->id != 12) throw invalid_tag_operator(value->id, 12); return dynamic_cast<ulongarray*>(value)->data; }
		std::string& _string() { if (value->id != 8) throw invalid_tag_operator(value->id, 8); return dynamic_cast<stringtag*>(value)->data; }

		bytetag& _bytetag() { if (value->id != 1) throw invalid_tag_operator(value->id, 1); return *dynamic_cast<bytetag*>(value); }
		ubytetag& _ubytetag() { if (value->id != 1) throw invalid_tag_operator(value->id, 1); return *dynamic_cast<ubytetag*>(value); }
		shorttag& _shorttag() { if (value->id != 2) throw invalid_tag_operator(value->id, 2); return *dynamic_cast<shorttag*>(value); }
		ushorttag& _ushorttag() { if (value->id != 2) throw invalid_tag_operator(value->id, 2); return *dynamic_cast<ushorttag*>(value); }
		inttag& _inttag() { if (value->id != 3) throw invalid_tag_operator(value->id, 3); return *dynamic_cast<inttag*>(value); }
		uinttag& _uinttag() { if (value->id != 3) throw invalid_tag_operator(value->id, 3); return *dynamic_cast<uinttag*>(value); }
		longtag& _longtag() { if (value->id != 4) throw invalid_tag_operator(value->id, 4); return *dynamic_cast<longtag*>(value); }
		ulongtag& _ulongtag() { if (value->id != 4) throw invalid_tag_operator(value->id, 4); return *dynamic_cast<ulongtag*>(value); }
		floattag& _floattag() { if (value->id != 5) throw invalid_tag_operator(value->id, 5); return *dynamic_cast<floattag*>(value); }
		doubletag& _doubletag() { if (value->id != 6) throw invalid_tag_operator(value->id, 6); return *dynamic_cast<doubletag*>(value); }
		bytearray& _bytearraytag() { if (value->id != 7) throw invalid_tag_operator(value->id, 7); return *dynamic_cast<bytearray*>(value); }
		ubytearray& _ubytearraytag() { if (value->id != 7) throw invalid_tag_operator(value->id, 7); return *dynamic_cast<ubytearray*>(value); }
		intarray& _intarraytag() { if (value->id != 11) throw invalid_tag_operator(value->id, 11); return *dynamic_cast<intarray*>(value); }
		uintarray& _uintarraytag() { if (value->id != 11) throw invalid_tag_operator(value->id, 11); return *dynamic_cast<uintarray*>(value); }
		longarray& _longarraytag() { if (value->id != 12) throw invalid_tag_operator(value->id, 12); return *dynamic_cast<longarray*>(value); }
		ulongarray& _ulongarraytag() { if (value->id != 12) throw invalid_tag_operator(value->id, 12); return *dynamic_cast<ulongarray*>(value); }
		stringtag& _stringtag() { if (value->id != 8) throw invalid_tag_operator(value->id, 8); return *dynamic_cast<stringtag*>(value); }
		compound& _compound();
		list& _list();
#ifdef NBT_SHORTHAND
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

	class list : public tag {
	public:
		std::vector<tag_p> tags;
		char tag_type = -1;
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
		list(tag* tag) {
			list* t = dynamic_cast<list*>(tag);
			this->tags = t->tags;
			this->id = 9;
			this->tag_type = t->tag_type;
			this->name = t->name;
		}
		void clear() {
			this->tags.clear();
			name = "";
		}
		void discard() {
			name.clear();
			for (auto it = tags.begin(); it != tags.end();) {
				it->discard();
				it = tags.erase(it);
			}
		}
		size_t load(char* bytes, size_t offset);
		size_t write(char* buffer, size_t offset) {
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
			toBytes((uint32_t)tags.size(), &buffer[off+1]);

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

		// SYNTAX AND CODE SIMPLIFICATION

		tag_p operator[](size_t i) {
			return tags[i];
		}
		operator std::vector<tag_p>() {
			return tags;
		}
		//operator tag** () {
		//	return &tags[0];
		//}
		void operator<<(tag_p t) {
			if (tag_type == -1)
				tag_type = t->id;
			if (t->id != tag_type)
				throw illegal_list_tag_type(t->id);
			else
				tags.push_back(t);
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
	private:
		char correct_tag() {
			return 9;
		}
	};

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
		compound(std::map<std::string, tag_p> tags, std::string name) {
			this->tags = tags;
			this->name = name;
			id = 10;
		}
		compound(std::string name) {
			this->name = name;
			id = 10;
		}
		compound(std::string name, std::map<std::string, tag_p> tags) {
			this->tags = tags;
			this->name = name;
			id = 10;
		}
		compound(tag* tag) {
			compound* t = dynamic_cast<compound*>(tag);
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

		size_t load(char* bytes, size_t offset) {
			size_t off = loadDefault(bytes, offset);
			char t;
			tag* tag;
			while (true) {
				t = bytes[off];
				switch (t)
				{
				case 0:
					tags.insert(std::make_pair(NBT_END_TAG_NAME, new end()));
					return off+1;
				case 1:
#ifdef NBT_UINT_DEF
					tag = new ubytetag();
#else
					tag = new bytetag();
#endif
					break;
				case 2:
#ifdef NBT_UINT_DEF
					tag = new ushorttag();
#else
					tag = new shorttag();
#endif
					break;
				case 3:
#ifdef NBT_UINT_DEF
					tag = new uinttag();
#else
					tag = new inttag();
#endif
					break;
				case 4:
#ifdef NBT_UINT_DEF
					tag = new ulongtag();
#else
					tag = new longtag();
#endif
					break;
				case 5:
					tag = new floattag();
					break;
				case 6:
					tag = new doubletag();
					break;
				case 7:
#ifdef NBT_UINT_DEF
					tag = new ubytearray();
#else
					tag = new bytearray();
#endif
					break;
				case 8:
					tag = new stringtag();
					break;
				case 9:
					tag = new list();
					break;
				case 10:
					tag = new compound();
					break;
				case 11:
#ifdef NBT_UINT_DEF
					tag = new uintarray();
#else
					tag = new intarray();
#endif
					break;
				case 12:
#ifdef NBT_UINT_DEF
					tag = new ulongarray();
#else
					tag = new longarray();
#endif
					break;
				default:
					throw missing_tag_id_exception(t);
					return off;
				}
				//pick += (std::chrono::high_resolution_clock::now() - n).count();
				//n = std::chrono::high_resolution_clock::now();
				off = tag->load(bytes, off);
				//load += (std::chrono::high_resolution_clock::now() - n).count();
				//n = std::chrono::high_resolution_clock::now();
				tags.insert(std::make_pair(tag->name, tag));
				//push += (std::chrono::high_resolution_clock::now() - n).count();
				//n = std::chrono::high_resolution_clock::now();
			}
			return off+1;
		}
		size_t write(char* buffer, size_t offset) {
			size_t off = writeDefault(buffer, offset);

			//for (int i = 0; i < tags.size(); i++)
			//	off = tags[i]->write(buffer, off);
			for (auto i = tags.begin(); i != tags.end(); i++)
				off = i->second->write(buffer, off);
			if (tags.count(NBT_END_TAG_NAME) == 0) {
#ifdef NBT_THROW_ENDLESS
				throw endless_compound_exception();
#endif
				buffer[off++] = 0x00;
			}
			return off;
		}
		size_t write(std::vector<char>& buffer) {
			writeDefault(buffer);

			for (auto i = tags.begin(); i != tags.end(); i++)
				i->second->write(buffer);

			if (tags.count(NBT_END_TAG_NAME) == 0) {
#ifdef NBT_THROW_ENDLESS
				throw endless_compound_exception();
#endif
				buffer.push_back(0x00);
			}

			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> buffer = std::vector<char>();
			for (auto i = tags.begin(); i != tags.end(); i++)
				i->second.value->write(buffer);
			return buffer;
		}

		tag_p get(const char* name) {
			if(tags.count(name)==0)
				return nullptr;
			return tags[name];
		}

		void add(tag_p tag) {
			tags.insert(std::make_pair(tag->name, tag));
		}

		// SYNTAX AND CODE SIMPLIFICATION

		tag_p operator[](const char* name) {
			return get(name);
		}
		operator std::map<std::string, tag_p>() {
			return tags;
		}
		void operator<<(tag_p t) {
			tags.insert(std::make_pair(t->name, t));
		}
		size_t size() {
			return tags.size();
		}

#ifdef NBT_COMPILE
		std::string compilation(std::string regex = "") {
			std::string out = regex + "CompoundTag(" + std::string(name) + "): " + std::to_string(tags.size()) + " tags {\n";
			for(auto i = tags.begin(); i!=tags.end(); i++) {
				if (i->second->id == 0)
					continue;
				std::string tag = i->second->compilation(regex + "\t");
				out += tag;

			}
			out += regex + "}\n";
			return out;
		}
		friend std::ostream& operator<<(std::ostream& left, compound& right) {
			left << right.compilation();
			return left;
		}
#endif
	private:
		char correct_tag() {
			return 10;
		}
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

}
#ifdef NBT_INCLUDE
#undef NBT_INCLUDE
nbt::tag_p& nbt::tag_p::operator[](std::string key) {
	compound* comp = dynamic_cast<compound*>(value);
	return comp->tags[key];
}
nbt::tag_p& nbt::tag_p::operator[](size_t index) {
	list* lis = dynamic_cast<list*>(value);
	return lis->tags[index];
}
nbt::compound& nbt::tag_p::_compound() { if (value->id != 10) throw invalid_tag_operator(value->id, 10); return *dynamic_cast<compound*>(value); }
nbt::list& nbt::tag_p::_list() { if (value->id != 9) throw invalid_tag_operator(value->id, 9); return *dynamic_cast<list*>(value); }

size_t nbt::list::load(char* bytes, size_t offset) {
	size_t off = loadDefault(bytes, offset);
	tag_type = bytes[off];
	uint32_t length = 0;
	fromBytes(&bytes[++off], &length);
	off += 4;
	tag* tag;
	for (uint32_t i = 0; i < length; i++) {
		switch (tag_type)
		{
		case 0:
			throw illegal_list_tag_type(tag_type);
			return off;
		case 1:
#ifdef NBT_UINT_DEF
			tag = new ubytetag();
#else
			tag = new bytetag();
#endif
			break;
		case 2:
#ifdef NBT_UINT_DEF
			tag = new ushorttag();
#else
			tag = new shorttag();
#endif
			break;
		case 3:
#ifdef NBT_UINT_DEF
			tag = new uinttag();
#else
			tag = new inttag();
#endif
			break;
		case 4:
#ifdef NBT_UINT_DEF
			tag = new ulongtag();
#else
			tag = new longtag();
#endif
			break;
		case 5:
			tag = new floattag();
			break;
		case 6:
			tag = new doubletag();
			break;
		case 7:
#ifdef NBT_UINT_DEF
			tag = new ubytearray();
#else
			tag = new bytearray();
#endif
			break;
		case 8:
			tag = new stringtag();
			break;
		case 9:
			tag = new list();
			break;
		case 10:
			tag = new compound();
			break;
		case 11:
#ifdef NBT_UINT_DEF
			tag = new uintarray();
#else
			tag = new intarray();
#endif
			break;
		case 12:
#ifdef NBT_UINT_DEF
			tag = new ulongarray();
#else
			tag = new longarray();
#endif
			break;
		default:
			throw missing_tag_id_exception(tag_type);
			return off;
		}
		bytes[off - 3] = -1;
		bytes[off - 2] = 0x00;
		bytes[off - 1] = 0x00;
		off = tag->load(bytes, off - 3);
		tags.push_back(tag);
	}
	return off;
}
#endif
//#endif