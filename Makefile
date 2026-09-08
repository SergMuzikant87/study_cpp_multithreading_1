build:
	g++ -std=c++11 -Wall -O2 programm_1.cpp -o programm_1
	g++ -std=c++11 -Wall programm_2.cpp -o programm_2_no_optomize
	g++ -std=c++11 -Wall -O2 programm_2.cpp -o programm_2_optomise_2
	g++ -std=c++11 -Wall -Os programm_2.cpp -o programm_2_optomise_for_fast
	
