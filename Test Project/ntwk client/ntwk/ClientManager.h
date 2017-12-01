#pragma once
#include <RakPeerInterface.h>
#include <RakString.h>
#include <string>
#include <vector>
#include <ntwk/typeDefs.h>

namespace ntwk
{
	//Holds username of a user and a unsigned int which is the
	//index number of the client relative to the server manager.
	struct ClientInfo
	{
		unsigned int id = 0;
		RakNet::RakString username = "";
	};
		
	class ClientManager
	{
	public:
		///Statics

		//Starts up the manager, doesn't set up the connection
		//Returns the startup result
		static RakNet::StartupResult Startup(const std::string&);
		//Searches for any servers using the port, returns if it successfully pinged
		static bool Search(unsigned short, bool = true);
		//Starts a connection to the given address
		//Returns the result of Connect or -1 for no instance
		//IP address and Port (seperated)
		static int SetConnection(const std::string&, const unsigned short&);
		//Starts a connection to the given address
		//Returns the result of Connect or -1 for no instance
		//IP address and Port (combined)
		static int SetConnection(const std::string&);
		//Returns the current instance of the manager (nullptr if none)
		static ClientManager * Instance(void);
		//Stops the server from processing, if true, will shutdown and delete the instance
		static void Shutdown(bool);

		///Functions

		//Performs any sync required processes and returns if it will process or not
		bool beginProcess(void);
		//Returns if it is currently in a process loop
		bool isProcessing(void);
		//Deallocates the packet from the peerInterface
		void deallocate(RakNet::Packet*);
		//Handles next packet in the peerInterface
		PacketReturn handleNextPacket(void);
		//Handles next packet in the peerInterface
		void handleNextPacket(PacketReturn*);
		//Sends a packet to the server writing a bitstream
		uint32_t sendPacket(const RakNet::BitStream&) const;
		//Sends a packet to the server writing data
		uint32_t sendPacket(const char*, const int&) const;
		//Clears the removed users array
		void clearRemovedUsers(void);
		//Pings a given address (use unassigned to ping all)
		void ping(RakNet::SystemAddress);
		//Pings the server it is connected to
		void ping(void);

		///Set functions

		//Set if the manager should ignore processing
		void setIgnoreProcess(const bool&);
		//Set the timeout time for the packets sent
		void setTimeoutTime(const RakNet::TimeMS&, const RakNet::SystemAddress& = RakNet::UNASSIGNED_SYSTEM_ADDRESS);
		//Set the timeout time for unreliable packets sent
		void setTimeoutTimeUnreliable(const RakNet::TimeMS&);
		//Sets the priority for when sending packets
		void setPriority(const PacketPriority&);
		//Sets the reliability for when sending packets
		void setReliability(const PacketReliability&);
		//Sets the channel for when sending packets
		void setChannel(const char&);
		//Sets the force receipt for when sending packets
		void setForceReceipt(const uint32_t&);
		//Sets weather to write out the port when refering to system adderesses
		void setWritePort(const bool&);
		//Sets the size of the buffers when reading / writing to the bitstream
		void setBufferSize(const unsigned int&);
		//Sets the character used to seperate the port from the address (when combined)
		void setSeperator(const char&);
		//Sets the function to use when reading packets with the following IDs
		//NTWK_ID_CNT_DIRECT
		//NTWK_ID_CNT_BROADCAST
		void setReadFunction(ReadDataPtr);

		///Get functions

		//Returns the current properties settings
		const ManagerProperties * getProperties(void) const;
		//Returns a pointer to the peerInterface
		const RakNet::RakPeerInterface * getPeerInterface(void) const;
		//Returns the connected users array
		const ClientInfo * getConnectedUsers(void) const;
		//Returns the username at the given index
		const RakNet::RakString getUsername(const unsigned int&) const;
		//Returns the index of the given username
		const int getUserIndex(const std::string&) const;
		//Returns the index of the given user ID
		const int getUserIndex(const unsigned int&) const;
		//Returns the length of the connected users array
		const size_t connectedUsersSize(void) const;
		//Returns the id of the client
		const int getOwnID(void) const;
		//Returns the name of the client
		const RakNet::RakString getOwnName(void) const;
		//Returns a list of user IDs that have lost connection
		const unsigned int * getRemovedUsers(void) const;
		//Returns the size of the removed users array
		const int removedUserSize(void) const;


	protected:
		ClientManager(); //Default constructor
		virtual ~ClientManager(); //Destructor

		///Removed constructors/operators

		ClientManager(const ClientManager&) = delete; //Copy constructor
		void operator=(const ClientManager&) = delete; //Assignment opperator

		///Make sure to overide the 'CreateInstance' function when creating a dirived class.
		///Otherwise it will return a new ClientManager and not a ClientManager pointer to the child class.

		//Creates the instance of the manager
		static ClientManager * CreateInstance(void);

		//Checks the packet and provides an appropiate output
		virtual void checkPacket(PacketReturn*);
		//Default initialization of f_readData, reads data as a char* and sets the PacketReturn output to it
		static bool ReadData(RakNet::BitStream&, ntwk::PacketReturn*);

		//Instance of the manager
		static ClientManager * s_instance;
		//Used when forcing the server to close outside of its process loop
		static bool s_closed;

		ReadDataPtr f_readData;
		
		RakNet::RakNetGUID m_host;
		RakNet::SocketDescriptor m_sd;
		RakNet::RakPeerInterface* m_pPeerInterface = nullptr;
		
		std::vector<ClientInfo> m_connectedUsers;
		std::vector<unsigned int> m_removedUsers;

		bool m_ignoreProcess = false;
		bool m_processing = false;
		
		int m_id = -1;
		RakNet::RakString m_name = "";
		
		ManagerProperties* m_property = nullptr;
	};

} //ntwk
