#pragma once
#include <BitStream.h>

namespace ntwk
{
	//Forward declarations
	struct PacketReturn;
	struct ManagerProperties;

	//Identifier for DataType when reading the data from a packet (similar to RakNet::MessageID).
	typedef unsigned char DataType;

	//Function pointer for reading data, should return false anytime the bitstream can't be read
	//Used to override the way the ClientManager reads data from 
	//CNT_BROADCAST and CNT_DIRECT packets, without the creation of a child class.
	typedef bool(*ReadDataPtr)(RakNet::BitStream&, ntwk::PacketReturn*);

} //ntwk