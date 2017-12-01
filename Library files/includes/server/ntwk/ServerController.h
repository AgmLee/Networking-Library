#pragma once
#include <string>
#include <vector>

namespace ntwk
{
	///Though this class was made to help with controlling the ServerManger functions and settings,
	///This was designed in a way that makes it more of a Command-line controller.
	///Create a derived class and in 'compareToCommands' don't reference the ServerManager or a 
	///dirived class of it and it will still be usable for taking in an input and checking it to different commands.	
	class ServerController
	{
	public:
		///Make sure to overide the 'Instance' function when creating a dirived class.
		///Otherwise it will return a new ServerController and not a ServerController pointer to the child class.

		//Returns the current instance of ServerController, if none exists it creates one
		static ServerController * Instance(void);
		//Destroys the current instance of ServerController
		static void Destroy(void);
		//Ends processing, use outside of the class
		static void EndProcessing(void);

		//Splits input then sends it through compareToCommands and reacts accordingly
		std::string checkInput(const std::string&);
		//Continues the processing loop until it returns false
		bool isProcessing(void);
		//Used for ending the processing from outside of the class
		void setProcessing(bool);

	protected:
		ServerController(); //Default constructor
		virtual ~ServerController(); //Destructor

		//Removed constructors/operators
		ServerController(const ServerController&) = delete; //Copy constructor
		void operator=(const ServerController&) = delete; //Assignment opperator

		//Compares the split input to different commands, can use the full input as well
		virtual std::string compareToCommands(const std::vector<std::string>&, const std::string&);

		static ServerController * s_instance;
		
		///Set processing to false in compareToCommands as to end the process loop from inside of the class.
		bool m_processing = true; 
	};

} //ntwk

