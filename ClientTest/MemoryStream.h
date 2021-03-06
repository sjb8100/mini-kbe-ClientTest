// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

namespace MemoryStreamConverter
{
	template <class T> 
	void swap(T& a, T& b)
	{
		T c(a); a = b; b = c;
	}

	template<size_t T>
	inline void convert(char *val)
	{
		swap(*val, *(val + T - 1));
		convert<T - 2>(val + 1);
	}

	template<> inline void convert<0>(char *) {}
	template<> inline void convert<1>(char *) {}            // ignore central byte

	template<typename T> inline void apply(T *val)
	{
		convert<sizeof(T)>((char *)(val));
	}

	inline void convert(char *val, size_t size)
	{
		if (size < 2)
			return;

		swap(*val, *(val + size - 1));
		convert(val + 1, size - 2);
	}
}

template<typename T> inline void EndianConvert(T& val) 
{ 
	if(false/*!FGenericPlatformProperties::IsLittleEndian()*/)
		MemoryStreamConverter::apply<T>(&val); 
}

template<typename T> inline void EndianConvertReverse(T& val) 
{
	if (true/*FGenericPlatformProperties::IsLittleEndian()*/)
		MemoryStreamConverter::apply<T>(&val);
}

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(uint8&) { }
inline void EndianConvert(int8&) { }
inline void EndianConvertReverse(uint8&) { }
inline void EndianConvertReverse(int8&) { }

/*
	二进制数据流模块
	能够将一些基本类型序列化(writeXXX)成二进制流同时也提供了反序列化(readXXX)等操作
*/
class  MemoryStream
{
public:
	union PackFloatXType
	{
		float	fv;
		uint32	uv;
		int		iv;
	};

public:
	const static size_t DEFAULT_SIZE = 0x100;

	MemoryStream() :
		rpos_(0), 
		wpos_(0),
		data_()
	{
		data_resize(DEFAULT_SIZE);
	}

	virtual ~MemoryStream()
	{
		clear(false);
	}

	static MemoryStream* createObject();
	static void reclaimObject(MemoryStream* obj);

public:
	uint8* data() {
		return (uint8*)&data_[0];
	}

	void clear(bool clearData)
	{
		if (clearData)
			data_.size();

		rpos_ = wpos_ = 0;
	}

	// array的大小
	virtual uint32 size() const { return data_.size(); }

	// array是否为空
	virtual bool empty() const { return data_.size() == 0; }

	// 读索引到与写索引之间的长度
	virtual uint32 length() const { return rpos() >= wpos() ? 0 : wpos() - rpos(); }

	// 剩余可填充的大小
	virtual uint32 space() const { return wpos() >= size() ? 0 : size() - wpos(); }

	// 将读索引强制设置到写索引，表示操作结束
	void done() { read_skip(length()); }

	void data_resize(uint32 newsize)
	{
		KBE_ASSERT(newsize <= 1310700);
		data_.resize(newsize);
	}

	void resize(uint32 newsize)
	{
		KBE_ASSERT(newsize <= 1310700);
		data_.resize(newsize);
		rpos_ = 0;
		wpos_ = size();
	}

	void reserve(uint32 ressize)
	{
		KBE_ASSERT(ressize <= 1310700);

		if (ressize > size())
			data_.reserve(ressize);
	}

	uint32 rpos() const { return rpos_; }

	uint32 rpos(int rpos)
	{
		if (rpos < 0)
			rpos = 0;

		rpos_ = rpos;
		return rpos_;
	}

	uint32 wpos() const { return wpos_; }

	uint32 wpos(int wpos)
	{
		if (wpos < 0)
			wpos = 0;

		wpos_ = wpos;
		return wpos_;
	}

	uint8 operator[](uint32 pos)
	{
		return read<uint8>(pos);
	}

	MemoryStream &operator<<(uint8 value)
	{
		append<uint8>(value);
		return *this;
	}

	MemoryStream &operator<<(uint16 value)
	{
		append<uint16>(value);
		return *this;
	}

	MemoryStream &operator<<(uint32 value)
	{
		append<uint32>(value);
		return *this;
	}

	MemoryStream &operator<<(uint64 value)
	{
		append<uint64>(value);
		return *this;
	}

	MemoryStream &operator<<(int8 value)
	{
		append<int8>(value);
		return *this;
	}

	MemoryStream &operator<<(int16 value)
	{
		append<int16>(value);
		return *this;
	}

	MemoryStream &operator<<(int32 value)
	{
		append<int32>(value);
		return *this;
	}

	MemoryStream &operator<<(int64 value)
	{
		append<int64>(value);
		return *this;
	}

	MemoryStream &operator<<(float value)
	{
		append<float>(value);
		return *this;
	}

	MemoryStream &operator<<(double value)
	{
		append<double>(value);
		return *this;
	}

	MemoryStream &operator<<(const char *str)
	{
		append((uint8 const *)str, str ? strlen(str) : 0);
		append((uint8)0);
		return *this;
	}

	MemoryStream &operator<<(bool value)
	{
		append<int8>(value);
		return *this;
	}

	MemoryStream &operator>>(bool &value)
	{
		value = read<char>() > 0 ? true : false;
		return *this;
	}

	MemoryStream &operator>>(uint8 &value)
	{
		value = read<uint8>();
		return *this;
	}

	MemoryStream &operator>>(uint16 &value)
	{
		value = read<uint16>();
		return *this;
	}

	MemoryStream &operator>>(uint32 &value)
	{
		value = read<uint32>();
		return *this;
	}

	MemoryStream &operator>>(uint64 &value)
	{
		value = read<uint64>();
		return *this;
	}

	MemoryStream &operator>>(int8 &value)
	{
		value = read<int8>();
		return *this;
	}

	MemoryStream &operator>>(int16 &value)
	{
		value = read<int16>();
		return *this;
	}

	MemoryStream &operator>>(int32 &value)
	{
		value = read<int32>();
		return *this;
	}

	MemoryStream &operator>>(int64 &value)
	{
		value = read<int64>();
		return *this;
	}

	MemoryStream &operator>>(float &value)
	{
		value = read<float>();
		return *this;
	}

	MemoryStream &operator>>(double &value)
	{
		value = read<double>();
		return *this;
	}

	void read_skip(uint32 skip)
	{
		KBE_ASSERT(skip <= length());
		rpos_ += skip;
	}

	template <typename T> T read()
	{
		T r = read<T>(rpos_);
		rpos_ += sizeof(T);
		return r;
	}

	template <typename T> T read(uint32 pos) 
	{
		KBE_ASSERT(sizeof(T) <= length());

		T val;
		memcpy((void*)&val, (data() + pos), sizeof(T));

		EndianConvert(val);
		return val;
	}

	void read(uint8 *dest, uint32 len)
	{
		if(len <= length()) return;

		memcpy(dest, data() + rpos_, len);
		rpos_ += len;
	}

	uint32 readBlob(std::vector<uint8>& datas)
	{
		if (length() <= 0)
			return 0;

		uint32 rsize = 0;
		(*this) >> rsize;
		if ((uint32)rsize > length())
			return 0;

		if (rsize > 0)
		{
			datas.resize(rsize);
			memcpy((uint8*)&datas[0], data() + rpos(), rsize);
			read_skip(rsize);
		}

		return rsize;
	}

	//uint32 readUTF8String(std::string& str)
	//{
	//	if (length() <= 0)
	//		return 0;

	//	std::vector<uint8> datas;
	//	uint32 rsize = readBlob(datas);

	//	// 结尾标志
	//	datas.Add(0);

	//	str = std::string(UTF8_TO_TCHAR(datas.GetData()));
	//	return rsize;
	//}

	template <typename T> void append(T value)
	{
		EndianConvert(value);
		append((uint8 *)&value, sizeof(value));
	}

	template<class T> void append(const T *src, uint32 cnt)
	{
		return append((const uint8 *)src, cnt * sizeof(T));
	}

	void append(const uint8 *src, uint32 cnt)
	{
		if (!cnt)
			return;

		KBE_ASSERT(size() < 10000000);

		if (size() < wpos_ + cnt)
			data_resize(wpos_ + cnt);

		memcpy((void*)&data()[wpos_], src, cnt);
		//for (size_t i = 0; i < cnt; i++)
		//{
		//	data_[wpos_ + i] = src[i];
		//}
		wpos_ += cnt;
	}

	void append(const uint8* datas, uint32 offset, uint32 size)
	{
		append(datas + offset, size);
	}

	void append(MemoryStream& stream)
	{
		wpos(stream.wpos());
		rpos(stream.rpos());
		data_resize(stream.size());
		memcpy(data(), stream.data(), stream.size());
	}

	void appendBlob(const std::vector<uint8>& datas)
	{
		uint32 len = (uint32)datas.size();
		(*this) << len;

		if (len > 0)
			append((const uint8*)&datas[0], len);
	}

	//void appendUTF8String(const std::string& str)
	//{
	//	//FTCHARToUTF8 EchoStrUtf8(*str);
	//	//uint32 len = (uint32)EchoStrUtf8.Length();
	//	//(*this) << len;
	//	//if (len > 0)
	//	//	append(TCHAR_TO_UTF8(*str), len);
	//}
	/** 输出流数据 */
	void print_storage();

protected:
	uint32 rpos_;
	uint32 wpos_;
	std::vector<uint8> data_;
};
