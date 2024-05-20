# RL_GA_LS_Greedy_ThOP
Reinforcement Genetic Algorithm with Local Search &amp; Simple Randomized Greedy Heuristic for ThOP

# How to run my code
First, install and unzip all the files in the github
Then, in the run_testcase.py file, change the directory into where you put the hold thing (for example my is in C disk)
Then, compile the file RLS_GA_RL_S.cpp using the following command in Windows, Linus, etc
g++ RLS_GA_RL_LS.cpp -o main.exe -std=c++11 -O3
Then, if you want to run a file by itself, in Terminal you write:
main.exe testcase_submit\eil51_01_bsc_01_01.thop 23521285 5 
The eil51 can be replace by the file name (with .thop), the 23521285 is the random seed number, the 5 is the time limit, 
if you want to run all the file, use:
python run_testcase.py

