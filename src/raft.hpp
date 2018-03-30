#include <string>
#include <vector>

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
	int req_type_;
	std::string key_;
	std::string val_;
	bool committed_;
	public:
	log_entry(int req, std::string key, std::string val)
	: req_type_(req) , key_(key) , val_(val)
	{
		committed_ = false;
	}
};

class log{
	std::vector<log_entry> entries;
	int cmt_cnt_;
	int st_cnt_;
	public:
	log(){
		st_cnt_ = 0;
		cmt_cnt_ = 0;
	}
};