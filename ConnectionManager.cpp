//Primary author: Jonathan Bedard
//Confirmed Working: 11/2/2014

#ifndef CONNECTOINMANAGER_CPP
#define CONNECTOINMANAGER_CPP

#include <string>
#include <iostream>
#include <fstream>

#include "osFunctions.h"
#include "ConnectionManager.h"
#include "Controller.h"
#include "AVL.h"

#define IPADDRESSFILE "IP_Address_Bank"
#define IDFILE "ID_Info_File"

using namespace std;

/* NOTE: BYTE IDENTIFIER

Server flag identifier
  |        Base/Remote ID bit
  |         | Checks if server is on
  |         | |
  V         V V
X-0-0-0-0-0-0-0
   |       |
   ---------
 Primary identifier

*/
//Interface Receiveing class
	//Receives a message, an interface
	void receiveInterface::receive_message(interior_message* msg)
	{
		//Virtua function, no action
	}

//Helper Class (Addresses)
	//Construct/Destructor
	address_module::address_module()
	{
		outbound = NULL;

		is_me = false;
		is_polling = true;
		is_receiving = false;
		first_con = false;

		module_lock.release();
		con_type = false;
		last_message = 0;

		security_gate = NULL;
		is_authenticated = false;
	}
	address_module::~address_module()
	{
		//NEED TO DELETE AVL
		if(outbound != NULL)
			delete(outbound);

		if(security_gate != NULL)
			delete(security_gate);
	}
	//Required for AVL functionality
	const bool address_module::operator==(const address_module& comp) const
	{
		if(0 == identifier.compare(&comp.identifier))
			return true;
		return false;
	}
	const bool address_module::operator>(const address_module& comp) const
	{
		if(1 == identifier.compare(&comp.identifier))
			return true;
		return false;
	}

//Helper Class (ID)
	//Construct/Destruct
	ID_module::ID_module(ConnectionManager* m)
	{
		mstr = m;
		name_ID = "NULL";
		choice_status = 0;

		LANCounter = 0;
		isConnected = false;
	}
	ID_module::~ID_module()
	{
	}
	
	interior_message* ID_module::getMessage(address_module* where_to)
	{
		uint8_t array[8];

		//Header data
		array[0] = 4;
		array[1] = 0;
		array[2] = 0;
		array[3] = 0;

		//Message
			//Coded message type (connection status)
		array[4] = 0;
			//Your priority
		array[5] = choice_status;
			//My screening type
		array[6] = mstr->getScreenType();
			//Sends if I would/wouldn't connect
		array[7] = (int) mstr->willConnect(this);

		control_stream_establish.push_data(array,8);
		return &control_stream_establish;
	}
	int ID_module::getConnectionStatus()
	{
		if(isConnected == false)
			return 0;
		if(LANCounter > 0)
			return 1;
		return 0;
	}
	bool ID_module::canDelete()
	{
		if(getConnectionStatus() == 0 && addresses_ascociated.empty())
			return true;
		return false;
	}
	//Required for AVL functionality
	const bool ID_module::operator==(const ID_module& comp) const
	{
		return name_ID==comp.name_ID;
	}
	const bool ID_module::operator>(const ID_module& comp) const
	{
		return name_ID>=comp.name_ID;
	}


//Static Event Functions------------------------------------------------------------------------------------

	//Triggers the conneciton manager's polling loop
	static void connection_magager_polling_loop(void* ptr)
	{
		((ConnectionManager*) ptr)->polling_loop();
	}
	//Asynchronous receive events
	static void connection_manager_server_receive(void* ptr)
	{
		void** ptr_array = (void**) ptr;
		ConnectionManager* conMan =  (ConnectionManager*) ptr_array[0];
		UDPServer* temp_serve = (UDPServer*) ptr_array[1];
		UDPPacket* hld = temp_serve->recieve();
		if(hld!=NULL)
			conMan->server_receive(hld);
	}
	static void connection_manager_client_receive(void* ptr)
	{
		void** ptr_array = (void**) ptr;
		ConnectionManager* conMan =  (ConnectionManager*) ptr_array[0];
		UDPClient* temp_cli = (UDPClient*) ptr_array[1];
		address_module* mdl = (address_module*) ptr_array[2];
		UDPPacket* hld = temp_cli->recieve();
		if(hld!=NULL)
			conMan->receive(hld,mdl);
	}
	static void async_INetModule_Delete(void* ptr)
	{
		sleep(2500);
		address_module* hld = (address_module*) ptr;
		delete(hld);
	}
	static void async_IDModule_Delete(void* ptr)
	{
		sleep(5000);

		ID_module* hld = (ID_module*) ptr;
		hld->module_lock.acquire();
		if(!hld->canDelete())
		{
			cerr<<"Fatal error! Module ordered for deletion is still active!"<<endl;
			exit(0);
		}
		delete(hld);
	}
//Constructor/Destructor------------------------------------------------------------------------------------

	//Constructor
	ConnectionManager::ConnectionManager(char* folder_loc, char* name, myIPAddress* home, int port, int type, int ID, int* con_scr)
	{
		//Initialize global authentication tracking
		LAN_track = 0;

		//Link to connection screening int
		connection_screening = con_scr;
		connectedModule = NULL;
		receiver = NULL;

		//Create ID byte
		if(ID<=15&&ID>=0)
			byte_ID = ID<<2;
		else
		{
			cerr<<"Invalid ID valud"<<endl;
			exit(EXIT_FAILURE);
		}
		if(type)
			byte_ID = byte_ID | 1<<1;

		//Lock name array and directory
		name_array = name;
		directory_name = string(folder_loc);

		//Create the Inet folder
		testCreateFolder(directory_name);

		home_IP = home;

		//Load/Save back to back
		load_addresses();
		load_IDs();

		//Deal with local IP
		address_module* hld = new address_module;
		hld->identifier = home->getAddress();
		hld->is_me = true;
		IP_Address_Tree.insert(hld);

		save_addresses();

		//Initiate the server
		shared_port = port;
		serve = new UDPServer(shared_port);

		//Set up clients
		IP_Address_Tree.resetTraverse();
		AVLNode<address_module>* trace = IP_Address_Tree.getFirst();
		
		//Dont need guards, no second threads yet
		while(trace!=NULL)
		{
			trace->getData()->outbound = new UDPClient(shared_port,
							trace->getData()->identifier);
			trace = trace->getNext();
		}

		//Set thread status
		active = false;

		//Load the crypto
		pbky = NULL;
		spawnThread(&generate_new_key, (void*) this);
	}
	//Destructor
	ConnectionManager::~ConnectionManager()
	{
		//Ensure the thread is killed
		setActive(false);
		delete(serve);
		if(pbky!=NULL)
			delete pbky;

		//Delete all buffered messages
		estList.acquire();
		for (list<interior_message*>::iterator it = msg_establish_list.begin(); it!=msg_establish_list.end(); ++it)
			delete((*it));
		estList.release();

		gdMsg.acquire();
		for (list<interior_message*>::iterator it = msg_good_messages.begin(); it!=msg_good_messages.end(); ++it)
			delete((*it));
		gdMsg.release();
	}

//File Functions--------------------------------------------------------------------------------------------

	//Loads the addresses
	void ConnectionManager::load_addresses()
	{
		saveLock.acquire();

		validSave = true;
		ifstream loadFile;
		loadFile.open ((directory_name+"/"+IPADDRESSFILE+".rcf").c_str());
		
		string comp;

		//Read the header
		comp = readTill(&loadFile, ';');
		if(comp != "IP_Address_Bank")
		{
			cerr<<"Corrupted IP Address bank."<<endl;
			validSave = false;
			loadFile.close();
		}

		bool quit = false;
		address_module* me_temp = new address_module;
		me_temp->identifier = home_IP->getAddress();
		
		//Read all IP addresses
		do {
			readTill(&loadFile, '>');
			if(validSave)
				comp = readTill(&loadFile, ':');
			if(comp == "end")
				quit = true;
			else if(comp == "Address")
			{
				//Found address tag, try to find an IP
				comp = readTill(&loadFile, ':');
				if(validSave)
				{
					address_module* hld = new address_module;
					hld->identifier = IPAddress(comp);
					comp = readTill(&loadFile, ';');
					if(validSave&&comp=="blocked")
						hld->is_polling = false;
					IPAddress temp = home_IP->getAddress();
					if((*me_temp) == (*hld))
						hld->is_me = true;

					avlLock.acquire();
					if(!IP_Address_Tree.insert(hld))
						delete(hld);
					avlLock.release();
				}
			}
			else
				validSave = false;

			//Catch an error
			if(!validSave)
			{
				cerr<<"Corrupted IP Address bank."<<endl;
				loadFile.close();
			}

		}while(validSave&&!quit);
		delete (me_temp);

		if(validSave)
			loadFile.close();

		saveLock.release();
	}
	//Saves the addresses
	void ConnectionManager::save_addresses()
	{
		saveLock.acquire();

		ofstream saveFile;
		saveFile.open ((directory_name+"/"+IPADDRESSFILE+".rcf").c_str());

		//Header
		saveFile<<"IP_Address_Bank;"<<endl;

		avlLock.acquire();

		IP_Address_Tree.resetTraverse();
		AVLNode<address_module>* trace = IP_Address_Tree.getFirst();
		
		while(trace!=NULL)
		{
			saveFile<<">Address:";
			saveFile<<trace->getData()->identifier.printAddress();
			saveFile<<":";
			if(trace->getData()->is_polling)
				saveFile<<"active";
			else
				saveFile<<"blocked";
			saveFile<<";"<<endl;
			trace = trace->getNext();
		}

		avlLock.release();
		//Closer
		saveFile<<">end:;";
		saveFile.close();
		saveLock.release();
	}
	//Loads the IDs
	void ConnectionManager::load_IDs()
	{
		saveLock.acquire();

		validSave = true;
		ifstream loadFile;
		loadFile.open ((directory_name+"/"+IDFILE+".rcf").c_str());

		string comp;
		ID_module* mdl = NULL;

		//Read the header
		comp = readTill(&loadFile, ';');
		if(comp != "IDs_and_Keys")
		{
			cerr<<"ID File error. Header reads: "<<comp<<endl;
			validSave = false;
			loadFile.close();
		}

		//Read line headers
		if(validSave)
		{
			readTill(&loadFile,'>');
			comp = readTill(&loadFile,':');
		}
		while(validSave && comp != "end")
		{
			//Should be a name ID
			if(validSave&&comp!="String_ID")
			{
				cerr<<"ID File improperly formated: Missing String_ID"<<endl;
				validSave = false;
				loadFile.close();
			}

			//Read the ID
			if(validSave)
				comp = readTill(&loadFile,';');
			if(validSave)
			{
				mdl = new ID_module(this);
				mdl->name_ID = comp;
			}

			//Next, attempt to read the choice status
			if(validSave)
			{
				readTill(&loadFile,'>');
				comp = readTill(&loadFile,':');
			}
			if(comp != "Choice_Status")
			{
				cerr<<"ID File improperly formated: Missing Choice+Status"<<endl;
				validSave = false;
				loadFile.close();
			}

			//Read in choice status
			if(validSave && mdl !=NULL)
				comp = readTill(&loadFile,';');
			if(validSave && comp == LIST_CHOICE_STRING)
				mdl->choice_status = LIST_CHOICE;
			else if(validSave && mdl !=NULL && comp == PREFERED_CHOICE_STRING)
				mdl->choice_status = PREFERED_CHOICE;
			else if(validSave && mdl !=NULL && comp == BLOCKED_CHOICE_STRING)
				mdl->choice_status = BLOCKED_CHOICE;
			else if(validSave && mdl !=NULL)
				mdl->choice_status = DEFAULT_CHOICE;

			//Next, attempt to load the key
			if(validSave)
			{
				readTill(&loadFile,'>');
				comp = readTill(&loadFile,':');
			}
			if(validSave&&comp!="Public_Key")
			{
				cerr<<"ID File improperly formated: Missing Public_Key"<<endl;
				validSave = false;
				loadFile.close();
			}

			//Read in the key itself
			int num = LARGE_NUMBER_SIZE/2-1;
			uint32_t num_array[LARGE_NUMBER_SIZE/2];
			while(validSave && num > 0)
			{
				comp = readTill(&loadFile,'-');
				int str_trc = 0;
				
				//Ensure that it is a number
				while(validSave && comp.length()>str_trc)
				{
					if(!check_numeric(comp.at(str_trc)))
					{
						cerr<<"ID File impropertly formated: Invalid key"<<endl;
						validSave = false;
					}
					str_trc++;
				}
				//Load the number
				if(validSave)
					num_array[num] = (uint32_t) convert_64(comp);
				num--;
			}
			if(validSave)
				comp = readTill(&loadFile,';');
			int str_trc = 0;
				
			//Ensure that it is a number
			while(validSave && comp.length()>str_trc)
			{
				if(!check_numeric(comp.at(str_trc)))
				{
					cerr<<"ID File impropertly formated: Invalid key"<<endl;
					validSave = false;
				}
				str_trc++;
			}
			//Load the number
			if(validSave)
				num_array[0] = (uint32_t) convert_64(comp);

			//Push number onto 
			if(validSave)
				mdl->myKey.push_array(num_array, LARGE_NUMBER_SIZE/2);

			//Input the string
			if(validSave)
				ID_Tree.insert(mdl);

			//Read next section header
			if(validSave)
			{
				readTill(&loadFile,'>');
				comp = readTill(&loadFile,':');
			}
		}

		if(validSave)
			loadFile.close();
		saveLock.release();
	}
	//Saves the ID's
	void ConnectionManager::save_IDs()
	{
		saveLock.acquire();

		ofstream saveFile;
		saveFile.open ((directory_name+"/"+IDFILE+".rcf").c_str());

		//Header
		saveFile<<"IDs_and_Keys;"<<endl;

		IDLock.acquire();

		ID_Tree.resetTraverse();
		AVLNode<ID_module>* trace = ID_Tree.getFirst();

		while(trace!=NULL)
		{
			trace->getData()->module_lock.acquire();

			saveFile<<">String_ID:"<<trace->getData()->name_ID<<";"<<endl;
			saveFile<<"\t>Choice_Status:";
			if(trace->getData()->choice_status == DEFAULT_CHOICE)
				saveFile<<DEFAULT_CHOICE_STRING;
			else if(trace->getData()->choice_status == LIST_CHOICE)
				saveFile<<LIST_CHOICE_STRING;
			else if(trace->getData()->choice_status == PREFERED_CHOICE)
				saveFile<<PREFERED_CHOICE_STRING;
			else
				saveFile<<BLOCKED_CHOICE_STRING;
			saveFile<<";"<<endl;
			saveFile<<"\t>Public_Key:";

			//Print out the key
			int cnt = LARGE_NUMBER_SIZE/2-1;
			while(cnt>=0)
			{
				saveFile<<trace->getData()->myKey.getArrayNumber(cnt);
				cnt--;
				if(cnt>=0)
					saveFile<<"-";
			}
			saveFile<<";"<<endl;

			trace->getData()->module_lock.release();
			trace = trace->getNext();
		}


		IDLock.release();

		//Closer
		saveFile<<">end:;";
		saveFile.close();

		saveLock.release();
	}
	//Reads till the specified character
	string ConnectionManager::readTill(ifstream* file, char x)
	{
		if(!validSave)
			return "";
		string ret = "";
		char t = x+1;

		while(file->good()&&t!=x)
		{
			(*file)>>t;
			if(x!=t)
				ret = ret+t;
		}

		//Trigger load error
		if(!file->good())
		{
			cerr<<"File out of bounds"<<endl;
			validSave = false;
			file->close();
			ret = "";
		}
		return ret;
	}

//Action Functions------------------------------------------------------------------------------------------

	//Connection management polling loop
	void ConnectionManager::polling_loop()
	{
		//Create the default message
		makeIDMessage();

		//Start the server
		serve->start();

		//Start the clients
		IP_Address_Tree.resetTraverse();
		AVLNode<address_module>* trace = IP_Address_Tree.getFirst();
		
		while(trace!=NULL)
		{
			trace->getData()->outbound->connect();
			trace->getData()->ptr_array[0] = (void*) this;
			trace->getData()->ptr_array[1] = (void*) trace->getData()->outbound;
			trace->getData()->ptr_array[2] = (void*) trace->getData();
			trace->getData()->outbound->setRecieveEvent(&connection_manager_client_receive, (void*) trace->getData()->ptr_array);
			trace = trace->getNext();
		}
		//Aquired in the previous call
		avlLock.release();

		uint64_t timestamp = 0;

		int cnt = 0;
		//Main loop
		while(active)
		{
			activeLock.acquire();

			bool save_ID_file = false;

			//Attempt to start the server
			if((cnt+1)%50==0)
				serve->start();

			//Sets a new timestamp
			timestamp = get_timestamp();
			uint8_t* timestamp_ptr = (uint8_t*) &timestamp;
			int cnt_stmp = 0;
			while(cnt_stmp<8)
			{
				ID_Array[4+ID_SIZE+cnt_stmp] = timestamp_ptr[cnt_stmp];
				cnt_stmp++;
			}

			//Set byte data
			if(serve->getConnected())
				byte_ID = byte_ID | 1;
			else
				byte_ID = byte_ID & (~1);

			//Process Messages------------------------------------------------------------------------

			estList.acquire();
			while(!msg_establish_list.empty())
			{
				interior_message* temp_msg = msg_establish_list.front();
				address_module* add_mdl = add_establish_list.front();

				add_mdl->module_lock.acquire();
				uint8_t msg_type = temp_msg->get_char_data()[0];

				//Message type 0, check for good name
				if(msg_type == 0)
				{
					string nom_comp(&(temp_msg->get_char_data()[4]));
					//Create a security gateway, if this is a new initialization message
					if(add_mdl->is_polling&&add_mdl->security_gate==NULL && nom_comp != "NULL")
					{
						add_mdl->security_gate = new security_gateway();
						add_mdl->security_gate->push_data(pbky,0,name_array);
						add_mdl->module_lock.release();

						//Add to ID list
						IDLock.acquire();

						ID_module* hld = new ID_module(this);
						hld->name_ID = nom_comp;
						AVLNode<ID_module>* temp;

						//Find or build a new ID module
						temp = ID_Tree.find(hld);
						if(temp==NULL)
						{
							hld->addresses_ascociated.push_back(add_mdl);
							save_ID_file = true;
							ID_Tree.insert(hld);
						}
						else
						{
							temp->getData()->module_lock.acquire();
							temp->getData()->addresses_ascociated.push_back(add_mdl);
							temp->getData()->module_lock.release();
						}

						IDLock.release();
						add_mdl->module_lock.acquire();
						
						//Tie address module to an ID module
						if(temp!=NULL)
						{
							add_mdl->ID_mod = temp->getData();
							if(temp->getData()->myKey != zero_test)
								add_mdl->security_gate->push_old_key(temp->getData()->myKey);
							delete(hld);
						}
						else
							add_mdl->ID_mod = hld;
					}
				}

				//Then, attempt to process
				if(add_mdl->is_polling && add_mdl->security_gate!=NULL)
					add_mdl->security_gate->process_message(temp_msg);

				//Confirm LAN connection
				if(add_mdl->security_gate!=NULL && add_mdl->security_gate->connected())
				{
					if(!add_mdl->is_authenticated && add_mdl->ID_mod!=NULL)
					{
						add_mdl->is_authenticated = true;

						add_mdl->ID_mod->module_lock.acquire();
						add_mdl->ID_mod->LANCounter++;
						add_mdl->ID_mod->module_lock.release();
						authen_lock.acquire();
						LAN_track++;
						authen_lock.release();
					}

				}
				else
				{
					if(add_mdl->is_authenticated && add_mdl->ID_mod!=NULL)
					{
						add_mdl->is_authenticated = false;

						add_mdl->ID_mod->module_lock.acquire();
						add_mdl->ID_mod->LANCounter--;
						add_mdl->ID_mod->module_lock.release();

						authen_lock.acquire();
						LAN_track--;
						authen_lock.release();
					}
				}

				//Test the connection
				if(add_mdl->security_gate!=NULL && add_mdl->security_gate->connected())
				{
					//Test if this is the inial connection
					if(!add_mdl->first_con)
					{
						add_mdl->first_con = true;

						//Change the value of the ID key, if neccessary
						ID_module* ID_mod = add_mdl->ID_mod;

						if(ID_mod != NULL)
						{
							ID_mod->module_lock.acquire();

							//Set brother key and push it to all others in the list
							if(add_mdl->security_gate->getBrotherKey() != ID_mod->myKey)
							{
								save_ID_file = true;
								ID_mod->myKey = add_mdl->security_gate->getBrotherKey();
								add_mdl->module_lock.release();

								for(list<address_module*>::iterator it = ID_mod->addresses_ascociated.begin(); it != ID_mod->addresses_ascociated.end(); it++)
								{
									if((*it) != add_mdl)
									{
										(*it)->module_lock.acquire();
										(*it)->security_gate->push_old_key(ID_mod->myKey);
										(*it)->module_lock.release();
									}
								}
								add_mdl->module_lock.acquire();
							}

							ID_mod->module_lock.release();
						}
					}
				}
				delete(temp_msg);

				add_mdl->module_lock.release();
				msg_establish_list.pop_front();
				add_establish_list.pop_front();
			}
			estList.release();


			//Send Messages---------------------------------------------------------------------------


			list<address_module*> message_output_list;

			avlLock.acquire();
			IP_Address_Tree.resetTraverse();
			trace = IP_Address_Tree.getFirst();

			interior_message* msg_hld = NULL;
			IPAddress hld = home_IP->getAddress();

			while(trace!=NULL)
			{
				trace->getData()->module_lock.acquire();

				//Check if we are the "home" IP
				if(trace->getData()->identifier.compare(&hld)==0)
					trace->getData()->is_me = true;
				else
					trace->getData()->is_me = false;

				//Handle connection type
				if(trace->getData()->last_message==0 || trace->getData()->last_message < timestamp - 15 || !trace->getData()->is_polling)
				{
					delete(trace->getData()->security_gate);
					trace->getData()->security_gate = NULL;
					trace->getData()->is_receiving = false;
					trace->getData()->con_type = false;

					//Disconnect LAN
					if(trace->getData()->ID_mod!=NULL)
					{
						trace->getData()->ID_mod->module_lock.acquire();
						if(trace->getData()->is_authenticated)
						{
							trace->getData()->ID_mod->LANCounter--;
							authen_lock.acquire();
							LAN_track--;
							authen_lock.release();
						}
						trace->getData()->ID_mod->addresses_ascociated.remove(trace->getData());
						trace->getData()->ID_mod->module_lock.release();
					}
					trace->getData()->is_authenticated = false;

					//Triggered disconnection
					if(trace->getData()->first_con)
					{
						trace->getData()->first_con = false;
						trace->getData()->ID_mod->module_lock.acquire();
						trace->getData()->ID_mod->addresses_ascociated.remove(trace->getData());
						trace->getData()->ID_mod->module_lock.release();
						trace->getData()->ID_mod = NULL;
					}
				}
				else
					trace->getData()->is_receiving = true;

				//Create UDP packet
				UDPPacket* temp = NULL;
				if(trace->getData()->is_polling&&trace->getData()->security_gate == NULL)
					temp = new UDPPacket(ID_Array, 4+ID_SIZE+8+LARGE_NUMBER_SIZE*2, byte_ID, &trace->getData()->identifier, shared_port);
				else if(trace->getData()->is_polling)
					message_output_list.push_back(trace->getData());

				//Send UDP packet
				if(temp!=NULL)
				{
					if(trace->getData()->con_type)
						serve->send(temp);
					else
						trace->getData()->outbound->send(temp);
					delete(temp);
				}

				trace->getData()->module_lock.release();
				trace = trace->getNext();
			}
			avlLock.release();

			//Build and send crypto messages
			for (list<address_module*>::iterator it = message_output_list.begin(); it!=message_output_list.end(); ++it)
			{
				(*it)->module_lock.acquire();

				msg_hld = (*it)->security_gate->get_message();

				//Create a custom sustained connection message
				if((*it)->security_gate->connected() && msg_hld->get_int_data()[0] == 3)
				{
					(*it)->ID_mod->module_lock.acquire();
					msg_hld = (*it)->ID_mod->getMessage((*it));
					(*it)->ID_mod->module_lock.release();
					msg_hld = (*it)->security_gate->encrypt_message(msg_hld);
				}

				UDPPacket* temp = new UDPPacket(msg_hld->get_int_data(), msg_hld->get_length(), byte_ID, &(*it)->identifier, shared_port);

				if((*it)->con_type)
					serve->send(temp);
				else
					(*it)->outbound->send(temp);
				delete(temp);

				(*it)->module_lock.release();
			}

			//Check the validity of the current connection, if its not valid, disconnect
			if(connectedModule != NULL&&
				(connectedModule->getConnectionStatus()<=0||!willConnect(connectedModule)))
			{
				connectedModule->isConnected = false;
				connectedModule = NULL;
			}

			if(save_ID_file)
				save_IDs();

			activeLock.release();

			//Brief sleep, increment counter
			sleep(250);
			cnt++;
			if(cnt>=100)
				cnt = 0;
		}
	}
	//The asynchronous receive function
	void ConnectionManager::server_receive(UDPPacket* pck)
	{
		//Search for IP
		avlLock.acquire();

		address_module temp;
		address_module* addr_mod;
		AVLNode<address_module>* avl_node = NULL;
		temp.identifier = pck->getAddress();

		avl_node = IP_Address_Tree.find(&temp);
		if(avl_node == NULL)
		{
			avlLock.release();
			addr_mod = addIPAddress(pck->getAddress());
		}
		else
		{
			addr_mod = avl_node->getData();
			avlLock.release();
		}

		receive(pck, addr_mod);
	}
	//Receives a message, with its address module
	void ConnectionManager::receive(UDPPacket* pck, address_module* mdl)
	{
		//Check the status
		uint8_t foriegn_ID = pck->getType();

		//Check general type (rejects all servers at the moment)
		if(foriegn_ID>>2 != byte_ID>>2)
		{
			delete(pck);
			return;
		}

		//Check base/remote comparison
		if((foriegn_ID&2) == (byte_ID&2))
		{
			delete(pck);
			return;
		}

		mdl->module_lock.acquire();
		mdl->last_message = get_timestamp();

		//Set connection type
		if((foriegn_ID&1)==(byte_ID&1))
		{
			if((byte_ID&2))
				mdl->con_type = true;
			else
				mdl->con_type = false;
		}
		else if(!(byte_ID&1))
			mdl->con_type = false;
		else
			mdl->con_type = true;

		//Build an interior message
		interior_message* msg = new interior_message(pck->getData(), pck->getLength());

		mdl->module_lock.release();

		if(msg->get_int_data()[0] == 4)
		{
			async_receive(msg, mdl);
			delete(pck);
			return;
		}
		estList.acquire();
		msg_establish_list.push_back(msg);
		add_establish_list.push_back(mdl);
		estList.release();
		delete(pck);
	}
	//Adds an IP address to the bank
	address_module* ConnectionManager::addIPAddress(IPAddress ad)
	{
		avlLock.acquire();

		address_module* addr_mod = new address_module();
		addr_mod->identifier = ad;

		//Try and find it first
		if(IP_Address_Tree.find(addr_mod)!=NULL)
		{
			avlLock.release();
			delete(addr_mod);
			return NULL;
		}

		addr_mod->outbound = new UDPClient(shared_port,ad);
		if(active)
		{
			addr_mod->outbound->connect();
			addr_mod->ptr_array[0] = (void*) this;
			addr_mod->ptr_array[1] = (void*) addr_mod->outbound;
			addr_mod->ptr_array[2] = (void*) addr_mod;
			addr_mod->outbound->setRecieveEvent(&connection_manager_client_receive, (void*) addr_mod->ptr_array);
		}
		IP_Address_Tree.insert(addr_mod);

		avlLock.release();

		save_addresses();

		return addr_mod;
	}
	//Deletes an IP address module
	void ConnectionManager::deleteModule(address_module* mdl)
	{
		//Try and find it first
		if(IP_Address_Tree.findDelete(mdl))
		{
			delete (mdl->outbound);
			mdl->outbound = NULL;
			spawnThread(&async_INetModule_Delete,(void*) mdl);
		}
	}
	//Deletes an ID module
	void ConnectionManager::deleteIDModule(ID_module* mdl)
	{
		//Check if the module can be deleted
		mdl->module_lock.acquire();
		if(!mdl->canDelete())
			return;
		mdl->module_lock.release();

		IDLock.acquire();
		if(mdl->module_lock.isTaken())
			return;
		ID_Tree.findDelete(mdl);
		IDLock.release();

		spawnThread(&async_IDModule_Delete,(void*) mdl);
	}

//Set Functions---------------------------------------------------------------------------------------------

	//Spawns/ends the threads
	void ConnectionManager::setActive(bool x)
	{
		activeLock.acquire();

		//Turn on
		if(x==true&&active == false&&pbky!=NULL)
		{
			avlLock.acquire();
			active = true;

			//Set asynchronous actions
			serve_ptr_array[0] = (void*) this;
			serve_ptr_array[1] = (void*) serve;
			serve->setRecieveEvent(&connection_manager_server_receive, (void*) serve_ptr_array);

			//Spawn the thread
			spawnThread(&connection_magager_polling_loop, (void*) this);
		}
		//Turn off
		if(x==false&&active==true)
		{
			active = false;

			avlLock.acquire();

			//End the server
			serve->end();

			//End the clients

			IP_Address_Tree.resetTraverse();
			AVLNode<address_module>* trace = IP_Address_Tree.getFirst();
		
			while(trace!=NULL)
			{
				trace->getData()->outbound->disconnect();
				delete(trace->getData()->security_gate);
				trace->getData()->security_gate = NULL;
				trace->getData()->is_receiving = false;
				trace->getData()->con_type = false;

				//Disconnect LAN
				if(trace->getData()->ID_mod!=NULL)
				{
					trace->getData()->ID_mod->module_lock.acquire();
					if(trace->getData()->is_authenticated)
					{
						trace->getData()->ID_mod->LANCounter--;
						authen_lock.acquire();
						LAN_track--;
						authen_lock.release();
					}
					trace->getData()->ID_mod->addresses_ascociated.remove(trace->getData());
					trace->getData()->ID_mod->module_lock.release();
				}
				trace->getData()->is_authenticated = false;

				//Triggered disconnection
				if(trace->getData()->first_con)
					trace->getData()->first_con = false;
				trace = trace->getNext();
			}
			avlLock.release();
		}

		activeLock.release();
	}
	//Sets the public key structure, WARNING: NO MEMORY PROTECTION
	void ConnectionManager::setPublicKey(public_key_base* k)
	{
		pbky = k;
	}
	//Sets the receiver interface
	void ConnectionManager::setReceiver(receiveInterface* rc)
	{
		receiverLock.acquire();
		receiver = rc;
		receiverLock.release();
	}

//Get Functions---------------------------------------------------------------------------------------------

	//Returns if the server is functioning
	bool ConnectionManager::getServerOn()
	{
		if(!active)
			return false;
		return serve->getConnected();
	}
	//Return if the connection manager is active
	bool ConnectionManager::getActive()
	{
		return active;
	}
	//Returns a pointer to the AVL tree, holds the tree till the data is returned
	AVLTree<address_module>* ConnectionManager::getClientData()
	{
		avlLock.acquire();
		return &IP_Address_Tree;
	}
	//Frees the AVL tree
	void ConnectionManager::returnClientData()
	{
		avlLock.release();
	}
	//Returns a pointer to the ID AVL tree, holds the tree till the data is returned
	AVLTree<ID_module>* ConnectionManager::getIDData()
	{
		IDLock.acquire();
		return &ID_Tree;
	}
	//Frees the ID AVL tree
	void ConnectionManager::returnIDData()
	{
		IDLock.release();
	}
	//Returns the public key
	public_key_base* ConnectionManager::getPublicKey()
	{
		return pbky;
	}
	//Return the directory name
	string ConnectionManager::getDirName()
	{
		return directory_name;
	}
	//Checks if there are authenicated LAN connection
	bool ConnectionManager::isLANAuthentic()
	{
		if(LAN_track)
			return true;
		else
			return false;
	}
	//Return the screen type
	int ConnectionManager::getScreenType()
	{
		return (*connection_screening);
	}

//Connection Interface--------------------------------------------------------------------------------------

	//Tests if an ID would be an acceptable connection
	bool ConnectionManager::willConnect(ID_module* mdl)
	{
		int scr_type = (*connection_screening);

		//If I am blocked, return false
		if(mdl==NULL || mdl->choice_status == BLOCKED_CHOICE)
			return false;

		//If I am prefered, an we are screening based of that, return true
		if(mdl->choice_status == PREFERED_CHOICE &&
			(scr_type == ONE_SELECTION || scr_type == PREFFERED_LIST || scr_type == PREFFERED_AUTO))
			return true;

		//If I only accept the prefered choice, and you are not that, return false
		if(scr_type == ONE_SELECTION)
			return false;

		//See if there is a current connection
		bool isConnected;
		connectionLock.acquire();

		//Figure out if we're connected
		if(connectedModule == NULL)
			isConnected = false;
		else
			isConnected = true;

		//If I am already connected, check if my connection fulfills the requirments
		if(connectedModule == mdl)
		{
			if(mdl->choice_status!=LIST_CHOICE&&
				(scr_type==PREFFERED_LIST || scr_type==LIST_SELECTION))
			{
				connectionLock.release();
				return false;
			}
			connectionLock.release();
			return true;
		}

		//If the connected module is preffered and we are screening based off of that, return false
		if(connectedModule!=NULL&&connectedModule->choice_status == PREFERED_CHOICE && 
			(scr_type==PREFFERED_AUTO || scr_type==PREFFERED_LIST))
		{
			connectionLock.release();
			return false;
		}

		//If the connected module is on the list and we are screening based off of that, return false
		if(connectedModule!=NULL && (connectedModule->choice_status == LIST_CHOICE || connectedModule->choice_status == PREFERED_CHOICE) && 
			(scr_type==PREFFERED_LIST || scr_type==LIST_CHOICE))
		{
			connectionLock.release();
			return false;
		}

		connectionLock.release();

		//If I am on the list, and we are screening based on lists, connect
		if((mdl->choice_status == LIST_CHOICE || mdl->choice_status == PREFERED_CHOICE) && 
			(scr_type==PREFFERED_LIST || scr_type==LIST_SELECTION))
			return true;

		//I am not on the list, and we are screening based on the list
		if(scr_type==PREFFERED_LIST || scr_type==LIST_SELECTION)
			return false;

		//If we are connected, return false
		if(isConnected)
			return false;
		return true;
	}
	//Reads a connection message
	bool ConnectionManager::testConnectionMessage(uint8_t* start)
	{
		//Analyze my connection type
		if(start[0] == BLOCKED_CHOICE)
			return false;

		//If I am not prefered, and that is the screening protocol, return false
		if(start[0] != PREFERED_CHOICE && start[1] == ONE_SELECTION)
			return false;

		//If I am not list or prefered, an the protocol demands it, return
		if((start[0] != PREFERED_CHOICE && start[0] != PREFERED_CHOICE)&&
			(start[1] == LIST_SELECTION|| start[1] == PREFFERED_LIST))
			return false;

		//Check what the other controller said
		if(start[2])
			return true;
		return false;
	}
	//Lock and return the connectedModule
	ID_module* ConnectionManager::getModule()
	{
		connectionLock.acquire();
		return connectedModule;
	}
	//Unlock the connection module
	void ConnectionManager::returnModule()
	{
		connectionLock.release();
	}
	//Sends a message
	bool ConnectionManager::sendMessage(interior_message* out)
	{
		connectionLock.acquire();

		//There is no connected module
		if(connectedModule==NULL || connectedModule->getConnectionStatus() == 0)
		{
			connectionLock.release();
			return false;
		}

		//Try to send over Wifi
		if(connectedModule->getConnectionStatus() == 1)
		{
			address_module* add_mdl = (*connectedModule->addresses_ascociated.begin());
			connectionLock.release();
			
			add_mdl->module_lock.acquire();

			//Check crypto
			if(add_mdl->security_gate == NULL || !add_mdl->is_authenticated)
			{
				add_mdl->module_lock.release();
				return false;
			}

			//Run crypto
			out = add_mdl->security_gate->encrypt_message(out);

			//Build and sent message
			UDPPacket* temp = new UDPPacket(out->get_int_data(), out->get_length(), byte_ID, &add_mdl->identifier, shared_port);
			if(add_mdl->con_type)
				serve->send(temp);
			else
				add_mdl->outbound->send(temp);
			delete(temp);

			add_mdl->module_lock.release();

			return true;
		}

		connectionLock.release();

		return false;
	}
	//Returns if there is a valid connection
	bool ConnectionManager::isConnected()
	{
		connectionLock.acquire();

		//There is no connected module
		if(connectedModule==NULL || connectedModule->getConnectionStatus() == 0)
		{
			connectionLock.release();
			return false;
		}
		connectionLock.release();
		return true;
	}

//Private FUnctions-----------------------------------------------------------------------------------------

	//Writes the ID message array
	void ConnectionManager::makeIDMessage()
	{
		
		//Message header
		ID_Array[0] = 0;
		ID_Array[1] = 0;
		ID_Array[2] = 0;
		ID_Array[3] = 0;
  
		//Copy in the system ID
		int cnt = 4;
		while(cnt<ID_SIZE+4)
		{
			ID_Array[cnt] = name_array[cnt-4];
			cnt++;
		}
		while(cnt<ID_SIZE+12)
		{
			ID_Array[cnt] = 0;
			cnt++;
		}
  
		//Skip pointer ahead
		cnt = 0;
		uint8_t* ptr = ID_Array;
		while(cnt<4+ID_SIZE+8)
		{
			ptr++;
			cnt++;
		}
  
		//Plublish the public key_source
		large_integer pub_key = pbky->get_n();
		cnt = 0;
		uint32_t* trc = (uint32_t*) ptr;
		while(cnt<LARGE_NUMBER_SIZE/2)
		{
			trc[cnt] = pub_key.getArrayNumber(cnt);
			cnt++;
		}
	}
	//Receives communication messages
	void ConnectionManager::async_receive(interior_message* msg, address_module* mdl)
	{
		mdl->module_lock.acquire();
		//Bad security gate, drop the message
		if(mdl->security_gate ==NULL)
		{
			delete (msg);
			mdl->module_lock.release();
			return;
		}

		//Unauthorized gate, drop the message
		if(mdl->ID_mod == NULL || !mdl->security_gate->connected())
		{
			mdl->security_gate->process_message(msg);
			delete(msg);
			mdl->module_lock.release();
			return;
		}
		mdl->security_gate->process_message(msg);
		mdl->module_lock.release();

		//Connection processing
		if(msg->get_int_data()[4] == 0)
		{
			//Standard processesing
			if(willConnect(mdl->ID_mod) && testConnectionMessage(&msg->get_int_data()[5]))
			{
				connectionLock.acquire();

				if(connectedModule != NULL && connectedModule != mdl->ID_mod)
					connectedModule->isConnected = false;
				connectedModule = mdl->ID_mod;
				connectedModule->isConnected = true;

				connectionLock.release();
			}
			//The brother has been disconnected
			else if(!testConnectionMessage(&msg->get_int_data()[5]))
			{
				connectionLock.acquire();
				if(mdl->ID_mod==connectedModule)
				{
					connectedModule->isConnected = false;
					connectedModule = NULL;
				}
				connectionLock.release();
			}

			delete(msg);
			return;
		}
		
		//Send out a standard message to the receivers
		receiverLock.acquire();
		if(receiver != NULL)
			receiver->receive_message(msg);
		receiverLock.release();

		delete(msg);
	}

#endif