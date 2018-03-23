#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "com.hpp"
#include <mutex>
#include <thread>
#include <boost/algorithm/string.hpp>


class node{
public:
	std::string port_;
	std::string ip_addr_;
	
	node(std::string ip_addr, std::string port)
	:port_(port),ip_addr_(ip_addr){

	}

	friend bool operator==(const node& lhs, const node& rhs){
		if(lhs.ip_addr_ == rhs.ip_addr_ && lhs.port_ == rhs.port_ ){
			return true;
		}
		return false;
	}

	friend bool operator!=(const node& lhs, const node& rhs){
		return !(lhs == rhs);
	}

};


class node_info{
public:
	std::string leader_;
	std::vector<node> node_list_;
	std::string serialize(){
		std::string ret = "";
		for(node n : node_list_){
			ret += n.ip_addr_ + "," + n.port_ + ";";
		}
		return ret;
	}

	/*
	 * TODO improve performance , change node_list_ to map
	 */
	void deserialize(std::string str){
		std::vector<std::string> vs1;
    	boost::split(vs1, str , boost::is_any_of(";"));
    	for(std::string s : vs1 ){
    		std::vector<std::string> vs2;
    		boost::split(vs2, s , boost::is_any_of(","));
 			bool found = false;
 			node n(vs2[0],vs2[1]);
 			for(node itr : node_list_){
 				if(n == itr)
 				{
 					found = true;
 					break;
 				}
 			}
 			if( !found){

 				node_list_.push_back(n);
 			}
    	}
	}
}info;
std::mutex info_m;

void update_nodes(){
	for(int i =0; i < info.node_list_.size()-1;++i){
		std::string message = "UPDATE;" + info.serialize();
		std::string response;
	    try{
	        udp_sendmsg(message, info.node_list_[i].ip_addr_, std::stoi(info.node_list_[i].port_), response);
	    }catch(boost::system::system_error const& e){
	    	std::cout << "Error sending message\n";
	    }
	} 
}

/*
 * this function receives the hearbeat from the neighbour
 * @param string request 
 * @param string r_ep
 */
std::string heartbeat_handler(std::string& request, udp::endpoint r_ep){
	std::vector<std::string> vs1;
    boost::split(vs1, request , boost::is_any_of(";"));
    if(vs1[0] == "JOIN"){
    	info_m.lock();
    	info.node_list_.push_back(node(r_ep.address().to_string(),std::to_string(r_ep.port())));
    	info_m.unlock();
    	std::thread t(update_nodes);
    	return info.serialize();
    }else if(vs1[0] == "UPDATE"){
    	std::string req = request.substr(8);
    	info.deserialize(req);
    }
    else if(vs1[0] == "HEARTBEAT"){

    }
}


/*
 * This is the listener which listens to heartbeat from neighbours
 *
 */
void heartbeat_server(int port){
    try{

        boost::asio::io_service io_service;
        udp_server server(io_service, port, heartbeat_handler);
        std::cout << "[heartbeat Server] Started    \n";
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
void start_node(std::string u_ip_addr, std::string u_port, std::string i_ip_addr, std::string i_port){
	std::cout << "here\n";
	if(u_ip_addr == i_ip_addr && u_port == i_port){
		//case where this is the first node
		info.node_list_.push_back(node(u_ip_addr,u_port));
		std::cout << "here\n";
	}else{
		std::string response;
		std::string message = "JOIN";
		try{
	        udp_sendmsg(message, i_ip_addr, std::stoi(i_port), response);
	    }catch(boost::system::system_error const& e){
	    	std::cout << "Error \n";
	    }
	    info.deserialize(response);
	}
}

int main(int argc, char* argv[]){
	std::cout << "starting node \n";
	if(argc < 5)
	{
		std::cout << "Invalid parameters, enter user ip and port and introducer ip and port";
		return -1;
	}
	std::string u_ip_addr,u_port,i_ip_addr, i_port;
	u_ip_addr = argv[1];
	u_port = argv[2];
	i_ip_addr = argv[3];
	i_port = argv[4];
	std::thread t1(heartbeat_server,stoi(u_port));
	std::thread t2(start_node, u_ip_addr,u_port,i_ip_addr,i_port);
	while(1){
		int choice;
		std::cout << "Type 1 to print member list \n" << std::endl;
		std::cin >> choice;
		switch(choice){
			case 1:
			{
				for(int i=0; i < info.node_list_.size();++i){
					std::cout << info.node_list_[i].ip_addr_ << " : " << info.node_list_[i].port_ << std::endl;
				}
			}
			break;
		}
	}


}