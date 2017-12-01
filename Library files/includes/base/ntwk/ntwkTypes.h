#pragma once
#include <PacketPriority.h>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <string>
#include "typeDefs.h"

namespace ntwk
{
	//Simple struct that is what the managers return when handleing packets.
	//Packet is the current packet being worked on.
	//Output is the return string for errors or just as an output.
	struct PacketReturn
	{
		RakNet::Packet* packet = nullptr;
		std::string output = "";
	};

	//Properties of the manager
	struct ManagerProperties
	{
		PacketPriority		priority = PacketPriority::HIGH_PRIORITY;
		PacketReliability	reliability = PacketReliability::RELIABLE_ORDERED;
		char				channel = (char)0;
		uint32_t			forceReceipt = 0;
		unsigned int		bufferSize = 100;
		unsigned int		shutdownTime = 300;
		bool				writePort = false;
		char				seperator = '|';
	};

	///Enum identifiers

	//Premade MessageID used throughout the Server and Client Managers
	enum NTWKMessageID
	{
		//Server base
		NTWK_ID_SVR_MESSAGE = ID_USER_PACKET_ENUM, //Server message to client

		//Client base
		NTWK_ID_CNT_BROADCAST,	//Client message to be sent to all users on the server
		NTWK_ID_CNT_DIRECT,		//Client message to be send to a given user on the server

		//Both
		NTWK_ID_CONNECTION_INFO, //Client sends name, Clients also reveive name to add to list
		NTWK_ID_CONNECTED_LIST, //List of connected clients for the client to keep track of
		NTWK_ID_DISCONNECTED_INFO, //Client sends name, Clients also receive name to remove from list
		NTWK_ID_DISCONNECTION_NOTICE, //
		
		NTWK_ID_USER_PACKET_ENUM //For the user to use.  Start your first enumeration at this value.
	};

	//DataTypes that can be used to structure packets that contain multiple kinds of data,
	//or could contain different kinds of data based on input.
	enum NTWKDataType
	{
		///Integral
		NTWK_DATATYPE_SHORT,		//signed short
		NTWK_DATATYPE_INT,			//signed int
		NTWK_DATATYPE_LONG,			//signed long
		NTWK_DATATYPE_LONGLONG,		//signed long long

		///Unsigned integral
		NTWK_DATATYPE_USHORT,		//unsigned short
		NTWK_DATATYPE_UINT,			//unsigned int
		NTWK_DATATYPE_ULONG,		//unsigned long
		NTWK_DATATYPE_ULONGLONG,	//unsigned long long

		///Character
		NTWK_DATATYPE_CHAR,			//signed char
		NTWK_DATATYPE_UCHAR,		//unsigned char
		NTWK_DATATYPE_WCHAR,		//wchar

		///Floating point
		NTWK_DATATYPE_FLOAT,		//float
		NTWK_DATATYPE_DOUBLE,		//double
		NTWK_DATATYPE_LONGDOUBLE,	//long double

		///Other types
		NTWK_DATATYPE_BOOL,			//bool
		NTWK_DATATYPE_STRING,		//std::string or char[] / char*
		NTWK_DATATYPE_USTRING,		//unsigned char[] / unsigned char*
		NTWK_DATATYPE_WSTRING,		//wchar_t[] / wchar_t*

		///Array data
		NTWK_DATATYPE_ARRAY,		//data[] (follow this with the size and the array type)
		NTWK_DATATYPE_VECTOR2,		//Vector2 (e.g. glm::vec2)
		NTWK_DATATYPE_VECTOR3,		//Vector3 (e.g. glm::vec3)
		NTWK_DATATYPE_VECTOR4,		//Vector4 (e.g. glm::vec4)

		NTWK_DATATYPE_USERTPYES		//Start here for identifying own types from this value (e.g. structs and classes).
	};

} //Networking