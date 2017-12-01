#include "ClientManager.h"
#include <ntwk/StaticFunctions.h>
#include <ntwk/ntwkTypes.h>
#include <BitStream.h>
#include <GetTime.h>
#include <StringCompressor.h>
#include <sstream>

//Initialize static variables
ntwk::ClientManager * ntwk::ClientManager::s_instance = nullptr;
bool ntwk::ClientManager::s_closed = false;

/** Public **/
//Static

ntwk::ClientManager * ntwk::ClientManager::Instance(void)
{
	return s_instance;
}
RakNet::StartupResult ntwk::ClientManager::Startup(const std::string& username)
{
	//Create instance if none exists
	if (!s_instance) s_instance = ntwk::ClientManager::CreateInstance();
	s_closed = false;

	//Initialize features
	s_instance->f_readData = ntwk::ClientManager::ReadData;
	s_instance->m_name = username.c_str();

	//Startup RakPeerInterface
	if (!s_instance->m_pPeerInterface)
	{
		s_instance->m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	}
	return s_instance->m_pPeerInterface->Startup(1, &s_instance->m_sd, 1);
}
bool ntwk::ClientManager::Search(unsigned short port, bool reply)
{
	if (!s_instance) s_instance = ntwk::ClientManager::CreateInstance();

	if (!s_instance->m_pPeerInterface)
	{
		s_instance->m_pPeerInterface;
	}

	return s_instance->m_pPeerInterface->Ping("255.255.255.255", port, reply);
}
int ntwk::ClientManager::SetConnection(const std::string & address)
{
	if (s_instance)
	{
		std::string ip = "";
		unsigned short port = 0;
		ntwk::GetIpFromFullAddress(address, &ip);
		ntwk::GetPortFromFullAddress(address, &port);
		return s_instance->m_pPeerInterface->Connect(ip.c_str(), port, nullptr, 0);
	}
	else return -1;
}
int ntwk::ClientManager::SetConnection(const std::string& ip, const unsigned short& port)
{
	if (s_instance) return s_instance->m_pPeerInterface->Connect(ip.c_str(), port, nullptr, 0);
	else return -1;
}
void ntwk::ClientManager::Shutdown(bool close)
{
	if (!s_closed)
	{
		s_instance->m_ignoreProcess = true;
		if (close)
		{
			s_closed = true;
			s_instance->m_pPeerInterface->CloseConnection(s_instance->m_host, true, s_instance->m_property->channel, s_instance->m_property->priority);
			delete s_instance;
			s_instance = nullptr;
		}
	}
}

//Normal functions
bool ntwk::ClientManager::beginProcess(void)
{
	if (m_ignoreProcess)
	{
		return false;
	}
	else
	{
		m_processing = true;
		return true;
	}
}
bool ntwk::ClientManager::isProcessing(void)
{
	return m_processing;
}
void ntwk::ClientManager::deallocate(RakNet::Packet * packet)
{
	m_pPeerInterface->DeallocatePacket(packet);
}
ntwk::PacketReturn ntwk::ClientManager::handleNextPacket(void)
{
	ntwk::PacketReturn pr;
	if (m_pPeerInterface && !m_ignoreProcess)
	{
		RakNet::Packet* packet = m_pPeerInterface->Receive();
		if (packet)
		{
			pr.packet = packet;
			checkPacket(&pr);
		}
		else
			m_processing = false;
	}
	else
		m_processing = false;

	return pr;
}
void ntwk::ClientManager::handleNextPacket(ntwk::PacketReturn * pr)
{
	if (m_pPeerInterface && !m_ignoreProcess)
	{
		RakNet::Packet* packet = m_pPeerInterface->Receive();
		if (packet)
		{
			pr->packet = packet;
			checkPacket(pr);
		}
		else
			m_processing = false;
	}
	else
		m_processing = false;
}
uint32_t ntwk::ClientManager::sendPacket(const RakNet::BitStream & bs) const
{
	return m_pPeerInterface->Send(&bs, m_property->priority, m_property->reliability, m_property->channel, m_host, false, m_property->forceReceipt);
}
uint32_t ntwk::ClientManager::sendPacket(const char * data, const int& length) const
{
	return m_pPeerInterface->Send(data, length, m_property->priority, m_property->reliability, m_property->channel, m_host, false, m_property->forceReceipt);
}
void ntwk::ClientManager::clearRemovedUsers(void)
{
	m_removedUsers.resize(0);
}
void ntwk::ClientManager::ping()
{
	m_pPeerInterface->Ping(m_pPeerInterface->GetSystemAddressFromGuid(m_host));
}


//Set functions
void ntwk::ClientManager::setIgnoreProcess(const bool& bol)
{
	m_ignoreProcess = bol;
}
void ntwk::ClientManager::setTimeoutTime(const RakNet::TimeMS& time, const RakNet::SystemAddress& address)
{
	m_pPeerInterface->SetTimeoutTime(time, address);
}
void ntwk::ClientManager::setTimeoutTimeUnreliable(const RakNet::TimeMS& time)
{
	m_pPeerInterface->SetUnreliableTimeout(time);
}
void ntwk::ClientManager::setPriority(const PacketPriority& pri)
{
	m_property->priority = pri;
}
void ntwk::ClientManager::setReliability(const PacketReliability& rel)
{
	m_property->reliability = rel;
}
void ntwk::ClientManager::setChannel(const char& cha)
{
	m_property->channel = cha;
}
void ntwk::ClientManager::setForceReceipt(const uint32_t& rec)
{
	m_property->forceReceipt = rec;
}
void ntwk::ClientManager::setWritePort(const bool& wp)
{
	m_property->writePort = wp;
}
void ntwk::ClientManager::setBufferSize(const unsigned int& size)
{
	m_property->bufferSize = size;
}
void ntwk::ClientManager::setSeperator(const char & sep)
{
	m_property->seperator = sep;
}
void ntwk::ClientManager::setReadFunction(ReadDataPtr rdp)
{
	f_readData = rdp;
}

//Get functions
const ntwk::ManagerProperties * ntwk::ClientManager::getProperties(void) const
{
	return m_property;
}
const RakNet::RakPeerInterface * ntwk::ClientManager::getPeerInterface(void) const
{
	return m_pPeerInterface;
}
const ntwk::ClientInfo * ntwk::ClientManager::getConnectedUsers(void) const
{
	if (m_connectedUsers.size() > 0)
		return m_connectedUsers.data();
	return nullptr;
}
const RakNet::RakString ntwk::ClientManager::getUsername(const unsigned int& index) const
{
	if (index < m_connectedUsers.size())
		return m_connectedUsers[index].username;
	return "";
}
const int ntwk::ClientManager::getUserIndex(const std::string & username) const
{
	for (unsigned int i = 0; i < m_connectedUsers.size(); i++)
	{
		if (ntwk::Compare2String(m_connectedUsers[i].username.C_String(), username))
			return i;
	}
		
	return -1;
}
const int ntwk::ClientManager::getUserIndex(const unsigned int& ID) const
{
	for (unsigned int i = 0; i < m_connectedUsers.size(); i++)
	{
		if (m_connectedUsers[i].id == ID)
			return i;
	}
	return -1;
}
const size_t ntwk::ClientManager::connectedUsersSize(void) const
{
	return m_connectedUsers.size();
}
const int ntwk::ClientManager::getOwnID(void) const
{
	return m_id;
}
const RakNet::RakString ntwk::ClientManager::getOwnName(void) const
{
	return m_name;
}
const unsigned int * ntwk::ClientManager::getRemovedUsers(void) const
{
	if (m_removedUsers.size() > 0)
		return m_removedUsers.data();
	return nullptr;
}
const int ntwk::ClientManager::removedUserSize(void) const
{
	return m_removedUsers.size();
}

/** Protected **/
//Constructor/Destructor/Operator
ntwk::ClientManager::ClientManager()
{
	m_connectedUsers.resize(0);
	m_property = new ntwk::ManagerProperties();
}
ntwk::ClientManager::~ClientManager()
{
	if (m_pPeerInterface) m_pPeerInterface = nullptr;
	if (m_property)
	{
		delete m_property;
		m_property = nullptr;
	}
}

//Static
ntwk::ClientManager * ntwk::ClientManager::CreateInstance(void)
{
	return new ClientManager();
}
bool ntwk::ClientManager::ReadData(RakNet::BitStream& bs, ntwk::PacketReturn* pr)
{
	char* data = new char[s_instance->m_property->bufferSize];
	if (bs.Read(data))
	{
		pr->output = data;
		delete data;
		return true;
	}
	delete data;
	return false;
}

//Functions
void ntwk::ClientManager::checkPacket(ntwk::PacketReturn * pr)
{
	RakNet::MessageID packetId = ntwk::GetPacketID(pr->packet);

	switch (packetId)
	{
	///Both of these have the same received packet structure
	///and also go through the same process when reading.
	case NTWK_ID_CNT_DIRECT:
	case NTWK_ID_CNT_BROADCAST:
	{
		/* Packet structure
		PacketID
		clientID
		data*/
		RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, false);

		if (f_readData(bsIn, pr)) break;
		else 
		{
			if (pr->output != "")
				pr->output += "\nPacket Read Error in f_readData!";
			else pr->output = "Packet Read Error in f_readData!";
		}
		break;
	}
	case NTWK_ID_SVR_MESSAGE:
	{
		/*Packet Structure
		PacketID
		Message*/
		RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, false);
		bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

		RakNet::RakString message;
		if (RakNet::StringCompressor::Instance()->DecodeString(&message, m_property->bufferSize, &bsIn)) pr->output = "Server: " + message;
		else pr->output = "Read Packet Error at line 315!";
		break;
	}
	case NTWK_ID_CONNECTION_INFO:
	{
		/*Packet Structure
		PacketID
		newClientID
		message*/
		RakNet::BitStream bs(pr->packet->data, pr->packet->length, false);
		bs.IgnoreBytes(sizeof(RakNet::MessageID));
		unsigned short id = 0;
		RakNet::RakString message;
		if (bs.Read(id))
		{
			if (RakNet::StringCompressor::Instance()->DecodeString(&message, m_property->bufferSize, &bs))
			{
				ClientInfo info;
				std::string mstr = message;
				
				info.id = (unsigned int)id;
				info.username = (mstr.substr(0, mstr.find_first_of(' '))).c_str();
				
				m_connectedUsers.push_back(info);
				pr->output = mstr;
			}
			else pr->output = "Read Packet Error at line 331!";
		}
		else pr->output = "Read Packet Error at line 329!";
		break;
	}
	case NTWK_ID_DISCONNECTED_INFO:
	{
		/*Packet Structure
		PacketID
		DisconnectedClientID
		Message*/
		RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, false);
		bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

		unsigned short disID;
		RakNet::RakString message;
		if (bsIn.Read(disID))
		{
			if (RakNet::StringCompressor::Instance()->DecodeString(&message, m_property->bufferSize, &bsIn))
			{
				pr->output = message;
				unsigned int i = 0;
				bool found = false;
				for (; i < m_connectedUsers.size(); i++)
				{
					if (m_connectedUsers[i].id == disID)
					{
						found = true;
						break;
					}
				}
				if (found)
				{
					m_removedUsers.push_back(m_connectedUsers[i].id);
					m_connectedUsers.erase(m_connectedUsers.begin() + i);
				}
				break;
			}
			else pr->output = "Packet Read Error at line 360!";
		}
		else pr->output = "Packet Read Error at line 358!";

		break;
	}
	case NTWK_ID_DISCONNECTION_NOTICE:
	{
		/*Packet Structure
		PacketID
		ReasonForClosure*/

		RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, false);
		bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
		
		RakNet::RakString reason;
		if (RakNet::StringCompressor::Instance()->DecodeString(&reason, m_property->bufferSize, &bsIn))
		{
			pr->output = reason;
			m_pPeerInterface->DeallocatePacket(pr->packet);
			pr->packet = nullptr;
			Shutdown(true);
			break;
		}
		else pr->output = "Packet Read Error at line 392!";
		break;
	}
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_REMOTE_DISCONNECTION_NOTIFICATION:
	{
		pr->output = "Server is shutting down.";
		m_pPeerInterface->DeallocatePacket(pr->packet);
		pr->packet = nullptr;
		Shutdown(true);
		break;
	}
	case ID_CONNECTED_PING:
	{
		pr->output = "";
		break;
	}
	case ID_CONNECTED_PONG:
	{
		pr->output = "Ping replied.";
		break;
	}
	case ID_CONNECTION_LOST:
	{
		pr->output = "Lost connection to server.";
		m_pPeerInterface->DeallocatePacket(pr->packet);
		pr->packet = nullptr;
		Shutdown(true);
		break;
	}
	case ID_UNCONNECTED_PONG:
	{
		//Broadcast found a server
		RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, false);
		bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
		
		RakNet::TimeMS time;
		if (bsIn.Read(time))
		{
			//Transform address to string
			char* address = new char[m_property->bufferSize];
			pr->packet->systemAddress.ToString(m_property->writePort, address, m_property->seperator);
			
			RakNet::TimeMS received = RakNet::GetTimeMS();

			int ping = int(received) - int(time);
			std::stringstream ss;
			ss << ping;

			//Build output
			pr->output = "Server found at: ";
			pr->output += address;
			pr->output += ", connection time: ";
			pr->output += ss.str();

			delete address;
		}
		else pr->output = "Error Reading Packet at line 414!";

		break;
	}
	case ID_CONNECTION_REQUEST_ACCEPTED:
	{
		/*Packet Structure
		PacketID
		username*/
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)NTWK_ID_CONNECTION_INFO);
		RakNet::StringCompressor::Instance()->EncodeString(m_name, m_property->bufferSize, &bs);

		m_host = pr->packet->guid;

		uint32_t i = sendPacket(bs);
		if (i == 0) pr->output = "Unable to send packet at line 425!";
		else pr->output = "Connecting...";
		break;
	}
	case ID_NO_FREE_INCOMING_CONNECTIONS:
	{
		pr->output = "Server is full.";
		m_processing = false;
		m_ignoreProcess = true;
		break;
	}
	case NTWK_ID_CONNECTED_LIST:
	{
		///Note with this packets structure the X, this is the number of elements (AmountOfClients).
		///Meaning that for 2 elements its Username0, UserID0, Username1, UserID1
		/*Packet Structure
		PacketID
		AmountOfClients
		UsernameX
		UserIDX*/
		RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, true);
		bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
		unsigned short amount = 0;
		if (bsIn.Read(amount))
		{
			RakNet::StringCompressor* compressor = RakNet::StringCompressor::Instance();
			m_connectedUsers.resize(amount);
			for (unsigned int i = 0; i < amount; i++)
			{
				unsigned short id;
				RakNet::RakString username;
				if (compressor->DecodeString(&username, m_property->bufferSize, &bsIn))
				{
					if (bsIn.Read(id))
					{
						m_connectedUsers[i].username = username;
						m_connectedUsers[i].id = (unsigned int)id;
						if (ntwk::Compare2String(m_connectedUsers[i].username.C_String(), m_name.C_String()))
						{
							//Do this as because it is skipped if m_id is set directrly
							unsigned int temp = unsigned int(id);
							m_id = temp;
						}
					}
					else
					{
						pr->output = "Packet Read Error at line 475!";
						break;
					}
				}
				else
				{
					pr->output = "Packet Read Error at line 473!";
					break;
				}
			}
		}
		else pr->output = "Packet Read Error at line 456!";
		break;
	}
	default:
	{
		pr->output = "Unknown packetID.";
		pr->output += packetId;
		break;
	}
	}
}
