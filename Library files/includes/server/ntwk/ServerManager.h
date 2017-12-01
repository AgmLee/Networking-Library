#pragma once
#include <RakPeerInterface.h>
#include <RakString.h>
#include <string>
#include <vector>
#include <ntwk/typeDefs.h>

namespace ntwk 
{
	//Server side ClientInfo stuct.
	//Holds username of a user and the RakNetGUID of that user.
	//The GUID is uniqe to the client (being based on the instance of RakNet).
	struct ClientInfo
	{
		RakNet::RakNetGUID id = RakNet::UNASSIGNED_RAKNET_GUID;
		RakNet::RakString username = "";
	};

	class ServerManager
	{
	public:		
		///Statics
		
		//Creates the instance of the manager and sets up its connection, returns the StartupResult
		//If the file path is left blank, it will read/write from the local directory
		static int Startup(const unsigned short&, const std::string& = "");
		//Returns the current instance of the manager (nullptr if none)
		static ServerManager * Instance(void);
		//If true, closes the connection and destroys the current instance of the manager
		//If false, sets the instance to return false at beginProcess
		//If forcing, will ignore warning of users and ignore saving the config file
		static void Shutdown(bool, bool force = false);

		///Functions

		//Performs any sync required processes and returns if it will process or not
		bool beginProcess(void);
		//Returns if it is currently in a process loop
		bool isProcessing(void) const;
		//Deallocates the packet from the peerInterface
		void deallocate(RakNet::Packet*);
		//Handles next packet in the peerInterface
		PacketReturn handleNextPacket(void);
		//Handles next packet in the peerInterface
		void handleNextPacket(PacketReturn*);
		//Broadcast a packet to all connected users
		uint32_t broadcastPacket(const RakNet::BitStream&);
		//Broadcast a packet to all connected users
		uint32_t broadcastPacket(const char*, const int&);
		//Sends a packet based on the clients username
		//If bool is true, insteads broadcasts ignoring the client
		uint32_t sendPacket(const RakNet::BitStream&, const std::string&, bool = false) const;
		//Sends a packet based on the clients username
		//If bool is true, insteads broadcasts ignoring the client
		uint32_t sendPacket(const char*, const int&, const std::string&, bool = false) const;
		//Sends a packet based on the index number of the client
		//If bool is true, insteads broadcasts ignoring the client
		uint32_t sendPacket(const RakNet::BitStream&, unsigned int, bool = false) const;
		//Sends a packet based on the index number of the client
		//If bool is true, insteads broadcasts ignoring the client
		uint32_t sendPacket(const char*, const int&, unsigned int, bool = false) const;
		//Saves the value for the member variables into a configurable .cfg file
		virtual void saveConfig(void) const;
		//Closes a connection to the user
		uint32_t closeConnection(const std::string& username, const std::string& reason);
		//Pings the given client
		bool ping(const std::string&);
		//Pings a client at the given index
		bool ping(unsigned int);
		//Pings a given System Addess (use unassigned to ping all)
		void ping(RakNet::SystemAddress);
		//Ban the ip of a given users index
		void ban(unsigned int);
		//Ban the given ip
		void ban(const std::string&);
		//Unban the given ip
		void unban(const std::string&);

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
		//Sets the read data function
		void setReadDataFunction(ReadDataPtr);

		///Get functions

		//Returns the current properties settings
		const ManagerProperties * getProperties(void) const;
		//Returns a pointer to the peerInterface
		const RakNet::RakPeerInterface * getPeerInterface(void) const;
		//Returns the pointer to the connected clients
		const ClientInfo * getConnectedUsers(void) const;
		//Returns the index of a given username
		const unsigned int getIndexOfUser(const std::string&) const;
		//Returns the corrisponding ID of a username
		const RakNet::RakNetGUID getIDFromUsername(const std::string&) const;
		//Returns the corrisponding username of a ID
		const RakNet::RakString getUsernameFromID(const RakNet::RakNetGUID&) const;
		//Returns the current number of connected clients
		const unsigned int getConnectedCount(void) const;
		//Returns the maximum number of users
		const unsigned int getMaxUserCount(void) const;
		
	protected:		
		ServerManager(); //Default constructor
		virtual ~ServerManager(); //Destructor
		
		///Removed constructors/operators

		ServerManager(const ServerManager&) = delete; //Copy constructor
		void operator=(const ServerManager&) = delete; //Assignment opperator

		///Make sure to overide the 'CreateInstance' function when creating a dirived class.
		///Otherwise it will return a new ServerManager and not a ServerManager pointer to the child class.

		//Creates the instance of the manager
		static ServerManager * CreateInstance(void);
		//Default initialization of f_readData
		static bool ReadData(RakNet::BitStream&, ntwk::PacketReturn*);

		//Loads data from a string into a member variable
		virtual void loadVariable(const std::string& lineLower, const std::string& subStr);
		//Loads the default value for the member variables if not initialized (values based on defines in defins.h)
		virtual void initializeDefaults(const unsigned short&);
		//Checks the packet and provides an appropiate output
		virtual void checkPacket(PacketReturn*);
		//Operations to perform on a non-forced shutdown
		virtual void shutdownOperations(void);
		//Loads the config file, returns false if none exists
		bool loadConfig(void);
		//Removes a client from the array
		void removeClient(unsigned int);
		//Adds a client to the manager, returns the index in the array
		int addClient(const RakNet::RakString&, const RakNet::RakNetGUID&);

		//Instance of the manager
		static ServerManager * s_instance;
		//Used when forcing the server to close outside of its process loop
		static bool s_closed;

		RakNet::SocketDescriptor m_sd;
		RakNet::RakPeerInterface* m_pPeerInterface = nullptr;

		ClientInfo* m_connectedClients = nullptr;
		
		bool m_ignoreProcess = false;
		bool m_processing = false;
		
		ManagerProperties* m_property = nullptr;

		ReadDataPtr f_readData = nullptr;

		std::string m_configPath = "";
		unsigned int m_amountOfClients = 0;
		RakNet::RakString m_motd = "";
		RakNet::RakString m_serverName = "";
		unsigned short m_port = 0;
		unsigned int m_maxConnections = 0;
	};

} //ntwk