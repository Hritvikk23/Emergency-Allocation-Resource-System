#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
using namespace std;
 
class Disaster {
    public:
        string id;
        string type;
        string location;
        int severity;
        int population;
        int food, water, medical, shelter;
        int deadline;

        Disaster(vector<string> records) {
            id         = records[0];
            type       = records[1];
            location   = records[2];
            severity   = stoi(records[3]);
            population = stoi(records[4]);
            food       = stoi(records[5]);
            water      = stoi(records[6]);
            medical    = stoi(records[7]);
            shelter    = stoi(records[8]);
            deadline   = stoi(records[9]);
        }
};

class Depot {
    public:
        string id;
        string name;
        int food, water, medical, shelter;

        Depot(vector<string> records) {
            id      = records[0];
            name    = records[1];
            food    = stoi(records[2]);
            water   = stoi(records[3]);
            medical = stoi(records[4]);
            shelter = stoi(records[5]);
        }
};

class Route {
    public:
        string from; // depot id
        string to;   // disaster id
        double distance;
        double travelTime;
        int capacity;

        Route(vector<string> records) {
            from       = records[0];
            to         = records[1];
            distance   = stod(records[2]);
            travelTime = stod(records[3]);
            capacity   = stoi(records[4]);
        }
};

class DataBuilder {
    private:
        vector<Disaster> disasters;
        vector<Depot> depots;
        vector<Route> routes;

        vector<string> parseCSVLine(const string& line) {
            vector<string> cells;
            string currentCell = "";
            bool insideQuotes = false;

            for (size_t i = 0; i < line.length(); ++i) {
                char ch = line[i];

                if (ch == '"') {
                    if (insideQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                        currentCell += '"';
                        i++;
                    } else {
                        insideQuotes = !insideQuotes;
                    }
                }
                else if (ch == ',' && !insideQuotes) {
                    cells.push_back(currentCell);
                    currentCell = "";
                }
                else if (ch != '\r') {
                    currentCell += ch;
                }
            }

            cells.push_back(currentCell);
            return cells;
        }

        bool readRows(const string& fileName, size_t minCols, vector<vector<string>>& rows) {
            ifstream file(fileName);
            if (!file.is_open()) {
                cerr << "Error: could not open " << fileName << endl;
                return false;
            }
            rows.clear();

            string line;
            bool header = true;
            while (getline(file, line)) {
                if (header) { header = false; continue; }
                if (line.empty()) continue;
                vector<string> cells = parseCSVLine(line);
                if (cells.size() >= minCols) rows.push_back(cells);
            }
            return true;
        }

    public:
        const vector<Disaster>& getDisasters() const { return disasters; }
        const vector<Depot>& getDepots() const { return depots; }
        const vector<Route>& getRoutes() const { return routes; }

        void clearAll() {
            disasters.clear();
            depots.clear();
            routes.clear();
        }

        bool loadDisasters(const string& fileName) {
            vector<vector<string>> rows;
            if (!readRows(fileName, 10, rows)) return false;
            disasters.clear();
            for (size_t i = 0; i < rows.size(); i++) {
                try {disasters.push_back(Disaster(rows[i]));} catch (...) {}
            }
            return true;
        }

        bool loadSupplies(const string& fileName) {
            vector<vector<string>> rows;
            if (!readRows(fileName, 6, rows)) return false;
            depots.clear();
            for (size_t i = 0; i < rows.size(); i++) {
                try {depots.push_back(Depot(rows[i]));} catch (...) {}
            }
            return true;
        }

        bool loadRoutes(const string& fileName) {
            vector<vector<string>> rows;
            if (!readRows(fileName, 5, rows)) return false;
            routes.clear();
            for (size_t i = 0; i < rows.size(); i++) {
                try {routes.push_back(Route(rows[i]));} catch (...) {}
            }
            return true;
        }

        bool loadAll(const string& dir, const string& tag) {
            return loadDisasters(dir + "disasters_" + tag + ".csv") && loadSupplies(dir + "supplies_" + tag + ".csv") && loadRoutes(dir + "routes_" + tag + ".csv");
        }

        void printAll() const {
            cout << "Depots (" << depots.size() << "):" << endl;
            for (size_t i = 0; i < depots.size(); i++) {
                const Depot& d = depots[i];
                cout << "  " << d.id << ", " << d.name << ", food " << d.food << ", water " << d.water
                     << ", medical " << d.medical << ", shelter " << d.shelter << endl;
            }
 
            cout << "Disasters (" << disasters.size() << "):" << endl;
            for (size_t i = 0; i < disasters.size(); i++) {
                const Disaster& e = disasters[i];
                cout << "  " << e.id << ", " << e.type << ", " << e.location << ", severity " << e.severity
                     << ", population " << e.population << ", food " << e.food << ", water " << e.water
                     << ", medical " << e.medical << ", shelter " << e.shelter
                     << ", deadline " << e.deadline << endl;
            }
 
            cout << "Routes (" << routes.size() << "):" << endl;
            for (size_t i = 0; i < routes.size(); i++) {
                const Route& r = routes[i];
                cout << "  " << r.from << " -> " << r.to << ", distance " << r.distance
                     << ", travel time " << r.travelTime << ", capacity " << r.capacity << endl;
            }
        }
};

// ---------- greedy allocation ----------
const char* RESOURCES_N[4] = {"food", "water", "medical", "shelter"};
const int SHIP_ORDER[4] = {2, 1, 0, 3};

class Allocation {
    public:
        string depotId;
        string disasterId;
        int amount[4];
};

class GreedyAllocator {
    private:
        const DataBuilder& data;
        vector<vector<int>> stock, demand;
        vector<int> capacityLeft, routeDepot, routeDisaster;
        vector<Allocation> result;

        double urgency(const Disaster& dis) const {
            double hours = dis.deadline > 0 ? dis.deadline : 1;
            return dis.severity * (double)dis.population / hours;
        }

        int computeShipment(int d, int e, int cap, int out[4]) const {
            int remaining = cap, total = 0;
            for (int i = 0; i < 4; i++) out[i] = 0;
            for (int i = 0; i < 4; i++) {
                int k = SHIP_ORDER[i];
                int take = min(min(stock[d][k], demand[e][k]), remaining);
                out[k] = take;
                remaining -= take;
                total += take;
            }
            return total;
        }
    public:
        GreedyAllocator(const DataBuilder& builder) : data(builder) {}

        void run() {
            const vector<Depot>& depots = data.getDepots();
            const vector<Disaster>& disasters = data.getDisasters();
            const vector<Route>& routes = data.getRoutes();
            capacityLeft.clear(); routeDepot.clear(); routeDisaster.clear(); result.clear();
            unordered_map<string, int> depotIndex, disasterIndex;
            
            stock.assign(depots.size(), vector<int>(4, 0));
            for (size_t i = 0; i < depots.size(); i++) {
                stock[i][0] = depots[i].food;
                stock[i][1] = depots[i].water;
                stock[i][2] = depots[i].medical;
                stock[i][3] = depots[i].shelter;
                depotIndex[depots[i].id] = (int)i;
            }
            demand.assign(disasters.size(), vector<int>(4, 0));
            for (size_t i = 0; i < disasters.size(); i++) {
                demand[i][0] = disasters[i].food;
                demand[i][1] = disasters[i].water;
                demand[i][2] = disasters[i].medical;
                demand[i][3] = disasters[i].shelter;
                disasterIndex[disasters[i].id] = (int)i;
            }
            for (size_t r = 0; r < routes.size(); r++) {
                capacityLeft.push_back(routes[r].capacity);
                routeDepot.push_back(depotIndex.count(routes[r].from) ? depotIndex[routes[r].from] : -1);
                routeDisaster.push_back(disasterIndex.count(routes[r].to) ? disasterIndex[routes[r].to] : -1);
            }

            while (true) {
                int best = -1;
                double bestScore = -1;
                int out[4];

                for (size_t r = 0; r < routes.size(); r++) {
                    int d = routeDepot[r], e = routeDisaster[r];
                    if (d < 0 || e < 0) continue;
                    if (routes[r].travelTime / 60 > disasters[e].deadline) continue;
                    if (computeShipment(d, e, capacityLeft[r], out) <= 0) continue;

                    double score = urgency(disasters[e]) / (routes[r].travelTime + 1.0);
                    if (score > bestScore) {
                        bestScore = score;
                        best = (int)r;
                    }
                }
                if (best < 0) break;

                int d = routeDepot[best], e = routeDisaster[best];
                int total = computeShipment(d, e, capacityLeft[best], out);
                Allocation a;
                a.depotId = depots[d].id;
                a.disasterId = disasters[e].id;
                for (int k = 0; k < 4; k++) {
                    a.amount[k] = out[k];
                    stock[d][k] -= out[k];
                    demand[e][k] -= out[k];
                }
                capacityLeft[best] -= total;
                result.push_back(a);
            }
        }

        const vector<Allocation>& getResult() const { return result; }

        void printResults() const {
            const vector<Disaster>& disasters = data.getDisasters();
 
            cout << "Shipments (" << result.size() << "):" << endl;
            for (size_t i = 0; i < result.size(); i++) {
                cout << "  " << result[i].depotId << " -> " << result[i].disasterId << ":";
                for (int k = 0; k < 4; k++) cout << " " << RESOURCES_N[k] << " - " << result[i].amount[k] << ";";
                cout << endl;
            }

            cout << "Unmet demand:" << endl;
            long long unmetTotal = 0;
            for (size_t e = 0; e < disasters.size(); e++) {
                long long unmet_check = 0;
                for (int k = 0; k < 4; k++) unmet_check += demand[e][k];
                if (unmet_check != 0) {
                    long long unmet = 0;
                    cout << "  " << disasters[e].id << ":";
                    for (int k = 0; k < 4; k++) {
                        cout << " " << RESOURCES_N[k] << " " << demand[e][k];
                        unmet += demand[e][k];
                    }
                    cout << endl;
                    unmetTotal += unmet;
                }
            }
            cout << "Total unmet units: " << unmetTotal << endl;
        }
};

void baseTests(DataBuilder& data, int i) {
    GreedyAllocator greedy(data);
    vector<string> cases = {"normal", "scarcity", "conflict"};
    switch (i)
    {
    case 0:
        // Test 0
        cout << "Base data load and greedy allocation test:" << endl;
        data.loadAll("../data/test0/", "sample0");
        data.printAll();

        cout << endl << "== Greedy allocation ==" << endl;
        greedy.run();
        greedy.printResults();

        data.clearAll(); cout << "Data cleared" << endl;
        break;
    case 1:
        // Test 1
        cout << "Test cases for greedy allocation:";
        for (string c : cases) {
            cout << endl;
            cout << "== Test case: " << c << " ==" << endl;
            data.loadAll("../tests/" + c + "/", c);
            greedy.run();
            greedy.printResults();    
            data.clearAll();
        }
        break;
    default:
        cout << "Invalid selection." << endl;
        break;
    }
};

int main() {
    DataBuilder data;
    int nTest;
    cout << "Test type (0: Base data load; 1: Test cases): ";
    cin >> nTest;

    baseTests(data, nTest);
    return 0;
}