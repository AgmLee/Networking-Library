#include "ServerManager.h"
#include <ntwk/ntwkTypes.h>
#include <ntwk/StaticFunctions.h>
#include <StringCompressor.h>
#include <BitStream.h>
#include <GetTime.h>
#include <fstream>
#include <iostream>

//Initialize static variables
ntwk::ServerManager * ntwk::ServerManager::s_instance = nullptr;
bool ntwk::ServerManager::s_closed = false;

/** Public **/
//Static
ntwk::ServerManager * ntwk::ServerManager::Instance()
{
	return s_instance;
}
int ntwk::ServerManager::Startup(const unsigned short& port, const std::string& filePath)
{
	//Create instance
	if (!s_instance) s_instance = CreateInstance();
	
	//Initialize variables
	s_instance->initializeDefaults(port);
	if (filePath != "") s_instance->m_configPath = filePath;

	//Load/create config
	if (!s_instance->loadConfig())
		s_instance->saveConfig();
	
	//Set up clientInfo array
	s_instance->m_connectedClients = new ntwk::ClientInfo[s_instance->m_maxConnections];

	//Setup connection
	s_instance->m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	s_instance->m_sd = RakNet::SocketDescriptor(s_instance->m_port, 0);
	
	//Check connection
	RakNet::StartupResult res = s_instance->m_pPeerInterface->Startup(s_instance->m_maxConnections, &s_instance->m_sd, 1);
	if (res == RakNet::StartupResult::RAKNET_STARTED)
	{
		s_instance->m_pPeerInterface->SetMaximumIncomingConnections(s_instance->m_maxConnections);
		s_closed = false; //Re-open if closed beforehand
	}
	
	return res;
}
void ntwk::ServerManager::Shutdown(bool shutdown, bool force)
{
	if (s_instance)
	{
		if (!s_closed)
		{
			if (shutdown)
			{
				s_closed = true;
				s_instance->m_ignoreProcess = true;
				if (!force) //Perform shutdown operations (e.g. saving config)
				{
					s_instance->shutdownOperations();
					s_instance->m_pPeerInterface->Shutdown(s_instance->m_property->shutdownTime);
				}
				else
				{
					s_instance->m_pPeerInterface->Shutdown(0);
				}
				
				RakNet::BitStream bs;
				bs.Write((RakNet::MessageID)ID_DISCONNECTION_NOTIFICATION);

				delete s_instance;
				s_instance = nullptr;
			}
			else s_instance->m_ignoreProcess = true;
		}
	}
}

//Normal functions
bool ntwk::ServerManager::beginProcess(void)
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
bool ntwk::ServerManager::isProcessing(void) const
{
	return m_processing;
}
void ntwk::ServerManager::deallocate(RakNet::Packet * packet)
{
	m_pPeerInterface->DeallocatePacket(packet);
}
ntwk::PacketReturn ntwk::ServerManager::handleNextPacket(void)
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
void ntwk::ServerManager::handleNextPacket(PacketReturn * pr)
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
uint32_t ntwk::ServerManager::broadcastPacket(const RakNet::BitStream& bs)
{
	return m_pPeerInterface->Send(&bs, m_property->priority, m_property->reliability, m_property->channel, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, m_property->forceReceipt);
}
uint32_t ntwk::ServerManager::broadcastPacket(const char * data, const int& length)
{
	return m_pPeerInterface->Send(data, length, m_property->priority, m_property->reliability, m_property->channel, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, m_property->forceReceipt);
}
uint32_t ntwk::ServerManager::sendPacket(const RakNet::BitStream & bs, const std::string & username, bool broadcast) const
{
	RakNet::RakNetGUID client;
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (ntwk::Compare2String(m_connectedClients[i].username.C_String(), username))
			return m_pPeerInterface->Send(&bs, m_property->priority, m_property->reliability, m_property->channel, m_connectedClients[i].id, broadcast, m_property->forceReceipt);
	}
	return 0;
}
uint32_t ntwk::ServerManager::sendPacket(const char * data, const int& length, const std::string & username, bool broadcast) const
{
	RakNet::RakNetGUID client;
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (ntwk::Compare2String(m_connectedClients[i].username.C_String(), username))
			return m_pPeerInterface->Send(data, length, m_property->priority, m_property->reliability, m_property->channel, m_connectedClients[i].id, broadcast, m_property->forceReceipt);
	}
	return 0;
}
uint32_t ntwk::ServerManager::sendPacket(const RakNet::BitStream & bs, unsigned int index, bool broadcast) const
{
	if (index < m_maxConnections) 
		return m_pPeerInterface->Send(&bs, m_property->priority, m_property->reliability, m_property->channel, m_connectedClients[index].id, broadcast, m_property->forceReceipt);
	else return 0;
}
uint32_t ntwk::ServerManager::sendPacket(const char * data, const int& length, unsigned int index, bool broadcast) const
{
	if (index < m_maxConnections) 
		return m_pPeerInterface->Send(data, length, m_property->priority, m_property->reliability, m_property->channel, m_connectedClients[index].id, broadcast, m_property->forceReceipt);
	else return 0;
}
void ntwk::ServerManager::saveConfig() const
{
	std::fstream file;
	file.open(m_configPath, std::ios_base::out);

	if (file.is_open())
	{
		file << "servername=" << m_serverName << std::endl;
		file << "port=" << m_port << std::endl;
		file << "maxcount=" << m_maxConnections << std::endl;
		file << "motd=" << m_motd.C_String() << std::endl;
	}
}
uint32_t ntwk::ServerManager::closeConnection(const std::string & username, const std::string & reason)
{
	unsigned int i = 0;
	RakNet::BitStream bs;
	for (; i < m_maxConnections; i++)
	{
		if (ntwk::Compare2String(m_connectedClients[i].username.C_String(), username))
		{
			/*Packet Structure
			PacketID
			ReasonForClosure*/
			bs.Write((RakNet::MessageID)NTWK_ID_DISCONNECTION_NOTICE);
			RakNet::RakString reasonRak = reason.c_str();
			RakNet::StringCompressor::Instance()->EncodeString(&reasonRak, m_property->bufferSize, &bs);
			break;
		}
	}

	return sendPacket(bs, i);
}
bool ntwk::ServerManager::ping(const std::string & username)
{
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (ntwk::Compare2String(m_connectedClients[i].username.C_String(), username))
		{
			m_pPeerInterface->Ping(m_pPeerInterface->GetSystemAddressFromGuid(m_connectedClients[i].id));
			return true;
		}
	}
	return false;
}
bool ntwk::ServerManager::ping(unsigned int ind)
{
	if (ind < m_maxConnections)
	{
		m_pPeerInterface->Ping(m_pPeerInterface->GetSystemAddressFromGuid(m_connectedClients[ind].id));
		return true;
	}
	return false;
}
void ntwk::ServerManager::ping(RakNet::SystemAddress address)
{
	m_pPeerInterface->Ping(address);
}
void ntwk::ServerManager::ban(unsigned int index)
{
	if(index < m_maxConnections)
	{
		char* buffer = new char[m_property->bufferSize];
		m_connectedClients[index].id.ToString(buffer);
		m_pPeerInterface->AddToBanList(buffer);
		std::fstream file;
		file.open("banlist.cfg", std::ios_base::app);
		if (file.is_open())
		{
			file << buffer << std::endl;
		}
		file.close();
		delete buffer;
		buffer = nullptr;
	}
}
void ntwk::ServerManager::ban(const std::string & ip)
{
	m_pPeerInterface->AddToBanList(ip.c_str());
	std::fstream file;
	file.open("banlist.cfg", std::ios_base::app);
	if (file.is_open())
	{
		file << ip.c_str() << std::endl;
	}
	file.close();
}
void ntwk::ServerManager::unban(const std::string & ip)
{
	m_pPeerInterface->RemoveFromBanList(ip.c_str());
	std::fstream file;
	std::vector<std::string> ips;
	file.open("banlist.cfg", std::ios_base::in);
	if (file.is_open())
	{
		char buff[30];
		while (file.getline(buff, 30))
		{
			std::string line = "";
			line = buff;
			if (!ntwk::Compare2String(ip, line))
			{
				ips.push_back(line);
			}
		}
	}
	file.close();
	file.open("banlist.cfg", std::ios_base::out);
	if (file.is_open())
	{
		file.clear();
		for (auto s : ips)
		{
			file << s << std::endl;
		}
	}
	file.close();
}

//Set functions
void ntwk::ServerManager::setIgnoreProcess(const bool& val)
{
	m_ignoreProcess = val;
}
void ntwk::ServerManager::setTimeoutTime(const RakNet::TimeMS& time, const RakNet::SystemAddress& address)
{
	m_pPeerInterface->SetTimeoutTime(time, address);
}
void ntwk::ServerManager::setTimeoutTimeUnreliable(const RakNet::TimeMS& time)
{
	m_pPeerInterface->SetUnreliableTimeout(time);
}
void ntwk::ServerManager::setPriority(const PacketPriority& pri)
{
	m_property->priority = pri;
}
void ntwk::ServerManager::setReliability(const PacketReliability& rel)
{
	m_property->reliability = rel;
}
void ntwk::ServerManager::setChannel(const char& cha)
{
	m_property->channel = cha;
}
void ntwk::ServerManager::setForceReceipt(const uint32_t& rec)
{
	m_property->forceReceipt = rec;
}
void ntwk::ServerManager::setWritePort(const bool & wp)
{
	m_property->writePort = wp;
}
void ntwk::ServerManager::setBufferSize(const unsigned int & size)
{
	m_property->bufferSize = size;
}
void ntwk::ServerManager::setSeperator(const char & sep)
{
	m_property->seperator = sep;
}

void ntwk::ServerManager::setReadDataFunction(ReadDataPtr ptr)
{
	f_readData = ptr;
}

//Get functions
const ntwk::ManagerProperties * ntwk::ServerManager::getProperties(void) const
{
	return m_property;
}
const RakNet::RakPeerInterface * ntwk::ServerManager::getPeerInterface(void) const
{
	return m_pPeerInterface;
}
const ntwk::ClientInfo * ntwk::ServerManager::getConnectedUsers(void) const
{
	return m_connectedClients;
}
const unsigned int ntwk::ServerManager::getIndexOfUser(const std::string & username) const
{
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (ntwk::Compare2String(m_connectedClients[i].username.C_String(), username))
			return i;
	}
	return m_maxConnections;
}
const RakNet::RakNetGUID ntwk::ServerManager::getIDFromUsername(const std::string& username) const
{
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (ntwk::Compare2String(m_connectedClients[i].username.C_String(), username))
			return m_connectedClients[i].id;
	}
	return RakNet::UNASSIGNED_RAKNET_GUID;
}
const RakNet::RakString ntwk::ServerManager::getUsernameFromID(const RakNet::RakNetGUID& id) const
{
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (m_connectedClients[i].id == id)
			return m_connectedClients[i].username;
	}
	return "";
}
const unsigned int ntwk::ServerManager::getConnectedCount(void) const
{
	return m_amountOfClients;
}
const unsigned int ntwk::ServerManager::getMaxUserCount(void) const
{
	return m_maxConnections;
}

/** Protected **/
//Constructor/Destructor/Operator
ntwk::ServerManager::ServerManager()
{
	m_property = new ntwk::ManagerProperties;
}
ntwk::ServerManager::~ServerManager()
{
	if (m_connectedClients)
	{
		delete[] m_connectedClients;
		m_connectedClients = nullptr;
	}
	if (m_pPeerInterface) m_pPeerInterface = nullptr;
	if (m_property)
	{
		delete m_property;
		m_property = nullptr;
	}
}

//Static
ntwk::ServerManager * ntwk::ServerManager::CreateInstance(void)
{
	return new ServerManager();
}
bool ntwk::ServerManager::ReadData(RakNet::BitStream & bs, ntwk::PacketReturn * pr)
{
	RakNet::MessageID id = ntwk::GetPacketID(pr->packet);
	ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
	switch (id)
	{
	case NTWK_ID_CNT_DIRECT:
	{
		/* Packet structure
		PacketID
		sendToID
		clientID
		data*/

		bs.IgnoreBytes(sizeof(RakNet::MessageID));

		//Extract Index of who to send to
		unsigned short sendToID;
		if (!bs.Read(sendToID))
		{
			pr->output = "Packet Read Error!\n";
			break;
		}

		//Build the out BitStream
		unsigned short clientID;
		if (!bs.Read(clientID))
		{
			pr->output = "Packet Read Error!\n";
			break;
		}

		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID)NTWK_ID_CNT_DIRECT);
		bsOut.Write(clientID);
		bsOut.Write(bs, bs.GetNumberOfUnreadBits());


		//Send the packet
		uint32_t i = instance->sendPacket(bsOut, unsigned int(sendToID));

		if (i != 0)
		{
			pr->output = "";
			return true;
		}
		else pr->output = "Packet Send Error!\n";
		break;
	}
	case NTWK_ID_CNT_BROADCAST:
	{
		/*Packet Structure
		PacketID
		ClientID
		data*/
		
		//Copy packet to be broadcast to the connected clients
		RakNet::BitStream bsOut(pr->packet->data, pr->packet->length, true);

		//Get the clientID
		bs.IgnoreBytes(sizeof(RakNet::MessageID));
		unsigned short clientID;
		if (!bs.Read(clientID))
		{
			pr->output = "Packet Read Error!\n";
			break;
		}

		//Send packets
		uint32_t i = instance->sendPacket(bsOut, unsigned int(clientID), true);

		if (i != 0)
		{
			pr->output = "";
			return true;
		}
		else pr->output = "Packet Send Error!\n";
		break;
	}
	default:
		pr->output = "Unknown Error!\n";
		break;
	}
	
	return false;
}

//Functions
void ntwk::ServerManager::loadVariable(const std::string & lineLower, const std::string & subString)
{
	if (lineLower.find("servername") != std::string::npos)
	{
		m_serverName = subString.c_str();
		return;
	}
	if (lineLower.find("port") != std::string::npos)
	{
		m_port = (unsigned short)std::strtoul(subString.c_str(), NULL, 0);
		return;
	}
	if (lineLower.find("maxcount") != std::string::npos)
	{
		m_maxConnections = (unsigned int)std::strtoul(subString.c_str(), NULL, 0);
		return;
	}
	if (lineLower.find("motd") != std::string::npos)
	{
		m_motd = subString.c_str();
		return;
	}
}
void ntwk::ServerManager::initializeDefaults(const unsigned short& port)
{
	f_readData = ntwk::ServerManager::ReadData;
	m_serverName = "Server";
	m_port = port;
	m_maxConnections = 16;
	m_configPath = "config.cfg";
	m_motd = "This is a server.";
	
	m_amountOfClients = 0;
}
void ntwk::ServerManager::checkPacket(ntwk::PacketReturn * pr)
{
	RakNet::MessageID packetID = ntwk::GetPacketID(pr->packet);
	switch (packetID)
	{
		case NTWK_ID_CNT_DIRECT:
		case NTWK_ID_CNT_BROADCAST:
		{
			RakNet::BitStream bs(pr->packet->data, pr->packet->length, false);
			if (!f_readData(bs, pr)) pr->output += "Packet Error in readData!";
			break;
		}
		case NTWK_ID_CONNECTION_INFO:
		{
			/*Packet Structure
			PacketID
			username*/
			RakNet::BitStream bsIn(pr->packet->data, pr->packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

			//Extract username
			RakNet::RakString username;
			RakNet::StringCompressor* compressor = RakNet::StringCompressor::Instance();

			if (compressor->DecodeString(&username, m_property->bufferSize, &bsIn))
			{
				int index = addClient(username, pr->packet->guid);
				if (index < 0)
				{
					pr->output = "Error, index return negative at line 471!";
					break;
				}
				if (m_amountOfClients > 1) //At least one other connected client
				{
					///Note how the packet doesn't include the username, this is because the username
					///is the first word in the message making it easy to extract and to not send redundent data.
					/*Packet Structure
					PacketID
					newClientID
					message*/
					RakNet::BitStream bs;
					bs.Write((RakNet::MessageID)NTWK_ID_CONNECTION_INFO);
				
					bs.Write(unsigned short(index));

					RakNet::RakString message = username;
					message += " joined the server.";

					compressor->EncodeString(&message, m_property->bufferSize, &bs);
				
					uint32_t i = sendPacket(bs, index, true);
					if (i != 0) pr->output = message;
					else 
					{
						pr->output = "Unable to send packet at line 495!";
						break;
					}
				}
				else //First client to connect
				{
					pr->output = username;
					pr->output += " joined the server.";
				}


				/*Packet Structure
				PacketID
				AmountOfClients
				UsernameX
				UserIDX*/

				RakNet::BitStream bsOut; //Send the new client the current list of users (includes them)

				bsOut.Write((RakNet::MessageID)NTWK_ID_CONNECTED_LIST);
				bsOut.Write((unsigned short)m_amountOfClients);
				for (unsigned int i = 0; i < m_maxConnections; i++)
				{
					if (m_connectedClients[i].username != "")
					{
						compressor->EncodeString(m_connectedClients[i].username, m_property->bufferSize, &bsOut);
						bsOut.Write(unsigned short(i)); //index
					}
				}

				uint32_t i = sendPacket(bsOut, index);
				if (i == 0) pr->output = "Unable to send packet at line 529!";
			}
			else pr->output = "Packet Read Error at line 467!";
			break;
		}
		case ID_NEW_INCOMING_CONNECTION:
		{
			break;
		}
		case ID_DISCONNECTION_NOTIFICATION:
		{
			bool found = false;
			for (unsigned int i = 0; i < m_maxConnections; i++)
			{
				if (m_connectedClients[i].id == pr->packet->guid)
				{
					if ((m_amountOfClients - 1) != 0) //If not the only client
					{
						/*Packet Structure
						PacketID
						DisconnectedClientID
						Message*/
						RakNet::BitStream bs;
						bs.Write((RakNet::MessageID)NTWK_ID_DISCONNECTED_INFO);

						bs.Write(unsigned short(i));

						RakNet::RakString message = m_connectedClients[i].username + " has disconnected.";
						RakNet::StringCompressor::Instance()->EncodeString(&message, m_property->bufferSize, &bs);

						uint32_t r = sendPacket(bs, i, true);
						if (r != 0) pr->output = message;
						else pr->output = "Unable to send packet at line 562!";
					}
					else pr->output = m_connectedClients[i].username + " has disconnected.";

					removeClient(i); //Remove from the array
					found = true;
					break;
				}
			}
			if(!found) pr->output = "Error, id at line 541 doesn't exist!";
			break;
		}
		case ID_CONNECTION_LOST:
		{
			if (m_amountOfClients != 0)
			{
				bool found = false;
				for (unsigned int i = 0; i < m_maxConnections; i++)
				{
					if (m_connectedClients[i].id == pr->packet->guid)
					{
						if ((m_amountOfClients) != 0) //If not the only client
						{
							/*Packet Structure
							PacketID
							DisconnectedClientID
							Message*/

							RakNet::BitStream bs;
							bs.Write((RakNet::MessageID)NTWK_ID_DISCONNECTED_INFO);

							bs.Write(unsigned short(i));

							RakNet::RakString message = m_connectedClients[i].username + " timed out.";
							RakNet::StringCompressor::Instance()->EncodeString(&message, m_property->bufferSize, &bs);

							uint32_t r = sendPacket(bs, i, true);
							if (r != 0) pr->output = message;
							else pr->output = "Unable to send packet at line 597!";
						}
						else pr->output = m_connectedClients[i].username + " timed out.";

						removeClient(i); //Remove from the array
						m_pPeerInterface->CloseConnection(pr->packet->guid, false);
						found = true;
						break;
					}
				}
				if (!found) pr->output = "Error, id at line 576 doesn't exist!";
			}
			break;
		}
		case ID_UNCONNECTED_PING:
		{
			pr->output = "";
			break;
		}
		case ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
		{
			/*Packet Structure
			PacketID
			TimeStamp*/

			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)ID_UNCONNECTED_PONG);
			bs.Write((RakNet::TimeMS)RakNet::GetTimeMS());
			
			m_pPeerInterface->Send(&bs, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE, 0, pr->packet->guid, false);

			pr->output = "";
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
		default:
		{
			pr->output = "Unknown packetID: ";
			pr->output += packetID;
			break;
		}
	}
}
void ntwk::ServerManager::shutdownOperations(void)
{
	saveConfig();
}
bool ntwk::ServerManager::loadConfig()
{
	std::fstream file;
	file.open(m_configPath, std::ios_base::in);

	if (file.is_open())
	{
		char buff[30];
		while (file.getline(buff, 30))
		{
			std::string line = "";
			std::string lower = "";
			line = buff;
			lower = StringToLower(line);
			std::string substring = line.substr(line.find('=') + 1, line.length());
			loadVariable(lower, substring);
		}
		return true;
	}
	return false;
}
void ntwk::ServerManager::removeClient(unsigned int ind)
{
	if (ind >= m_maxConnections || m_amountOfClients == 0)
	{
		std::cerr << "Error, tried to remove non-existent client: " << ind << std::endl;
		return;
	}
	m_connectedClients[ind].id = RakNet::UNASSIGNED_RAKNET_GUID;
	m_connectedClients[ind].username = "";
	m_amountOfClients--;
}
int ntwk::ServerManager::addClient(const RakNet::RakString& username, const RakNet::RakNetGUID& id)
{
	for (unsigned int i = 0; i < m_maxConnections; i++)
	{
		if (m_connectedClients[i].username == "")
		{
			m_connectedClients[i].username = username;
			m_connectedClients[i].id = id;
			
			m_amountOfClients++;
			
			return i;
		}
	}
	return -1;
}
