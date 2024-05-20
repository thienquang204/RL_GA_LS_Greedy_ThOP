#include <bits/stdc++.h>
#include <chrono>
#include "MTRand.h"
using namespace std;
MTRand rng;


// Dữ liệu mỗi đồ vật
struct Item {
    int index;
    int profit;
    int weight;
    int index_node;
};


// Dữ liệu mỗi đỉnh
struct Point {
    int index;
    double x;
    double y;
    vector<Item> bags;
};

// Sinh hoán vị ngẫu nhiên, Fisher arrangement algorithm
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






// Lời giải

struct Solution {
    float time_travel; // Thời gian di chuyển
    long int w_profit = 0; // Khối lượng
    long int profit = 0; // Lợi nhuận
    vector<int> item_taken; // Danh sách item
    vector<int> city_travel; // Danh sách đỉnh
    bool valid = true; // lời giải đúng => true, ngược lại false

    // Xây dựng item
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
    // Sinh hoán vị ngẫu nhiên
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
    // Fix bug cho tạo item mới
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
    // Tạo item mới
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



    // Trường hợp cần bỏ những đỉnh không có tác dụng
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
    // Tối ưu thời gian bằng LS
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
    // Hàm di chuyển tới đỉnh gần nhất
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
    // Thử sinh hoán vị rồi mới Local Search, để tối ưu thời gian
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
    // Ăn tham theo ưu tiên các item, thử nghiệm thất bại
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

    // ĂN tham ngẫu nhiên các item
    void GRASP_adapt_item() {
        int old_profit = profit;
        vector<bool> taken(ITEMS+1,false);
        for (auto i: item_taken) {
            taken[i] = true;
        }
        vector<bool> visit(DIMENSION+1,false);
        for (auto i: city_travel) {
            visit[i] = true;
        }
        visit[1] = true;

        bool flag = false;


        int L_optimize = 0;
        int cnt_stop = 0;

        while (true) {
            if (flag) break;

            if (L_optimize == 4) {
                if (optimize_time() == false) ILS_optimize_time();
                L_optimize = 0;
            }

            vector<int> possible;
            for (int i = 1; i <= ITEMS; i++) {
                if (taken[i] == false) {
                    possible.push_back(i);
                }
            }

            if (possible.size() == 0) {
                flag = true;
                continue;
            }

            sort(possible.begin(), possible.end(),
                [](int i, int j) {
                    return bags[i].profit*bags[j].weight > bags[j].profit*bags[i].weight;
                });
            int n = possible.size();
            while (possible.size() > n/2) {
                possible.pop_back();
            }
            
            n = city_travel.back();
            sort(possible.begin(), possible.end(),
                [n](int i, int j){
                    int index_1 = bags[i].index_node;
                    int index_2 = bags[j].index_node;
                    return (bags[i].profit)*(bags[j].weight)*dist[n][index_2] > (bags[j].profit)*(bags[i].weight)*dist[n][index_1];   
                });
            n = max((int)possible.size(), 5);
            bool flag_item = false;
            for (int index = 0; index < n; index++) {
                if (1.0*bags[possible[index]].profit < 0.2*bags[possible[index]].weight) continue;
                item_taken.push_back(possible[index]);
                int checker = 0;
                if (visit[bags[possible[index]].index_node] == false) {
                    checker = 1;
                    city_travel.push_back(bags[possible[index]].index_node);
                    visit[bags[possible[index]].index_node] = true;
                }
                if (checker == 1) {
                    fast_calculation_adding(possible[index],bags[possible[index]].index_node);
                } else {
                    checkerSolution((*this),false);
                }
                if (valid == false) {
                    if (checker == 1) {
                        visit[bags[possible[index]].index_node] = false;
                        city_travel.pop_back();
                        item_taken.pop_back();
                    } else {
                        item_taken.pop_back();
                    }
                    checkerSolution((*this),false);
                } else {
                    taken[possible[index]] = true;
                    flag_item = true;
                }
            }
            if (flag_item == false) {
                cnt_stop++;
            } else {
                cnt_stop = 0;
            }

            if (cnt_stop == 8) {
                flag = true;
            }
            L_optimize++;
        }

        // if (profit > old_profit) {
        //     cout << "YES IMPROVE";
        // }
    }
    // Thay đổi đường đi để thử hoán vị thời gian, thoát cực trị địa phương
    void shaking_time() {
        int n = city_travel.size();
        if (n <= 2) return;
        int index = rng.randInt(n-2) + 1;
        int index2 = rng.randInt(n-2) + 1;
        swap(city_travel[index],city_travel[index2]);
        return;
    }



    

    // Kiểm tra lời giải
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
    // TÍnh toán nhanh thi thêm đỉnh mới
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
    // Khởi tạo lúc đầu
    void construct() {
        for (int i = 1; i < DIMENSION; i++) {
            destination.push_back(i);
        }
        return;
    }
    // Tạo lời giải mới dựa trên GA và LS, trong trường hợp có kết quả tốt hơn

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
    // Sinh hoán vị ngẫu nhiên
    void random_shuffle() {
        randomshuffle<int>(destination);
        return;
    }
    // Tạo lời giải TSP mới bằng GA và Random Local Search, khi không có lời giải tối hơn
    void genetic_new_perm(vector<int> &seed_perm) {
        random_shuffle();
        set<int> holder;
        vector<int> perm = destination;
        for (auto i: seed_perm) {
            holder.insert(i);
        }
        int j = 0;
        for (auto i: perm) {
            if (holder.count(i)) {
                i = seed_perm[j];
                j++;
            }
        }
        destination.clear();
        destination = perm;
        
        return;
    }
};

Permutation_solver TSP_solver;



struct Algorithm {
    // Giải thuật chính
    Solution random_Path_Reinforcement_Heuristic() {
        // Nhập danh sách các đỉnh từ lời giải TSP
        vector<int> city;
        for (auto i: TSP_solver.destination) {
            city.push_back(i);
        }
        

        // Nhập danh sách các đồ vật vào vector để xét
        vector<int> item_list;
        for (auto i: city) {
            for (auto x: graph[i].bags) {
                item_list.push_back(x.index);
            }
        }
        

        // Tạo bảng thông số để Q-Learning, penalty cho lựa chọn sai có hằng số là 0.79, lựa chọn đúng là 2.23
        int n = (int)item_list.size();
        vector<vector<float>> reinforcement_table(n,vector<float>(2,0.5));
        float alpha_reward = 2.23;
        float belta_penalty = 0.79;


        Solution best_solution;
        best_solution.city_travel.push_back(1);
        best_solution.checkerSolution(best_solution,false);
        vector<int> best_solution_choice(n,1);

        for (int i = 1; i <= 1000; i++) {

            // Thực hiện lời giải
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
                // Giảm khối lượng tính toán bằng cách tận dụng thuật toán tính nhnah mõi khi có thể
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
            // Dựa vào chất lượng lời giải cải thiện bảng RL
            if (s.profit > best_solution.profit) {
                float score_reward = alpha_reward/(UPPER_BOUND+1-s.profit);
                float score_penalty = belta_penalty/s.profit;
                for (int i = 0; i < n; i++) {
                    if (choice[i] == best_solution_choice[i]) {
                        reinforcement_table[i][choice[i]] = min((float)0.85,score_reward + reinforcement_table[i][choice[i]]);
                        reinforcement_table[i][1-choice[i]] = 1 - reinforcement_table[i][choice[i]];
                    } else {
                        reinforcement_table[i][best_solution_choice[i]] = max((float)0.15,((float)1.0-score_penalty)*reinforcement_table[i][best_solution_choice[i]] - score_reward);
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
                        reinforcement_table[i][choice[i]] = min((float)0.85,((float)1.0-score_penalty)*reinforcement_table[i][choice[i]] + score_rewawrd);
                        reinforcement_table[i][1-choice[i]] = 1 - reinforcement_table[i][choice[i]];
                    } else {
                        reinforcement_table[i][choice[i]] = max((float)0.15,((float)1.0 - score_penalty)*reinforcement_table[i][choice[i]] - score_rewawrd);
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




// Hàm cập nhập

void update_bestSolution(Solution &b) {
    // Tạo một lời giải mới
    Solution a;
    Algorithm Algo;

    // Dùng thuật giải RL_GA_LS để giải
    a = Algo.random_Path_Reinforcement_Heuristic();

    // a.adapt_more_item();
    // Dùng Random Greedy để cải thiện lời giải
    a.GRASP_adapt_item();

    // Nếu kết quả tốt hơn thì tạo hoán vị mới bằng GA, còn không thì tối ưu thời gian rồi dùng GA
    if (a.profit > b.profit || (a.profit == b.profit && a.time_travel < b.time_travel)) {
        b = a;
        b.checkerSolution(b,false);
        TSP_solver.create_new_perm(b.city_travel); 
    } else {
        b.optimize_time();
        TSP_solver.create_new_perm(b.city_travel);
    }
    // cout << a.profit << endl;
}







int RUNTIME;






int main (int argc, char *argv[]) {
    // Nhập dữ liệu
    ios_base::sync_with_stdio(0);
    std::cin.tie(0); std::cout.tie(0);

    assert(argc == 4);

    char parameterStr[1000];

    string instanceFile;
    

    // 3 thông số: tên file, seed random, thời gian chạy file tính bằng s
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

    // TÍnh khoảng cách giữa các đỉnh
    
    dist.assign(DIMENSION+1, vector<double>(DIMENSION+1,0));
    for (int i = 1; i <= DIMENSION; i++) {
        for (int j = 1; j <= DIMENSION; j++) {
            int x = graph[i].x - graph[j].x;
            int y = graph[i].y - graph[j].y;
            dist[i][j] = sqrt(1.0*x*x + 1.0*y*y);
        }
    }

    // Sort độ ưu tiên của đồ vật tại từng đỉnh

    for (auto x: graph) {
        sort(x.bags.begin(),x.bags.end(),
            [&](Item &a, Item &b){
                return a.profit*b.weight >= b.profit*a.weight;
            }); 
    }


    // Tìm cận trên để update thông số trong thuật toán RL, dùng thuật ăn tham để tìm cận trong O(nlogn)
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



    // Thời gian chạy nếu muốn thay đổi thì chỉnh lại seed này
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
    // Khời tạo lời giải TSP,sinh hoán vị ngẫu nhiên và tối ưu đường đi
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
        // Tính thời gian
        tEnd = high_resolution_clock::now();
        exec_time = duration_cast<duration<double>> (tEnd - tStart);
        if (exec_time.count() >= RUNTIME) {
            // In kết quả sau khi ngừng thời gian chạy
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
        // Tìm lời giải mới và cập nhập lời giải tối ưu
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