#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "com.hpp"
#include <mutex>
#include "raft.hpp"
#include <thread>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <ctime>

class node{
public:
	std::string port_;
	std::string ip_addr_;
	
	node(std::string ip_addr, std::string port)
	:port_(port),ip_addr_(ip_addr){

	}
	node(){
		port_ = "";
		ip_addr_ = "";
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

class election{
public:
	int num_votes_;
};

class node_info{
public:
	std::vector<node> node_list_;
	bool vote_available_;
	bool leader_tout_;
	std::mutex vote_m_;
	node cur_;
	node leader_;
	node_info(){
		vote_available_ = true;
		leader_tout_ = true;
	}
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
    	for( int i=0; i < vs1.size()-1;++i ){
    		std::string s = vs1[i];
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
    std::string ret = "OK";
    if(vs1[0] == "JOIN"){
    	info_m.lock();
    	info.node_list_.push_back(node(r_ep.address().to_string(),vs1[1]));
    	info_m.unlock();
    	std::thread t(update_nodes);
    	t.detach();
    	//std::cout << "info : " << info.serialize();
    	return info.serialize();
    }else if(vs1[0] == "UPDATE"){
    	std::string req = request.substr(7);
    	info.deserialize(req);

    }else if(vs1[0] == "ELECT"){
    	info.vote_m_.lock();
    	if(info.vote_available_ == true){
    		info.vote_available_ = false;
    		info.vote_m_.unlock();
    		return "OK";
    	}else{
    		info.vote_m_.unlock();
    		return "NOK";
    	}

    }else if(vs1[0] == "LEADER"){
    	info.leader_tout_ = false;
    	std::cout << "heartbeat recevied\n";
    }
    else if(vs1[0] == "HEARTBEAT"){

    }
    return ret;
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
	if(u_ip_addr == i_ip_addr && u_port == i_port){
		//case where this is the first node
		info.node_list_.push_back(node(u_ip_addr,u_port));
	}else{
		std::string response;
		std::string message = "JOIN;" + u_port;
		try{
	        udp_sendmsg(message, i_ip_addr, std::stoi(i_port), response);
	    }catch(boost::system::system_error const& e){
	    	std::cout << "Error \n";
	    }
	    //std::cout << "Response is " << response << std::endl;
	    info.deserialize(response);
	}
}

void leader_fn(){
	while(1){
		for(int i=0 ; i < info.node_list_.size();++i){
			std::string response;
			std::string message = "LEADER;" ;
			udp_sendmsg(message,info.node_list_[i].ip_addr_, std::stoi(info.node_list_[i].port_),response);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(HB_FREQ));
	}
	

}
void start_election(){
	std::string message = "ELECT;" + info.cur_.ip_addr_ + info.cur_.port_ ;	
	int num_votes = 0;
	for(int i =0; i < info.node_list_.size();++i){
		std::string response;
		udp_sendmsg(message,info.node_list_[i].ip_addr_, std::stoi(info.node_list_[i].port_),response);
		if(response == "OK"){
			num_votes++;
		}
	}
	if(num_votes+1 > (info.node_list_.size()/2)){
		std::cout << "Leader Elected \n";
		std::thread t(leader_fn);
		t.detach();
	}
	std::cout << "num votes is " << num_votes << "\n";

}

void send_heartbeat(){

}

void start_raft(){
	
	info.leader_tout_ = true;
	int sleep_amt = LOWER_TIMEOUT + (rand() % (UPPER_TIMEOUT - LOWER_TIMEOUT));
	std::cout << "Sleeping for " << sleep_amt << "milliseconds \n";
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_amt));
	std::cout << "here1\n";
	if(info.node_list_.size() >= NODE_THRESHOLD && info.leader_tout_ == true){
		info.vote_m_.lock();
		if(info.vote_available_ == true){
			std::cout << "here2\n";
			info.vote_available_ = false;
			info.vote_m_.unlock();
			start_election();

		}else{
			send_heartbeat();
			info.vote_m_.unlock();
		}
		
	}
	info.vote_available_ = true;
	start_raft();
}

int main(int argc, char* argv[]){
	srand (time(NULL));
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
	info.cur_.port_ = std::stoi(u_port);
	info.cur_.ip_addr_ = u_ip_addr;
	std::thread t1(heartbeat_server,stoi(u_port));
	std::thread t2(start_node, u_ip_addr,u_port,i_ip_addr,i_port);
	std::thread t3(start_raft);
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