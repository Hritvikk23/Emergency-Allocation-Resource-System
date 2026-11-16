#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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

int main() {
    DataBuilder data;

    // Loading data test
    data.loadAll("../data/test0/", "sample0");
    data.printAll();
    data.clearAll(); cout << "Clearing data..." << endl;
    data.printAll();
    return 0;
}