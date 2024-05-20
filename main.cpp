#include <bits/stdc++.h>
#include <chrono>
#include "MTRand.h"
using namespace std;
MTRand rng;

struct Item {
    int index;
    int profit;
    int weight;
    int index_node;
};

struct Point {
    int index;
    double x;
    double y;
    vector<Item> bags;
};


template <typename T>
void randomshuffle(vector<T> &arr) {
    int n = arr.size();
    for (int i = n-1; i > 0; i--) {
        int j = rng.randInt(i);
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }

    return;
}


int DIMENSION, ITEMS; // Số đỉnh trong đồ thị, số items;
int CAPACITY; // Khối lượn tối đa của túi
float MAX_TIME; // Thời gian tối đa
double v_min, v_max; // Tốc độ nhỏ nhất, tốc độ lớn nhất
vector<vector<double>> dist;
vector<Point> graph; // Đồ thị
vector<Item> bags; // Túi đồ

vector<float> pheromone_table;

int UPPER_BOUND;








struct Solution {
    float time_travel;
    long int w_profit = 0;
    long int profit = 0;
    vector<int> item_taken;
    vector<int> city_travel;
    bool valid = true;


    void constructCity() {
        city_travel.clear();
        vector<bool> visit(DIMENSION+1,false);
        city_travel.push_back(1);

        for (auto i: item_taken) {
            if (visit[bags[i].index_node] == false) {
                city_travel.push_back(bags[i].index_node);
                visit[bags[i].index_node] = true;
            }
        }
        
        return;
    };

    void shuffle_city() {
        int n = city_travel.size();
        if (n <= 2) return;
        int index = rng.randInt(n-2)+1;
        int index_2 = rng.randInt(n-2)+1;

        int left = min(index,index_2);
        int right = max(index,index_2);
        for (int i = 0; 2*i <= right-left; i++) {
            swap(city_travel[left+i],city_travel[right-i]);
        }

        return;
    }

    void repair_travel(vector<int> &travel) {
        vector<int> visit(DIMENSION+1,false);
        for (auto i: item_taken) {
            visit[bags[i].index_node] = true;
        }
        visit[1] = true;

        city_travel.clear();
        for (auto i: travel) {
            if (visit[i] == true) {
                city_travel.push_back(i);
            }
        }
        return;
    }

    void mutation_item() {
        vector<int> bin_1(ITEMS+1,0);
        for (auto i: item_taken) {
            bin_1[i] = 1;
        }
        int index = rng.randInt(ITEMS-1)+1;
        bin_1[index] = 1 - bin_1[index];
        item_taken.clear();
        for (int i = 1; i <= ITEMS; i++) {
            if (bin_1[i] == 1) {
                item_taken.push_back(i);
            }
        }
        return;
    }


    void mutation_city() {
        vector<int> bin_1(DIMENSION+1,false);
        for (auto i : city_travel) {
            bin_1[i] = true;
        }

        vector<int> possible;
        for (int i = 2; i < DIMENSION; i++) {
            if (bin_1[i] == false) {
                possible.push_back(i);
            }
        }

        if (possible.size() == 0) return;

        randomshuffle<int>(possible);

        city_travel.push_back(possible[0]);
        return;
    }

    bool optimize_time() {
        if (city_travel.size() <= 2) return true;
        float very_old_time = time_travel;
        int n = city_travel.size();
        for (int i = 1; i < n; i++) {
            for (int j = i+1; j < n; j++) {
                float old_time = time_travel;
                swap(city_travel[i],city_travel[j]);
                checkerSolution((*this),false);
                if (old_time <= time_travel) {
                    swap(city_travel[i],city_travel[j]);
                    checkerSolution((*this),false);
                }
            }
        }
        checkerSolution((*this),false);
        if (very_old_time <= time_travel) {
            return false;
        }
        return true;
    }

    bool nearestNeighbor_insertion() {
        vector<int> visit(DIMENSION+1,false);
        for (auto i: city_travel) {
            visit[i] = true;
        }

        vector<int> possible;
        for (int i = 2; i < DIMENSION; i++) {
            if (visit[i] == false) possible.push_back(i);
        }

        int n = city_travel.size();
        int last = city_travel[n-1];

        if (possible.size() == 0) return false; 
        
        sort(possible.begin(), possible.end(),
            [last](int i, int j){
                return dist[i][last] < dist[j][last];
            });
        
        city_travel.push_back(possible[0]);
        if (checkerSolution((*this),false) == false) {
            return false;
        }
        return true;
    }

    void ILS_optimize_time() {
        Solution s;
        if (city_travel.size() <= 2) return;
        s.item_taken = item_taken;
        s.city_travel = city_travel;
        s.shaking_time();
        for (int i = 1; i <= 3; i++) {
            if (s.optimize_time() == false) break;
        }
        if (s.time_travel < time_travel) {
            city_travel.clear();
            city_travel = s.city_travel;
        }
        return;
    }

    void adapt_more_item() {
        int old_profit = profit;
        vector<bool> bin(ITEMS+1,false);
        vector<bool> visit(DIMENSION+1,false);
        for (auto i: item_taken) {
            bin[i] = true;
        }
        for (auto i: city_travel) {
            visit[i] = true;
        }

        vector<double> TSP_distance(DIMENSION+1,0.0);
        int city_travel_size = city_travel.size();
        for (int i = 1; i < city_travel.size(); i++) {
            TSP_distance[city_travel[i]] = TSP_distance[city_travel[i-1]] + dist[city_travel[i-1]][city_travel[i]];
        }
        int last_city_TSP = city_travel[city_travel_size-1];
        for (int i = 1; i <= DIMENSION; i++) {
            if (visit[i] == false) {
                TSP_distance[i] = TSP_distance[last_city_TSP] + dist[last_city_TSP][i];
            }
        }

        vector<int> possible;
        for (int i = 1; i <= ITEMS; i++) {
            if (bin[i] == false) {
                possible.push_back(i);
            }
        }

        sort(possible.begin(), possible.end(),
            [TSP_distance](int i, int j) {
                return (bags[i].profit)*(bags[j].weight) > (bags[j].profit)*(bags[i].weight);
            });
        int L = 0;
        for (auto i: possible) {
            if (L == 3) {
                for (int cnt = 1; cnt <= 3; cnt++) {
                    if (optimize_time() == false) ILS_optimize_time();
                }
                L = 0;
            }
            item_taken.push_back(i);
            int checker = 0;
            if (visit[bags[i].index_node] == false) {
                city_travel.push_back(bags[i].index_node);
                checker = 1;
                visit[bags[i].index_node] = true;
            }
            if (checker == 1) {
                fast_calculation_adding(i,bags[i].index_node);
            } else {
                checkerSolution((*this),false);
            }
            if (valid == false) {
                if (checker == 1) {
                    item_taken.pop_back();
                    city_travel.pop_back();
                    visit[bags[i].index_node] = false;
                }
                else {
                    item_taken.pop_back();
                }
            }
            checkerSolution((*this),false);

            L++;
        }

        // if (profit > old_profit) {
        //     cout << "GREEDY ADAPT \n";
        // }

        return;
    }




    void polish() {
        vector<int> after_polish;
        vector<int> bin_1(ITEMS+1,false);
        for (auto i: item_taken) {
            bin_1[i] = true;
        }
        bin_1[0] = true;

        for (auto i: city_travel) {
            bool has_item = false;
            for (auto j: graph[i].bags) {
                if (bin_1[j.index] == true) {
                    has_item = true;
                    break;
                }
            }
            if (has_item == true) {
                after_polish.push_back(i);
            }
        }

        city_travel.clear();
        city_travel.push_back(1);
        for (auto i: after_polish) {
            city_travel.push_back(i);
        }

    }

    void shaking_time() {
        int n = city_travel.size();
        if (n <= 2) return;
        int index = rng.randInt(n-2) + 1;
        int index2 = rng.randInt(n-2) + 1;
        swap(city_travel[index],city_travel[index2]);
        return;
    }



    


    bool checkerSolution(Solution &s, bool enable) {
        s.city_travel.push_back(DIMENSION);
        vector<int> Weight(DIMENSION+1,0);
        for (int i = 0; i < s.item_taken.size(); i++) {
            Weight[bags[s.item_taken[i]].index_node] += bags[s.item_taken[i]].weight;
        }
        int w = 0;
        double v = v_max;
        float time = 0.0;
        for (int i = 1; i < s.city_travel.size(); i++) {
            time += 1.0*(dist[s.city_travel[i]][s.city_travel[i-1]])/v;
            w += Weight[s.city_travel[i]];
            v = v_max - 1.0*w*(v_max-v_min)/CAPACITY;
            if (enable) {
                cout << "Time:" << time << endl;
                cout << "Weight:" << w << endl;
                cout << "Speed:" << v << endl;
                cout << "------" << endl;
            }
        }
        s.city_travel.pop_back();
        s.w_profit = w;

        if (w > CAPACITY) {
            s.time_travel = 10*1.0*MAX_TIME;
            s.profit = 0;
            return valid = false;
        } else {
            if (time > MAX_TIME) {
                s.time_travel = time;
                s.profit = 0;
                return valid = false;
            }
        }
        s.time_travel = time;
        s.profit = 0;
        for (auto x: item_taken) {
            s.profit += bags[x].profit;
        }
        return valid = true;
    };

    void fast_calculation_adding(int i, int j) {
        int n = city_travel.size();
        int last_city = city_travel[n-2];
        double v = v_max - (w_profit+bags[i].weight)*(v_max-v_min)/CAPACITY;
        float t_bonus = 1.0*(dist[last_city][j]/v) + 1.0*(dist[j][DIMENSION]/v);
        time_travel += t_bonus;
        if (v >= v_min && time_travel < MAX_TIME) {
            w_profit += bags[i].weight;
            time_travel -= 1.0*(dist[j][DIMENSION]/v);
            profit += bags[i].profit;
            valid = true;
        } else {
            valid = false;
        }
    }
};


struct Permutation_solver {
    vector<int> destination;
    
    void construct() {
        for (int i = 1; i < DIMENSION; i++) {
            destination.push_back(i);
        }
        return;
    }

    void create_new_perm(vector<int> &seed_perm) {
        Solution s;
        s.city_travel = seed_perm;
        s.optimize_time();
        vector<int> visit(DIMENSION+1,false);
        visit[1] = true;
        destination.clear();
        destination.push_back(1);
        for (auto i: s.city_travel) {
            if (i == 1) continue;
            visit[i] = true;
            destination.push_back(i);
        }

        vector<int> path_optimize;
        for (int i = 2; i < DIMENSION; i++) {
            if (visit[i] == false) {
                path_optimize.push_back(i);
            }
        }
        Solution best_path;
        best_path.city_travel = path_optimize;
        best_path.checkerSolution(best_path,false);
        bool flag = false;
        for (int i = 1; i <= 3; i++) {
            Solution candidate;
            if(flag) randomshuffle<int>(path_optimize);
            candidate.city_travel = path_optimize;
            candidate.optimize_time();
            candidate.checkerSolution(candidate,false);
            if (candidate.time_travel < best_path.time_travel) {
                best_path = candidate;
                best_path.time_travel = candidate.time_travel;
                flag = false;
            } else {
                flag = true;
            }
            path_optimize = candidate.city_travel;
        }

        for (auto i: best_path.city_travel) {
            destination.push_back(i);
        }

        return;
    }

    void random_shuffle() {
        randomshuffle<int>(destination);
        return;
    }
};

Permutation_solver TSP_solver;



struct Algorithm {
    void random_GreedyHeuristic(Solution &a) {
        vector<bool> bin_1(ITEMS+1,false);
        vector<bool> visit(DIMENSION+1,false);
        for (auto i: a.item_taken) {
            bin_1[i] = true;
        }
        for (auto i: a.city_travel) {
            visit[i] = true;
        }

        vector<int> possible;
        for (int i = 1; i <= ITEMS;i++) {
            if (bin_1[i] == false) {
                possible.push_back(i);
            }
        }
        randomshuffle<int>(possible);

        int L_shake = 0;

        for (auto i: possible) {
            if (L_shake == 3) {
                if (a.optimize_time() == false) a.ILS_optimize_time();
                L_shake = 0;
            }
            if (1.0*bags[i].profit < 1.0*bags[i].weight*0.26) continue;
            a.item_taken.push_back(i);
            int checker = 0;
            if (visit[bags[i].index_node] == false) {
                a.city_travel.push_back(bags[i].index_node);
                checker = 1;
                visit[bags[i].index_node] = true;
            }
            if (a.checkerSolution(a,false) == false) {
                if (checker == 1) {
                    a.item_taken.pop_back();
                    a.city_travel.pop_back();
                    visit[bags[i].index_node] = false;
                } else {
                    a.item_taken.pop_back();
                }
            }
            L_shake++;
            a.checkerSolution(a,false);
        }

        a.checkerSolution(a,false);
    };

    pair<Solution,Solution> pertubation_Heuristic(Solution &a) {
        Solution b;
        b.item_taken = a.item_taken;
        b.city_travel = a.city_travel;
        b.checkerSolution(b,false);

        int n = b.item_taken.size();
        vector<int> bin(ITEMS+1,0);
        vector<int> possible;
        // cout << "Pertubation debug \n";
        // for (auto i: b.item_taken) {
        //     possible.push_back(i);
        //     cout << i << " ";
        //     bin[i] = 1;
        // }
        // cout << "\n";
        for (auto i: b.item_taken) {
            possible.push_back(i);
            bin[i] = 1;
        }

        if (possible.size() == 0) {
            return make_pair(b,a);
        }



        sort(possible.begin(), possible.end(),
            [](int i, int j) {
                return (bags[i].profit)*(bags[j].weight) < (bags[j].profit)*(bags[i].weight);
            });
        bin[possible[0]] = 0;
        



        b.item_taken.clear();
        for (int i = 1; i <= ITEMS; i++) {
            if (bin[i] == 1) {
                b.item_taken.push_back(i);
            }
        }
        b.constructCity();

        set<int> have_visit;
        for (auto i: b.city_travel) {
            have_visit.insert(i);
        }

        Solution c;
        c.item_taken = b.item_taken;
        for (auto i: a.city_travel) {
            if (have_visit.count(i)) {
                c.city_travel.push_back(i);
            }
        }
        c.ILS_optimize_time();




        // for (auto i: b.item_taken) {
        //     cout << i  << " "; 
        // }
        // cout << "End debug \n";
        return make_pair(b,c);

    }


    Solution LargeNeighborhood_Heuristic(Solution &a) {
        vector<bool> visit(DIMENSION+1,false);
        for (auto i: a.city_travel) {
            visit[i] = true;
        }
        visit[1] = true;
        visit[DIMENSION] = true;

        vector<int> neighborhood;
        for (int i = 1;  i <= DIMENSION; i++) {
            if (visit[i] == false) {
                neighborhood.push_back(i);
            }
        }

        // int n = a.city_travel.size();


        // // // cout << "SORTED\n";
        // int last_visit = a.city_travel[n-1];

        sort(neighborhood.begin(), neighborhood.end(), 
            [](int i, int j){
                int profit_1 = 0;
                int profit_2 = 0;
                int weight_1 = 0;
                int weight_2 = 0;
                for (auto x: graph[i].bags) {
                    profit_1 += x.profit;
                    weight_1 += x.weight;
                }
                for (auto x: graph[j].bags) {
                    profit_2 += x.profit;
                    weight_2 += x.weight;
                }

                float score_1 = (1.0*profit_1*weight_2);
                float score_2 = (1.0*profit_2*weight_2);

                return score_1 < score_2;
            });

        // if (neighborhood.size() > 1) {
        //     randomshuffle<int>(neighborhood);
        // }

        Solution s;
        s.item_taken = a.item_taken;
        s.city_travel = a.city_travel;
        s.city_travel.push_back(neighborhood[0]);
        int counter = 0;
        for (int i = 0; i < graph[neighborhood[0]].bags.size(); i++) {
            s.item_taken.push_back(graph[neighborhood[0]].bags[i].index);
            if (s.checkerSolution(s,false) == true) {
                counter = 1;
            } else {
                s.item_taken.pop_back();
            }
        }
        if (counter == 0) {
            s.city_travel.pop_back();
            s.ILS_optimize_time();
            return s;
        } else {
            s.checkerSolution(s,false);
            return s;
        }
        
    }

    Solution flip_basedHeuristic(Solution a) {
        vector<int> bin(ITEMS+1,0);
        vector<int> bin_1(ITEMS+1,0);
        for (auto i: a.item_taken) {
            bin[i] = 1;
        }


        vector<int> change;
        for (int i = 1; i <= ITEMS;i++) {
            double prob = rng.randDblExc();
            // cout << 1 << endl;
            // cout << "YES PLEASE \n";
            if ((double)1.0*ITEMS*prob <= 1) {
                bin_1[i] = 1 - bin[i];
                change.push_back(i);
            } else {
                bin_1[i] = bin[i];
            }
            // cout << 1 << endl;
        }

        Solution b;
        for (int i = 1; i <= ITEMS;i++) {
            if (bin_1[i] == 1) {
                b.item_taken.push_back(i);
            }
        }

        b.constructCity();
        // cout << 1 << endl;
        for (int i = 1; i <= 3; i++) {
            if (b.optimize_time() == false) {
                b.ILS_optimize_time();
            }
        }
        return b;
    }

    Solution geneticAlgorithm_Heuristic(Solution &a, Solution &b) {
        vector<int> bin(ITEMS+1,0);
        vector<int> bin_2(ITEMS+1,0);
        for (auto i: a.item_taken) {
            bin[i] = 1;
        }

        for (auto i: b.item_taken) {
            bin_2[i] = 1;
        }

        int index = rng.randInt(ITEMS-1)+1;
        for (int i = 1; i <= index; i++) {
            bin[i] = bin_2[i];
        }

        Solution c;
        for (int i = 1; i <= ITEMS; i++) {
            if (bin[i] == 1) c.item_taken.push_back(i);
        }
        c.constructCity();
        c = flip_basedHeuristic(c);
        return c;
    }

    Solution acceptanceRate_Heuristic(Solution &a) {
        Solution b;
        b.city_travel = a.city_travel;
        b.item_taken = a.item_taken;
        
        
        int cnt_iterated = 0;
        while (b.nearestNeighbor_insertion() == true) {
            if (cnt_iterated >= 100) break;
            int checker = 0;
            for (auto x: graph[b.city_travel.back()].bags) {
                b.item_taken.push_back(x.index);
                if (b.checkerSolution(b,false) == true) {
                    if (x.profit < 1.0*x.weight*0.3) {
                        b.item_taken.pop_back();
                        continue;
                    }
                    checker = 1;
                } else {
                    b.item_taken.pop_back();
                }
            }

            if (checker == 0) {
                b.city_travel.pop_back();
                if (b.optimize_time() == false) b.ILS_optimize_time();
                cnt_iterated++;
            } else {
                cnt_iterated = 0;
            } 
        }

        b.checkerSolution(b,false);

        return b;
    }


    Solution GRASP_Heuristic() {
        Solution a;
        a.city_travel.push_back(1);

        bool stopping_condition = false;

        vector<bool> visit(DIMENSION+1,false);
        vector<bool> taken(ITEMS+1,false);
        int L = 0;

        int cnt_stop = 0;

        while (true) {
            if (stopping_condition) break;

            if (L == 3) {
                for (int i = 1; i <= 3; i++) {
                    if (a.optimize_time() == false) a.ILS_optimize_time();
                }
                L = 0;
            }

            
            vector<int> possible;
            for (int i = 2; i < DIMENSION; i++) {
                if (visit[i] == false) {
                    possible.push_back(i);
                }
            }

            if (possible.size() == 0) break;


            int last_city = a.city_travel.back();
            int last_weight = a.w_profit;


            sort(possible.begin(), possible.end(),
                [last_city,last_weight](int i , int j) {
                    int profit_1 = 0;
                    int profit_2 = 0;
                    int weight_1 = 0;
                    int weight_2 = 0;
                    for (auto x: graph[i].bags) {
                        profit_1 += x.profit;
                        weight_1 += x.weight;
                    }
                    for (auto x: graph[j].bags) {
                        profit_2 += x.profit;
                        weight_2 += x.weight;
                    }

                    if (profit_1*weight_2 > profit_2*weight_1 && dist[i][last_city] < dist[j][last_city]) {
                        return true;
                    } else if (profit_1*weight_2 <= profit_2*weight_1 && dist[i][last_city] >= dist[j][last_city]) {
                            return false;
                    } else {
                        return 1.0*profit_1*weight_2*dist[j][last_city] > 1.0*profit_2*weight_1*dist[i][last_city];
                    } 
                        // float v_1 = v_max - 1.0*(weight_1+last_weight)*(v_max - v_min)/CAPACITY;
                        // float v_2 = v_max - 1.0*(weight_2+last_weight)*(v_max - v_min)/CAPACITY;
                        // float time_1 = dist[i][last_city]/v_1;
                        // float time_2 = dist[j][last_city]/v_2;

                        // return 1.0*profit_1*time_2 > 1.0*profit_2*time_1;
                    //     }
                });

            int n = min((int)possible.size(), 30);
            vector<int> next_city;
            for (int i = 0; i < n; i++) {
                next_city.push_back(possible[i]);
            }
            // vector<double> distance_city(n);
            // double sum_distance = 0.0;
            // for (int i = 0; i < n; i++) {
            //     distance_city[i] = dist[last_city][next_city[i]];
            //     sum_distance += distance_city[i];
            // }
            // double roulete_prob = rng.randDblExc()*sum_distance;
            // double choice = 0.0;
            // for (int i = 0; i < n; i++) {
            //     choice += distance_city[i];
            //     if (roulete_prob <= choice) {
            //         index = i;
            //         break;
            //     }
            // }
            int index = rng.randInt(n-1);
            a.city_travel.push_back(next_city[index]);
            int counter = 0;
            for (auto i: graph[next_city[index]].bags) {
                if (1.0*i.profit/i.weight < 0.14) continue;
                a.item_taken.push_back(i.index);
                Solution b = a;
                // for (auto i: b.city_travel) {
                //     cout << i << " ";
                // }
                // cout << "\n";
                // for (auto i: b.item_taken) {
                //     cout << i << " ";
                // }
                // cout << "\n";
                for (int i = 1; i <= 3; i++) {
                    if (b.optimize_time() == false) b.ILS_optimize_time();
                }
                b.checkerSolution(b,false);
                // cout << b.profit << endl;
                if (b.profit > 0) {
                    counter = 1;
                    a = b;
                    a.checkerSolution(a,false);
                } else {
                    a.item_taken.pop_back();
                }
            }

            if (counter == 0) {
                a.city_travel.pop_back();
                cnt_stop++;
            } else {
                visit[next_city[index]] = true;
                cnt_stop = 0;
            }


            if (cnt_stop == 30) {
                stopping_condition = true;
            }



            L++;

        };

        return a;
    }

    Solution simulatedAnnealing_Heuristic(Solution &a) {
        Solution best_solution = a;

        float T = 100.0;
        float alpha = 0.965;

        vector<bool> possible(ITEMS+1,false);
        for (auto i: a.item_taken) {
            possible[i] = true;
        }
        
        


        while (T > 1.0) {
            Solution b = a;
            vector<int> visit(DIMENSION+1,false);
            for (auto i: b.city_travel) {
                visit[i] = true;
            }

            for (int i = 1; i <= ITEMS; i++) {
                vector<int> next_item;
                for (int i = 1; i <= ITEMS; i++) {
                    if (possible[i] == false) {
                        next_item.push_back(i);
                    }
                }
                int index = rng.randInt((int)next_item.size()-1);
                b.item_taken.push_back(next_item[index]);
                int checker = 0;
                
                if (visit[bags[next_item[index]].index_node] == false) {
                    b.city_travel.push_back(bags[next_item[index]].index_node);
                    visit[bags[next_item[index]].index_node] = true;
                    checker = 1;
                }

                int prob = rng.randDblExc();
                b.checkerSolution(b,false);
                if (b.valid == false || exp(-(1.0*bags[next_item[index]].profit)/(T)) > prob) {
                    if (checker == 0) {
                        b.item_taken.pop_back();
                    } else {
                        b.item_taken.pop_back();
                        b.city_travel.pop_back();
                        visit[bags[next_item[index]].index_node] = false;
                    }
                } else {
                    possible[next_item[index]] = true;
                }
            }

            b.checkerSolution(b,false);

            if (b.profit > best_solution.profit) {
                best_solution = b;
                best_solution.checkerSolution(best_solution,false);
            }

            T = T*alpha;
        }

        if (best_solution.profit > a.profit) {
            return best_solution;
        } else return a;

        return best_solution;
    }

    Solution Evolution_pathRelinking_Heuristic(Solution &a, Solution &b) {
        // Solution b guide Solution a
        vector<bool> bin_1(ITEMS+1,false);
        for (auto i: a.item_taken) bin_1[i] = true;



        // for (auto i: a.item_taken) {
        //     cout << i << " ";
        // }
        // cout << "\n";

        // for (auto i: b.item_taken) {
        //     cout << i << " ";
        // }
        // cout << "\n";

        vector<int> possible;
        for (auto i: b.item_taken) {
            if (bin_1[i] == false) {
                possible.push_back(i);
            }
        }
        // for (auto i: possible) {
        //     cout << i << " ";
        // }
        // cout << "\n";

        Solution best_solution = a;
        best_solution.checkerSolution(best_solution,false);

        for (int i = 1; i <= 10; i++) {
            Solution c = a;
            vector<int> visit(DIMENSION+1,false);
            for (auto i: a.city_travel) visit[i] = true;

            vector<int> candidate;
            for (auto i: possible) {
                double prob = rng.randDblExc();
                if (prob < 0.5) {
                    candidate.push_back(i);
                }
            }

            for (auto i: candidate) {
                int checker = 0;
                c.item_taken.push_back(i);
                if (visit[bags[i].index_node] == false) {
                    c.city_travel.push_back(bags[i].index_node);
                    visit[bags[i].index_node] = true;
                    checker = 1;
                }

                c.checkerSolution(c,false);
                if (c.valid == false) {
                    if (checker == 0) {
                        c.item_taken.pop_back();
                    } else {
                        c.item_taken.pop_back();
                        c.city_travel.pop_back();
                        visit[bags[i].index_node] = false;
                    }
                }
            }

            c.checkerSolution(c,false);

            if (c.profit > best_solution.profit) {
                best_solution = c;
                best_solution.checkerSolution(best_solution,false);
            }
        }

        if (best_solution.profit > a.profit) {
            return best_solution;
        } else return a;
        return best_solution;

    }


    Solution Efficiently_CoSolver_Heuristic() {
        Solution a;
        a.city_travel.push_back(1);


        vector<bool> visit(DIMENSION+1,false);
        visit[1] = true;
        vector<bool> taken(ITEMS+1,false);


        bool stopping_condition = false;

        int cnt_retry = 0;
        int L_optimize = 0;
        while (true) {
            if (stopping_condition) break;

            if (L_optimize == 3) {
                for (int i = 1; i <= 2; i++) {
                    if (a.optimize_time() == false) {
                        a.ILS_optimize_time();
                    }
                }
                L_optimize = 0;
            }

            vector<int> possible;
            for (int i = 2; i < DIMENSION; i++) {
                if (visit[i] == false) {
                    possible.push_back(i);
                }
            }

            // Three state selection
            sort(possible.begin(), possible.end(),
                [](int i, int j) {
                    int profit_1 = 0;
                    int profit_2 = 0;
                    int weight_1 = 0;
                    int weight_2 = 0;
                    for (auto x: graph[i].bags) {
                        if (1.0*x.profit > 0.2*x.weight) {
                            profit_1 += x.profit;
                            weight_1 += x.weight;
                        } 
                    }  
                    
                    for (auto x: graph[j].bags) {
                        if (1.0*x.profit > 0.2*x.weight) {
                            profit_2 += x.profit;
                            weight_2 += x.weight;
                        }
                    }

                    return profit_1*weight_2 > profit_2*weight_1;
                });

            if (possible.size() == 0) {
                stopping_condition = true;
                continue;
            }
            
            vector<int> second_stage_possible;
            for (int i = 0; 2*i < possible.size(); i++) {
                second_stage_possible.push_back(possible[i]);
            }

            // Distance stage
            int last_city = a.city_travel.back();
            sort(second_stage_possible.begin(), second_stage_possible.end(),
                [last_city](int i, int j) {
                    return dist[i][last_city] < dist[j][last_city];
                });

            vector<int> third_stage_possible;
            for (int i = 0; 2*i < second_stage_possible.size(); i++) {
                third_stage_possible.push_back(second_stage_possible[i]);
            }

            int max_stage = (int)(third_stage_possible.size());
            for (int i = 0; i < max_stage; i++) {
                Solution b = a;
                b.city_travel.push_back(third_stage_possible[i]);
                for (auto x: graph[third_stage_possible[i]].bags) {
                    if (1.0*x.profit < x.weight*0.2) continue;
                    b.item_taken.push_back(x.index);
                    b.checkerSolution(b,false);
                    if (b.valid == false) {
                        b.item_taken.pop_back();
                    }
                }
                b.checkerSolution(b,false);

                if (b.profit > a.profit) {
                    a = b;
                }
            }

            
            int new_last_city = a.city_travel.back();
            if (new_last_city == last_city) {
                cnt_retry++;
            } else {
                visit[new_last_city] = true;
            }


            if (cnt_retry == 3) {
                stopping_condition = true;
                continue;
            }
            L_optimize++;
        }
        return a;
    }

    Solution random_Path_Reinforcement_Heuristic() {
        vector<int> city;
        for (auto i: TSP_solver.destination) {
            city.push_back(i);
        }
        
        vector<int> item_list;
        for (auto i: city) {
            for (auto x: graph[i].bags) {
                item_list.push_back(x.index);
            }
        }
        
        int n = (int)item_list.size();
        vector<vector<float>> reinforcement_table(n,vector<float>(2,0.5));
        float alpha_reward = 1.23;
        float belta_penalty = 1.19;


        Solution best_solution;
        best_solution.city_travel.push_back(1);
        best_solution.checkerSolution(best_solution,false);
        vector<int> best_solution_choice(n,1);

        for (int i = 1; i <= 1000; i++) {
            Solution s;
            s.city_travel.push_back(1);
            vector<bool> visit(DIMENSION+1,false);
            visit[1] = true;
            vector<int> choice(n,1);
            for (int i = 0; i < n; i++) {
                int checker = 0;
                s.item_taken.push_back(item_list[i]);
                if (visit[bags[item_list[i]].index_node] == false) {
                    s.city_travel.push_back(bags[item_list[i]].index_node);
                    visit[bags[item_list[i]].index_node] = true;
                    checker = 1;
                }
                if (checker == 1) {
                    s.fast_calculation_adding(bags[item_list[i]].index,bags[item_list[i]].index_node);
                } else {
                    s.checkerSolution(s,false);
                }
                double prob = rng.randDblExc();
                if (s.valid == true && prob < reinforcement_table[i][0]) {
                    choice[i] = 0;
                    continue;
                } else {
                    choice[i] = 1;
                    s.item_taken.pop_back();
                    if (checker == 1) {
                        s.city_travel.pop_back();
                        visit[bags[item_list[i]].index_node] = false;
                    }
                    s.checkerSolution(s,false);
                }
            }

            if (s.profit > best_solution.profit) {
                float score_reward = alpha_reward/(UPPER_BOUND+1-s.profit);
                float score_penalty = belta_penalty/s.profit;
                for (int i = 0; i < n; i++) {
                    if (choice[i] == best_solution_choice[i]) {
                        reinforcement_table[i][choice[i]] = min((float)0.7,score_reward + reinforcement_table[i][choice[i]]);
                        reinforcement_table[i][1-choice[i]] = 1 - reinforcement_table[i][choice[i]];
                    } else {
                        reinforcement_table[i][best_solution_choice[i]] = max((float)0.3,((float)1.0-score_penalty)*reinforcement_table[i][best_solution_choice[i]] - score_reward);
                        reinforcement_table[i][1-best_solution_choice[i]] = 1 - reinforcement_table[i][best_solution_choice[i]];
                    }
                }
                best_solution = s;
                best_solution.checkerSolution(best_solution,false);
                for (int i = 0; i < n; i++) {
                    best_solution_choice[i] = choice[i];
                }
            } else {
                float score_rewawrd = alpha_reward/s.profit;
                float score_penalty = belta_penalty/s.profit;

                for (int i = 0; i < n; i++) {
                    if (choice[i] == best_solution_choice[i]) {
                        reinforcement_table[i][choice[i]] = min((float)0.7,((float)1.0-score_penalty)*reinforcement_table[i][choice[i]] + score_rewawrd);
                        reinforcement_table[i][1-choice[i]] = 1 - reinforcement_table[i][choice[i]];
                    } else {
                        reinforcement_table[i][choice[i]] = max((float)0.3,((float)1.0 - score_penalty)*reinforcement_table[i][choice[i]] - score_rewawrd);
                        reinforcement_table[i][1-choice[i]] = 1 - reinforcement_table[i][choice[i]];
                    }
                }
            }

            // for (int i = 0; i < n; i++) {
            //     cout << reinforcement_table[i][0] << " ";
            // }
            // cout << "\n";
            // for (int i =0; i < n; i++) {
            //     cout << reinforcement_table[i][1] << " ";
            // }
            // cout << "\n";
        }
        // cout << best_solution.profit << endl;
        return best_solution;
    }
};






void update_bestSolution(Solution &b) {
    Solution a;
    Algorithm Algo;
    a = Algo.random_Path_Reinforcement_Heuristic();
    a.adapt_more_item();


    if (a.profit > b.profit || (a.profit == b.profit && a.time_travel < b.time_travel)) {
        b = a;
        b.checkerSolution(b,false);
        TSP_solver.create_new_perm(b.city_travel); 
    } else {
        TSP_solver.create_new_perm(b.city_travel);
        TSP_solver.random_shuffle();
        
    }
}







int RUNTIME;






int main (int argc, char *argv[]) {
    ios_base::sync_with_stdio(0);
    std::cin.tie(0); std::cout.tie(0);

    assert(argc == 4);

    char parameterStr[1000];

    string instanceFile;
    
    sscanf(argv[1],"%s", parameterStr); instanceFile = string(parameterStr);
    int rngSeed; sscanf(argv[2],"%d", &rngSeed);
    sscanf(argv[3],"%d",&RUNTIME);
    rng = MTRand(rngSeed);

    freopen(instanceFile.c_str(), "r", stdin);
    freopen("main.out", "w", stdout);

    cin >> DIMENSION >> ITEMS >> CAPACITY >> MAX_TIME >> v_min >> v_max;
    
    graph.resize(DIMENSION+1);
    bags.resize(ITEMS+1);
    
    for (int i = 1; i <= DIMENSION; i++) {
        cin >> graph[i].index >> graph[i].x >> graph[i].y;  
    }
    for (int i = 1; i <= ITEMS; i++) {
        cin >> bags[i].index >> bags[i].profit >> bags[i].weight >> bags[i].index_node;
        graph[bags[i].index_node].bags.push_back(bags[i]);
    }
    
    dist.assign(DIMENSION+1, vector<double>(DIMENSION+1,0));
    for (int i = 1; i <= DIMENSION; i++) {
        for (int j = 1; j <= DIMENSION; j++) {
            int x = graph[i].x - graph[j].x;
            int y = graph[i].y - graph[j].y;
            dist[i][j] = sqrt(1.0*x*x + 1.0*y*y);
        }
    }

    for (auto x: graph) {
        sort(x.bags.begin(),x.bags.end(),
            [&](Item &a, Item &b){
                return a.profit*b.weight >= b.profit*a.weight;
            }); 
    }

    UPPER_BOUND = 0;
    vector<int> knapsack_solver;
    for (auto x: bags) {    
        knapsack_solver.push_back(x.index);
    }

    sort(knapsack_solver.begin(), knapsack_solver.end(),
        [](int i, int j) {
            return bags[i].profit*bags[j].weight > bags[j].profit*bags[i].weight;
        });
    int weight = 0;
    for (auto x: knapsack_solver) {
        if (weight + bags[x].weight < CAPACITY) {
            UPPER_BOUND += bags[x].profit;
            weight += bags[x].weight;
        }
    }
    // vector<double> prob_item(ITEMS+1,0.5);

    RUNTIME = min(ITEMS/10,300);
    Solution s;
    Algorithm Algo;
    Solution best_solution;
    best_solution.city_travel.push_back(1);
    best_solution.checkerSolution(best_solution,false);
    bool local_optima = false;
    // Solution best_solution;
    // best_solution = s;
    // best_solution.checkerSolution(best_solution,false);
    TSP_solver.construct();

    high_resolution_clock::time_point tStart = high_resolution_clock::now();
    int TRIALS = 1;
    while(true) {

        // for (int i = 1; i <= DIMENSION; i++) {
        //     cnt_city[i] = rng.randExc(1.0*bags_size);
        // }
        // for (int i = 1; i <= DIMENSION; i++) {
        //     cout << cnt_city[i] << " ";
        // }
        // cout << endl;
        tEnd = high_resolution_clock::now();
        exec_time = duration_cast<duration<double>> (tEnd - tStart);
        if (exec_time.count() >= RUNTIME) {
            s = best_solution;
            s.city_travel.push_back(DIMENSION);
            s.checkerSolution(s,false);
            cout << s.profit << endl;
            cout << s.item_taken.size() << endl;
            for (auto x: s.item_taken) {
                cout << x  << " ";
            }
            cout << endl;
            cout << s.city_travel.size() << endl;
            for (auto x: s.city_travel) {
                cout << x << " ";
            }
            // cout << "----------------------------------------------------------" << endl;
            // map_elite.print_Solution();
            exit(0);
        }

        update_bestSolution(best_solution);
        // cout << best_solution.profit << "\n";
        //  cout << s1.profit << endl;
        // cout << s1.profit << endl
        // s = Algo.LargeNeighborhood_Heuristic(s);
        // s.checkerSolution(s,false);
        // cout << s.profit << endl;
        // cout << s1.profit << endl;
    }
    // s.city_travel.push_back(DIMENSION);
    s.checkerSolution(s,false);
    std::cout << s.profit << endl;
    // cout << s.item_taken.size() << endl;
    // for (auto x: s.item_taken) {
    //     cout << x << " bags: " << bags[x].index_node << endl;
    // }
    // cout << endl;
    // cout << s.city_travel.size() << endl;
    // for (auto x: s.city_travel) {
    //     cout << x << " ";
    // }
    return 0;
}