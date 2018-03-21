#include "com.hpp"
#include <iostream>
#include <fstream>
#define DEBUG 0



/*
 * The request we receive has unnecessary additional characters even after the terminating character. so we will clean the string 
 * @param str input string
 * @return string cleaned string
 */
std::string clean_string(std::string& str){
    std::string new_str;
    for(int i= 0; i < str.size(); ++i){
        if(str[i] == 0 ){
            break;
        }
        new_str += str[i];
    }
    return new_str;
}

/*
 * Function to send a file to another node in the network
 * @param string ep_ip The ip address of the end point
 * @param int ep_port The port of the end point
 * @param string message The message to be sent
 * @param string response The response received from the endpoint
 * @return int Returns 0 on failure and returns 1 on success 
 */
int tcp_send_file(std::string& ep_ip, int ep_port, std::string& message, file_meta_data fm, std::string& response){
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(ep_ip),ep_port);
    try{
        boost::asio::ip::tcp::socket socket(io_service);
        boost::system::error_code error;
        socket.connect(ep);
        int total_blocks = fm.file_size/FILE_BLOCK_SIZE + 2;
        std::cout << "Total Blocks is " << total_blocks << std::endl;
        boost::asio::write(socket,boost::asio::buffer(std::to_string(total_blocks)),error);
        boost::asio::write(socket,boost::asio::buffer(message),error);
        for(int i=0; i < 10; ++i){
            std::vector<char> buf(FILE_BLOCK_SIZE);
            //buf = new std::vector<char>(BUF_SIZE);
           // std::copy(message.begin(), message.end(), buf.begin());
            //boost::asio::write(socket, boost::asio::buffer(buf), error);
            //int bytes_read = socket.read_some(boost::asio::buffer(buf) , error );
            // int bytes_read = boost::asio::read(socket, boost::asio::buffer(buf), error);
            // std::cout << "BYTES READ : " << bytes_read << std::endl;
            // std::copy(buf.begin(), buf.begin() + bytes_read , std::back_inserter(response));    
        }
        socket.close();
    }catch(std::exception& e){
        //std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}



void udp_sendmsg(std::string request, std::string host, int port_no, std::string& response)
{
    
    boost::asio::io_service io_service;
    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(), host, request);
    udp::endpoint receiver_endpoint(boost::asio::ip::address::from_string(host),port_no); //*resolver.resolve(query);
    udp::socket socket(io_service);
      
    try{
        socket.open(udp::v4());
        std::vector<char> send_buf(512);
        std::copy(request.begin(), request.end(), send_buf.begin());
        socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);
        boost::array<char, 512> recv_buf;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(
        boost::asio::buffer(recv_buf), sender_endpoint);
        socket.close();
        std::copy(recv_buf.begin(), recv_buf.begin() + len , std::back_inserter(response));
        //std::cout.write(recv_buf.data(), len);
    }catch (std::exception& e)
    {
        socket.close();
        std::cerr << "Exception :" << e.what() << std::endl;
    }

}





std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    std::string value= ctime(&now);
    value+="Alive";
    return value;
}

tcp_file_server::tcp_file_server(boost::asio::io_service& io_service, int port_no): acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port_no)){
    start_accept();
}

void tcp_file_server::start_accept(){
    std::cout << "Here 1\n";
    tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.get_io_service());
    acceptor_.async_accept(new_connection->socket(), boost::bind(&tcp_file_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
}

void tcp_file_server::handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error){
    std::cout << "Here 2\n";
    if (!error){
        std::cout << "Here 3\n";
        new_connection->start();
        start_accept();
    }
}

udp_server::udp_server(boost::asio::io_service& io_service, int port_no , std::string (*req_processor)(std::string&, udp::endpoint )): socket_(io_service, udp::endpoint(udp::v4(), port_no))
{
    req_processor_ = req_processor;  
    start_receive();
}

void udp_server::start_receive(){
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&udp_server::handle_receive, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void udp_server::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred){
    if (!error || error == boost::asio::error::message_size){
        if(DEBUG)
            std::cout << "Received Data is " << recv_buffer_.data()  << " And size is " << bytes_transferred << std::endl;
        std::string param(recv_buffer_.data());
        param = clean_string(param);
        std::string temp_ins = req_processor_(param,remote_endpoint_);
        boost::shared_ptr<std::string> message( new std::string(temp_ins));
        socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_ , boost::bind(&udp_server::handle_send, this, message, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        start_receive();
    }
}

void udp_server::handle_send(boost::shared_ptr<std::string> /*message*/,const boost::system::error_code& /*error*/, std::size_t /*bytes_transferred*/)
{

}

