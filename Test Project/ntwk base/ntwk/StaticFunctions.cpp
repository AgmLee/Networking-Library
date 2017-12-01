#include "StaticFunctions.h"
#include "ntwkTypes.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>

///Return functions
const unsigned short ntwk::GetPortFromFullAddress(const std::string & fullAddress, const char & seperator)
{
	std::string cPort = fullAddress.substr(fullAddress.find(seperator) + 1, fullAddress.length());
	return (unsigned short)std::stoul(cPort.c_str());
}
std::string ntwk::GetIpFromFullAddress(const std::string & fullAddress, const char & seperator)
{
	std::string cIP = fullAddress.substr(0, fullAddress.find(seperator));
	return cIP.c_str();
}
unsigned char ntwk::GetPacketID(RakNet::Packet * packet)
{
	if ((unsigned char)packet->data[0] == ID_TIMESTAMP)
		return (unsigned char)packet->data[sizeof(unsigned char) + sizeof(unsigned long)];
	else
		return (unsigned char)packet->data[0];
}
std::string ntwk::StringToLower(const std::string & str)
{
	std::string lower = str;

	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	return lower;
}
std::vector<std::string> ntwk::SplitString(const std::string & str)
{
	std::istringstream iss(str);
	return std::vector<std::string>(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>());
}

///Set functions
void ntwk::GetPortFromFullAddress(const std::string & input, unsigned short * output, const char & seperator)
{
	std::string cPort = input.substr(input.find(seperator) + 1, input.length());
	*output = (unsigned short)std::stoul(cPort.c_str());
}
void ntwk::GetIpFromFullAddress(const std::string & input, std::string * output, const char & seperator)
{
	*output = input.substr(0, input.find(seperator));
}
void ntwk::StringToLower(const std::string & input, std::string * output)
{
	*output = input;
	std::transform(output->begin(), output->end(), output->begin(), ::tolower);
}
void ntwk::GetPacketID(RakNet::Packet * packet, unsigned char * rchar)
{
	if ((unsigned char)packet->data[0] == ID_TIMESTAMP) *rchar = (unsigned char)packet->data[sizeof(unsigned char) + sizeof(unsigned long)];
	else *rchar = (unsigned char)packet->data[0];
}
void ntwk::SplitString(const std::string & str, std::vector<std::string> * strVector)
{
	std::istringstream iss(str);
	*strVector = std::vector<std::string>(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>());
}

///Check functions
bool ntwk::Compare2String(const std::string & lhs, const std::string & rhs)
{
	size_t lcheck = lhs.length();
	size_t rcheck = rhs.length();

	if (lcheck == rcheck)
	{
		lcheck = lhs.find(rhs);
		if (lcheck == std::string::npos)
			return false;
		rcheck = rhs.find(lhs);
		if (rcheck == std::string::npos)
			return false;

		if (lcheck == rcheck)
			return true;
	}
	return false;
}
