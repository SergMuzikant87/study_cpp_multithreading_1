build:
	g++ -pthread -std=c++11 -Wall -O2 programm_1.cpp -o programm_1
	g++ -pthread -std=c++11 -Wall programm_2.cpp -o programm_2_no_optomize
	g++ -pthread -std=c++11 -Wall -O2 programm_2.cpp -o programm_2_optomise_2
	g++ -pthread -std=c++11 -Wall -Os programm_2.cpp -o programm_2_optomise_for_fast
	
