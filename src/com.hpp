#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#define FILE_BLOCK_SIZE 100000
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

class file_meta_data{
public:
	char file_name[256];
	long long int file_size;

};


void udp_sendmsg(std::string str_val, std::string host , int port_no, std::string& response);

int tcp_send_file(std::string& ep_ip, int ep_port, std::string& message, file_meta_data fm, std::string& response);

std::string clean_string(std::string& str);

class udp_server{
public:
	udp_server(boost::asio::io_service& , int port_no , std::string (*req_processor_)(std::string& , udp::endpoint ));
private:
	udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::string (*req_processor_)(std::string&, udp::endpoint );
    boost::array<char, 512> recv_buffer_;
    void udp_sendmsg(std::string str_val, std::string host);
	void start_receive();
	void handle_receive(const boost::system::error_code& error,std::size_t /*bytes_transferred*/);
	void handle_send(boost::shared_ptr<std::string> /*message*/,const boost::system::error_code& /*error*/, std::size_t /*bytes_transferred*/);

};


class tcp_connection:public boost::enable_shared_from_this<tcp_connection>{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_service& io_service){
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket(){
		return socket_;
	}

	void start(){
		message_ = "test";
		std::vector<char> buf(FILE_BLOCK_SIZE);
		boost::system::error_code error;
		std::cout << "here 4\n";
		int req_bytes_read = boost::asio::read(socket_, boost::asio::buffer(buf), error);
		std::string contents;
		std::copy(buf.begin(), buf.begin() + req_bytes_read , std::back_inserter(contents));
		std::cout << "received data is " << contents << std::endl;
		if(seq_ == 0){
			total_blocks_ = stoi(contents);
		}else if(seq_ == 1){
			std::cout << "filename " << contents << std::endl;
		}else{

		}
		seq_++;
		boost::asio::async_write(socket_, boost::asio::buffer(message_),
		boost::bind(&tcp_connection::handle_write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred,true));
	}
	tcp_connection(boost::asio::io_service& io_service): socket_(io_service){
		std::cout << "in ctr \n";
		seq_ = 0;
		total_blocks_ = 0;
	}

private:

	void handle_write(const boost::system::error_code& error, size_t bytes_transferred,bool done){
		if(seq_ != total_blocks_){
			start();
		}
		std::cout << "Done\n";
  	}
  	int seq_;
  	int total_blocks_;
	tcp::socket socket_;
	std::string message_;
};

class tcp_file_server{
public:
	tcp_file_server(boost::asio::io_service& io_service, int port_no);
private:
	void start_accept();
	void handle_accept(tcp_connection::pointer new_connection,const boost::system::error_code& error);
	tcp::acceptor acceptor_;
};




  