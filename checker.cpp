#include <bits/stdc++.h>
using namespace std;


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





int DIMENSION, ITEMS; // Số đỉnh trong đồ thị, số items;
int CAPACITY; // Khối lượn tối đa của túi
float MAX_TIME; // Thời gian tối đa
double v_min, v_max; // Tốc độ nhỏ nhất, tốc độ lớn nhất
vector<vector<double>> dist;
vector<Point> graph; // Đồ thị
vector<Item> bags; // Túi đồ

int UPPER_BOUND;

int main (int argc, char *argv[]) {
    ios_base::sync_with_stdio(0);
    cin.tie(0); cout.tie(0);

    assert(argc == 2);

    char parameterStr[1000];

    string instanceFile;
    
    sscanf(argv[1],"%s", parameterStr); instanceFile = string(parameterStr);

    freopen(instanceFile.c_str(), "r", stdin);
    // freopen("main.out", "w", stdout);

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
    for (auto x: bags) {
        UPPER_BOUND += x.profit;
    }

    int n_items;
    cin >> n_items;
    vector<int> items_taken(n_items);
    for (int i = 0; i < n_items; i++) {
        cin >> items_taken[i];
    }
    int n_city;
    cin >> n_city;
    vector<int> city_travels(n_city); 
    for (int i = 0; i < n_city; i++) {
        cin >> city_travels[i];
    }
    vector<int> weight(DIMENSION+1,0);
    for (auto i: items_taken) {
        weight[bags[i].index_node] += bags[i].weight;
    }
    vector<int> cnt(ITEMS+1,0);
    bool non_taken_again = true;
    for (auto i: items_taken) {
        cnt[i]++;
    }
    for (int i = 1; i <= ITEMS; i++) {
        if (cnt[i] > 1) {
            non_taken_again = false;
            break;
        }
    }
    double v = v_max;
    int w = 0;
    float t = 0.0;
    for (int i = 1; i < n_city; i++) {
        t += 1.0*dist[city_travels[i]][city_travels[i-1]]/v;
        w = w + weight[city_travels[i]];
        cout << "Weight increaed: " << weight[city_travels[i]] << endl;
        v = v_max - 1.0*w*(v_max-v_min)/CAPACITY;
        cout << "Time: "<< setprecision(10) << t << endl;
        cout << "Weight:  " << w << endl;
        cout << "Speed: " << v << endl; 
        cout << "-----------------" << endl;
    }
    long int profit = 0;
    for (auto i: items_taken) {
        profit += bags[i].profit;
    }
    cout << profit << endl;
    cout << w << endl;
    if (t <= MAX_TIME && non_taken_again == true) cout << "TRUE SOLUTION" << endl;

    return 0;
}