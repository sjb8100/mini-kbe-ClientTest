// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "MemoryStream.h"

class NetworkInterface;
class MessageReader;

/*
	包接收模块(与服务端网络部分的名称对应)
	处理网络数据的接收
*/
class  PacketReceiver
{
public:
	PacketReceiver(NetworkInterface* pNetworkInterface);
	virtual ~PacketReceiver();

public:
	void process();
	bool processRecv(bool first);
protected:
	NetworkInterface* pNetworkInterface_;
	MessageReader* pMessageReader_;
	MemoryStream* pBuffer_;
};
