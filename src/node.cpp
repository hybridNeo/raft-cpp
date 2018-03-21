#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "com.hpp"

class node{
public:
	std::string port_;
	std::string ip_addr_;
	
	node(std::string ip_addr, std::string port)
	:port_(port),ip_addr_(ip_addr){

	}

};


class node_info{
public:
	std::string leader_;
	int num_nodes_;
	std::vector<node> node_list_;
}info;

/*
 * this function receives the hearbeat from the neighbour
 * @param string request 
 * @param string r_ep
 */
std::string heartbeat_handler(std::string& request, udp::endpoint r_ep){

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
	heartbeat_server(stoi(u_port));
	if(u_ip_addr == i_ip_addr && u_port == i_port){
		//case where this is the first node
		info.num_nodes_ = 1;
		info.node_list_.push_back(node(u_ip_addr,u_port));
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

	start_node(u_ip_addr,u_port,i_ip_addr,i_port);


}