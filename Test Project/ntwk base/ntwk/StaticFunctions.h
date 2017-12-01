#pragma once
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <PacketPriority.h>
#include <string>
#include <vector>

namespace ntwk
{	
	///String related functions
	
	//Returns a copy of the string that is in lowercase only
	std::string StringToLower(const std::string&);
	//Gets a copy of the string that is in lowercase only
	void StringToLower(const std::string&, std::string*);
	//Returns a vector containing each word in the string
	std::vector<std::string> SplitString(const std::string&);
	//Fills the vector with each word in the string
	void SplitString(const std::string&, std::vector<std::string>*);

	///Network related functions
	
	//Compares 2 strings, used in some cases as normal string comparison failed
	bool Compare2String(const std::string& lhs, const std::string& rhs);
	//Gets the port from a full address
	const unsigned short GetPortFromFullAddress(const std::string&, const char& = '|');
	//Gets the IP address from a full address
	std::string GetIpFromFullAddress(const std::string&, const char& = '|');
	//Gets the Packet ID of the given Packet
	unsigned char GetPacketID(RakNet::Packet*);
	//Gets the port from a full address
	void GetPortFromFullAddress(const std::string&, unsigned short*, const char& = '|');
	//Gets the IP address from a full address
	void GetIpFromFullAddress(const std::string&, std::string*, const char& = '|');
	//Gets the Packet ID of the given Packet
	void GetPacketID(RakNet::Packet*, unsigned char*);

} //ntwk
