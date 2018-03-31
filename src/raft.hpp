#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <iostream>

#define LOWER_TIMEOUT 800
#define UPPER_TIMEOUT 2000
#define HB_FREQ 200
#define API_PORT 4040
#define VOTE_TIME 100
#define NODE_THRESHOLD 5

void start_raft();
void start_election();
//TYPES OF REQUESTS
#define GET 10111
#define SET 10112



class log_entry{
public:
	int req_type_;
	std::string key_;
	std::string val_;
	bool committed_;
	log_entry(int req, std::string key, std::string val)
	: req_type_(req) , key_(key) , val_(val)
	{
		committed_ = false;
	}

	log_entry(std::string in){
		std::vector<std::string> vs1;
   	 	boost::split(vs1, in , boost::is_any_of(","));
   	 	req_type_ = std::stoi(vs1[0]);
   	 	key_ = (vs1[1]);
   	 	val_ = (vs1[2]);
   	 	committed_ = std::stoi(vs1[3]);
   	 }

	log_entry(const log_entry& l){
		req_type_ = l.req_type_;
		key_ = l.key_;
		val_ = l.val_;
		committed_ = l.committed_;
	}


	std::string serialize(){
		std::string res = std::to_string(req_type_) + "," + key_ + "," + val_ + "," + std::to_string((int)committed_) ;
		return res;
	}

};

class log_t : public std::vector<log_entry>{
	public:
	int cmt_cnt_;
	int st_cnt_;
	log_t() : std::vector<log_entry>() {
		st_cnt_ = 0;
		cmt_cnt_ = 0;
	}

	log_t(std::string in): std::vector<log_entry>()  {
		std::vector<std::string> vs1;
   	 	boost::split(vs1, in , boost::is_any_of(";"));
   	 	st_cnt_ = std::stoi(vs1[0]);
   	 	cmt_cnt_ = std::stoi(vs1[1]);
   	 	for(int i=2;i < vs1.size()-1; ++i){
   	 		log_entry e(vs1[i]);
   	 		std::vector<log_entry>::push_back(e);;
   	 	}

	
	}


	std::string serialize(){
		std::string res = std::to_string(st_cnt_) + ";" + std::to_string(cmt_cnt_) + ";" ;
		for(int i=0; i < std::vector<log_entry>::size(); ++i){
			res += std::vector<log_entry>::operator[](i).serialize()  + ";";
		}
		return res;
	}


};

int max(int a , int b){
	if( a > b)
		return a;
	return b;
}

