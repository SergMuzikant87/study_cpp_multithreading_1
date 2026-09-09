build:
	g++ -std=c++11 -pthread -Wall -O2 programm_1.cpp -o programm_1
	g++ -std=c++11 -pthread -Wall programm_2.cpp -o programm_2_no_optomize
	g++ -std=c++11 -pthread -Wall -O2 programm_2.cpp -o programm_2_optomise_2
	g++ -std=c++11 -pthread -Wall -Os programm_2.cpp -o programm_2_optomise_for_fast
	
