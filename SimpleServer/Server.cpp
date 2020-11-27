#include <iostream>
#include <net_includes.h>
#include <net_server.h>
#include <net_common.h>
#include <net_connection.h>
#include <net_client.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};



class Server :public net::net_server<CustomMsgTypes> {
public:
	Server(uint16_t nPort) : net::net_server<CustomMsgTypes>(nPort)
	{

	}

protected:
	virtual bool client_connect_handle(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual bool client_disconnect_handle(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->get_id() << "]\n";
		return true;
	}

	// Called when a message arrives
	virtual bool received_message_handle(std::shared_ptr<net::connection<CustomMsgTypes>> client, net::message<CustomMsgTypes>& msg) override
	{
		switch (msg.header.id)
		{
			case CustomMsgTypes::ServerPing:{
				std::cout << "[" << client->get_id() << "]: Server Ping\n";

				// Simply bounce message back to client
				client->send(msg);
			}

			case CustomMsgTypes::MessageAll:{
				std::cout << "[" << client->get_id() << "]: Message All\n";

				// Construct a new message and send it to all clients
				net::message<CustomMsgTypes> msg;
				msg.header.id = CustomMsgTypes::ServerMessage;
				msg << client->get_id();
				this->message_all(msg);

			}
		}
		return true;
	}
};

int main()
{
	Server server(60000);
	server.start();

	while (1)
	{
		server.update(-1);
	}



	return 0;
}