#include <ntwk/ServerManager.h>
#include <ntwk/ServerController.h>
#include <iostream>
#include <thread>
#include <ntwk/ntwkTypes.h>
#include <atomic>
#include <ntwk/StaticFunctions.h>

using namespace ntwk;

static std::atomic<bool> threadClosed = false; 
ServerManager* manager = nullptr;
ServerController* controller = nullptr;

int ControllerThreadFunction()
{
	std::string input;
	std::string output;
	
	//ServerController Loop
	while (controller->isProcessing())
	{
		std::getline(std::cin, input);
		output = controller->checkInput(input);
		std::cout << output;
	}
	
	//Clean up the server controller
	ServerController::Destroy();
	controller = nullptr;
	threadClosed = true;
	return 0;
}

int main()
{	
	//Create a new instance of ServerManager using the default settings
	{	
		std::cout << "STARTING SERVER...\n\n";
		int res = ServerManager::Startup(41014);
		if (res != RakNet::StartupResult::RAKNET_STARTED)
		{
			std::cout << "Unable to start connection, error code: " << res << std::endl;
			return res;
		}
		std::cout << "Server Started Successfully!\n\n";
	}
	manager = ServerManager::Instance();
	controller = ServerController::Instance();
	
	//Setup the ServerController as a thread.
	std::thread ControllerThread(ControllerThreadFunction);
	
	ControllerThread.detach();

	//ServerManager loop
	while (ServerManager::Instance())
	{
		if (manager->beginProcess())
		{
			while (manager->isProcessing())
			{
				PacketReturn pr = manager->handleNextPacket();

				if (pr.output != "")
					std::cout << pr.output << "\n\n";
				if (pr.packet)
					manager->deallocate(pr.packet);
			}
		}
		else
		{
			ServerManager::Shutdown(true);
		}
	}

	manager = nullptr;

	while (!threadClosed); //Loop to sync threads as too avoid errors
	return 0;
}