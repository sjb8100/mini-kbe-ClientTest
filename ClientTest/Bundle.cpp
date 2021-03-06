
#include "Bundle.h"
#include "MemoryStream.h"
#include "Message.h"
#include "NetworkInterface.h"
#include "KBDebug.h"

Bundle::Bundle():
	pCurrPacket_(NULL),
	streams_(),
	numMessage_(0),
	messageLength_(0),
	//pMsgtype_(NULL),
	curMsgStreamIndex_(0)
{
	pCurrPacket_ = MemoryStream::createObject();
}

Bundle::~Bundle()
{
	MemoryStream::reclaimObject(pCurrPacket_);
	pCurrPacket_ = NULL;
}

Bundle* Bundle::createObject()
{
	return new Bundle();
}

void Bundle::reclaimObject(Bundle* obj)
{
	delete obj;
}

void Bundle::newMessage(Message* pMsg)
{
	fini(false);

	//pMsgtype_ = pMsg;
	numMessage_ += 1;

	//(*this) << pMsgtype_->id;

	//if (pMsgtype_->msglen == -1)
	//{
	//	uint16 lengseat = 0;
	//	(*this) << lengseat;
	//	messageLength_ = 0;
	//}

	curMsgStreamIndex_ = 0;
}

void Bundle::newMessage(uint8 mainCmd, uint8 subCmd)
{
	fini(false);

	//pMsgtype_ = pMsg;
	numMessage_ += 1;

	(*this) << mainCmd;
	(*this) << subCmd;
	//(*this) << pMsgtype_->id;

	//if (pMsgtype_->msglen == -1)
	//{
	//	uint16 lengseat = 0;
	//	(*this) << lengseat;
	//	messageLength_ = 0;
	//}

	curMsgStreamIndex_ = 0;
}


void Bundle::fini(bool issend)
{
	if (numMessage_ > 0)
	{
		//writeMsgLength();

		streams_.push_back(pCurrPacket_);
		pCurrPacket_ = MemoryStream::createObject();
	}

	if (issend)
	{
		numMessage_ = 0;
		//pMsgtype_ = NULL;
	}

	curMsgStreamIndex_ = 0;
}

void Bundle::send(NetworkInterface* pNetworkInterface)
{
	fini(true);

	if (pNetworkInterface->valid())
	{
		for (int i = 0; i<streams_.size(); ++i)
		{
			MemoryStream* stream = streams_[i];
			pNetworkInterface->send(stream);
		}
	}
	else
	{
		ERROR_MSG("Bundle::send(): networkInterface invalid!");
	}

	// 把不用的MemoryStream放回缓冲池，以减少垃圾回收的消耗
	for (int i = 0; i < streams_.size(); ++i)
	{
		MemoryStream::reclaimObject(streams_[i]);
	}

	streams_.clear();

	if(pCurrPacket_)
		pCurrPacket_->clear(true);

	// 我们认为，发送完成，就视为这个bundle不再使用了，
	// 所以我们会把它放回对象池，以减少垃圾回收带来的消耗，
	// 如果需要继续使用，应该重新Bundle.createObject()，
	// 如果外面不重新createObject()而直接使用，就可能会出现莫名的问题，
	// 仅以此备注，警示使用者。
	Bundle::reclaimObject(this);
}

void Bundle::writeMsgLength()
{
	//if (pMsgtype_->msglen != -1)
	//	return;

	MemoryStream* writePacket = pCurrPacket_;

	if (curMsgStreamIndex_ > 0)
	{
		writePacket = streams_[streams_.size() - curMsgStreamIndex_];
	}

	uint8* data = writePacket->data();
	data[2] = (uint8)(messageLength_ & 0xff);
	data[3] = (uint8)(messageLength_ >> 8 & 0xff);
}

void Bundle::checkStream(uint32 v)
{
	if (v > pCurrPacket_->space())
	{
		streams_.push_back(pCurrPacket_);
		pCurrPacket_ = MemoryStream::createObject();
		++curMsgStreamIndex_;
	}

	messageLength_ += v;
}

Bundle &Bundle::operator<<(uint8 value)
{
	checkStream(sizeof(uint8));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(uint16 value)
{
	checkStream(sizeof(uint16));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(uint32 value)
{
	checkStream(sizeof(uint32));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(uint64 value)
{
	checkStream(sizeof(uint64));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(int8 value)
{
	checkStream(sizeof(int8));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(int16 value)
{
	checkStream(sizeof(int16));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(int32 value)
{
	checkStream(sizeof(int32));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(int64 value)
{
	checkStream(sizeof(int64));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(float value)
{
	checkStream(sizeof(float));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(double value)
{
	checkStream(sizeof(double));
	(*pCurrPacket_) << value;
	return *this;
}

Bundle &Bundle::operator<<(bool value)
{
	checkStream(sizeof(int8));
	(*pCurrPacket_) << value;
	return *this;
}


void Bundle::appendBlob(const std::vector<uint8>& datas)
{
	uint32 len = (uint32)datas.size() + 4/*len size*/;

	checkStream(len);

	(*pCurrPacket_).appendBlob(datas);
}

void Bundle::appendUTF8String(const std::string& str)
{
	//FTCHARToUTF8 EchoStrUtf8(*str);
	//uint32 len = (uint32)EchoStrUtf8.Length() + 4/*len size*/;

	//checkStream(len);

	//(*pCurrPacket_).appendUTF8String(str);
}
