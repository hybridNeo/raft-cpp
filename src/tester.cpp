#include <iostream>
#include "raft.hpp"
#include <string>

bool test_ser_deser(){
	std::cout << "Testing serialize deserialize \n";
	
	log_t l;
	log_entry ent(SET,"a","10");
	l.push_back(ent);
	log_entry ent2(SET,"b","100");
	l.push_back(ent2);
	std::string case1 = l.serialize();
	std::cout << case1 << "\n";
	std::cout << log_t(case1).serialize() << "\n";
}

int main(void){
	test_ser_deser();
}