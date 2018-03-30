#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>


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
		log_entry(deserialize(in));
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

	log_entry deserialize(std::string in){
		std::vector<std::string> vs1;
   	 	boost::split(vs1, in , boost::is_any_of(","));
   	 	log_entry temp(std::stoi(vs1[0]),vs1[1],vs1[2]);
   	 	return temp;
	}
};

class log_t : public std::vector<log_entry>{
	int cmt_cnt_;
	int st_cnt_;
	public:
	log_t(){
		st_cnt_ = 0;
		cmt_cnt_ = 0;
	}
	std::string serialize(){
		std::string res = std::to_string(st_cnt_) + ";" + std::to_string(cmt_cnt_) + ";" ;
		for(int i=0; i < std::vector<log_entry>::size(); ++i){

			res += std::vector<log_entry>::operator[](i).serialize()  + ";";
		}
	}
};