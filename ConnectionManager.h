//Primary author: Jonathan Bedard
//Confirmed Working: 10/29/2014

#ifndef CONNECTOINMANAGER_H
#define CONNECTOINMANAGER_H

#include <string>
#include <stdint.h>
#include <list>

#include "AVL.h"

#include "osFunctions.h"
#include "CryptoGateway.h"

class receiveInterface;
class address_module;
class ID_module;
class ConnectionManager;

class receiveInterface
{
public:
	virtual void receive_message(interior_message* msg);
};
class address_module
{
public:
	spinLock module_lock;

	IPAddress identifier;
	UDPClient* outbound;
	ID_module* ID_mod;

	bool is_me;
	bool is_polling;
	bool is_receiving;
	bool first_con;

	bool con_type;
	uint64_t last_message;

	void* ptr_array[3];
	security_gateway* security_gate;
	bool is_authenticated;

	//Construct/Destructoras
	address_module();
	virtual ~address_module();

	//Nessary for AVL tree
	const bool operator==(const address_module& comp) const;
	const bool operator>(const address_module& comp) const;
};
class ID_module
{
private:
	interior_message control_stream_establish;
	ConnectionManager* mstr;

public:
	string name_ID;
	large_integer myKey;
	int choice_status;

	spinLock module_lock;

	list<address_module*> addresses_ascociated;
	int LANCounter;
	bool isConnected;

	//Construct/Destructoras
	ID_module(ConnectionManager* m);
	virtual ~ID_module();

	//Stream management functions
	interior_message* getMessage(address_module* where_to);
	int getConnectionStatus();
	bool canDelete();

	//Nessary for AVL tree
	const bool operator==(const ID_module& comp) const;
	const bool operator>(const ID_module& comp) const;
};
class ConnectionManager
{
protected:
	string directory_name;
	bool validSave;
	spinLock saveLock;

	uint8_t byte_ID;
	uint8_t ID_Array[4+ID_SIZE+8+LARGE_NUMBER_SIZE*2];
	char* name_array;

	myIPAddress* home_IP;

	//IP addresses for UDP
	spinLock avlLock;
	AVLTree<address_module> IP_Address_Tree;

	//General ID tree
	spinLock IDLock;
	AVLTree<ID_module> ID_Tree;
	large_integer zero_test;

	int shared_port;
	UDPServer* serve;
	void* serve_ptr_array[2];

	bool active;
	spinLock activeLock;

	public_key_base* pbky;

	//Message buffer lists
	spinLock estList;
	list<interior_message*> msg_establish_list;
	list<address_module*> add_establish_list;
	spinLock gdMsg;
	list<interior_message*> msg_good_messages;
	list<address_module*> add_good_messages;

	//Authentication tracking
	spinLock authen_lock;
	int LAN_track;
	int* connection_screening;

	//Connected module
	spinLock connectionLock;
	ID_module* connectedModule;
	spinLock receiverLock;
	receiveInterface* receiver;

	//Private subroutines
	void makeIDMessage();
	void async_receive(interior_message* msg, address_module* mdl);

public:
	//Constructor/Destructors
	ConnectionManager(char* folder_loc, char* name, myIPAddress* home, int port, int type, int ID, int* con_scr);
	virtual ~ConnectionManager();

	//File Functions
	void load_addresses();
	void save_addresses();
	void load_IDs();
	void save_IDs();
	string readTill(ifstream* file, char x);

	//Action Functions
	void polling_loop();
	void server_receive(UDPPacket* pck);
	void receive(UDPPacket* pck, address_module* mdl);
	address_module* addIPAddress(IPAddress ad);
	void deleteModule(address_module* mdl);
	void deleteIDModule(ID_module* mdl);

	//Set Functions
	void setActive(bool x);
	void setPublicKey(public_key_base* k);
	void setReceiver(receiveInterface* rc);

	//Get Functions
	bool getServerOn();
	bool getActive();
	AVLTree<address_module>* getClientData();
	void returnClientData();
	AVLTree<ID_module>* getIDData();
	void returnIDData();
	public_key_base* getPublicKey();
	string getDirName();
	bool isLANAuthentic();
	int getScreenType();
	
	//Connection Interface
	bool willConnect(ID_module* mdl);
	bool testConnectionMessage(uint8_t* start);
	ID_module* getModule();
	void returnModule();
	bool sendMessage(interior_message* out);
	bool isConnected();
};

//Global function that resets the key structure.  Beare that this function is computationally expensive
static void generate_new_key(void* con_manage)
{
	srand(time(NULL));
	static spinLock key_lock;
	ConnectionManager* real_type = (ConnectionManager*) con_manage;
	public_key_base* hld;
	
	key_lock.acquire();

	//The base must be initialized
	if(real_type->getPublicKey() == NULL)
	{
		hld = new public_key_base(real_type->getDirName());
		real_type->setPublicKey(hld);
		real_type->setActive(true);
	}
	//Create new keys
	else
	{
		hld = real_type->getPublicKey();
		real_type->setActive(false);
		real_type->setPublicKey(NULL);

		hld->generate_new_keys();

		real_type->setPublicKey(hld);
		real_type->setActive(true);
	}

	key_lock.release();
}

#endif