/*
	title:		SE3313 Lab 4 Solution
	author: 	Daniel Bailey (dbaile7@uwo.ca)
	date:		November 15, 2018
	brief:
		This is the accepted TA solution for lab 4. There are many
		different ways to complete this task so yours may look completely
		different. If you have any questions about what I did or why
		I did something, don't hesitate to ask (Email is up top).
*/

#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include <signal.h> //to kill the process

using namespace Sync;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;

	// Reference to boolean flag for terminating the thread
	bool& terminate;

	// Are we connected?
	bool connected = false;
	
	// Data to send to server
	ByteArray data;
	std::string data_str;
	int expectedLength = 0;
public:
	ClientThread(Socket& socket, bool& terminate)
	: socket(socket), terminate(terminate)
	{}

	~ClientThread()
	{}

	void TryConnect()
	{
		try
		{
			std::cout << "Connecting...";
			std::cout.flush();
			socket.Open();
			connected = true;
			std::cout << "OK" << std::endl;
		}
		catch (std::string exception)
		{
			std::cout << "FAIL (" << exception << ")" << std::endl;
			return;
		}
	}

	virtual long ThreadMain()
	{
		// Initially we need a connection
		while (true)
		{
			// Attempt to connect
			TryConnect();

			// Check if we are exiting or connection was established
			if (terminate || connected)
			{
				break;
			}

			// Try again every 5 seconds
			std::cout << "Trying again in 5 seconds" << std::endl;
			sleep(5);
		}
		
		// Spawn a child process because I didn't know how to handle the reading from the socket and the input from the user (they both blocked the terminal)
        pid_t pid = fork();

		if (pid == 0)  // If pid is 0, we are a child, perform out child process 
        {

			while (!terminate)
			{
				// We are connected, perform our operations
				//std::cout << "Please input your data (done to exit): ";
				std::cout.flush();
	
				// Get the data
				data_str.clear();
				std::getline(std::cin, data_str);
				data = ByteArray(data_str);
						
				expectedLength = data_str.size();
	
				// Must have data to send
				if (expectedLength == 0)
				{
					std::cout << "Cannot send no data!" << std::endl;
					continue;
				}
				else if (data_str == "done")
				{
					std::cout << "Closing the client..." << std::endl;
					terminate = true;
					break;
				}
	
				if (socket.Write(data) <= 0 )
				{
					std::cout << "Server failed to respond. Closing client..." << std::endl;
					terminate = true;
				}

			}
			kill(0, SIGKILL);
        }
        
		else
        {
			while (1) //it can be killed when 'done' is typed (so killed by the child)
			{
				socket.Read(data);
				data_str = data.ToString();
				if (data_str == "" ) //it means that the socket has been closed by the server (you can comment this if to see what happens when we close the server)
				{
					kill(0, SIGKILL);
				}
				//std::cout << "Server Response: " << data_str << std::endl;
				std::cout << data_str << std::endl;
			}

		}
		
		
		return 0;
	}
};

int main(void)
{
	// Welcome the user and try to initialize the socket
	//std::cout << "SE3313 Lab 4 Client" << std::endl;
	std::cout << "SE3313 Lab 4 Client: First enter your name please" << std::endl;

	// Create our socket
	Socket socket("127.0.0.1", 3000);
	//Socket socket("35.162.177.130", 3000);
	bool terminate = false;

	// Scope to kill thread
	{
		// Thread to perform socket operations on
		ClientThread clientThread(socket, terminate);

		while(!terminate)
		{
			// This will wait for 'done' to shutdown the server
			sleep(1);
		}
		
		// Wait to make sure the thread is cleaned up
	}
	
	// Attempt to close the socket
	try
	{
		socket.Close();
	}
	catch (...)
	{
		// We don't care if this failed because the application is exiting anyways.
	}

	return 0;
}
