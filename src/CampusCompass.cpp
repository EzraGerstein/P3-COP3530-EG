#include "CampusCompass.h"

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <regex>
#include <sstream>
#include <unordered_set>


const regex validID("^[0-9]{8}$");
const regex validName("^(?=.*[a-zA-Z])[a-zA-Z ]+$");
const regex validClassCode("[A-Z]{3}[0-9]{4}");

// enum of accepted commands
enum class Command {
    insert, remove, dropClass, replaceClass, removeClass,
    toggleEdgesClosure, checkEdgeStatus, isConnected,
    printShortestEdges, printStudentZone, verifySchedule
};

static const unordered_map<string, Command> Commands = {
    {"insert",             Command::insert},
    {"remove",             Command::remove},
    {"dropClass",          Command::dropClass},
    {"replaceClass",       Command::replaceClass},
    {"removeClass",        Command::removeClass},
    {"toggleEdgesClosure", Command::toggleEdgesClosure},
    {"checkEdgeStatus",    Command::checkEdgeStatus},
    {"isConnected",        Command::isConnected},
    {"printShortestEdges", Command::printShortestEdges},
    {"printStudentZone",   Command::printStudentZone},
    {"verifySchedule",     Command::verifySchedule}
};

CampusCompass::CampusCompass() = default;

// Uses regex to check for a valid ID
bool CampusCompass::isValidStudentID(const string &id) {
    return regex_match(id, validID);
}

// Uses regex to check for a valid name
bool CampusCompass::isValidName(const string &name) {
    return regex_match(name, validName);
}

// Uses regex to check for a valid class code
bool CampusCompass::isValidClassCode(const string &classCode) {
    return regex_match(classCode, validClassCode);
}

// Helper function of isValidLocationID
bool CampusCompass::isValidInteger(const string &id) {
    if (id.empty() || !isdigit(static_cast<unsigned char>(id[0]))) return false;
    try {
        size_t position = 0;
        stoi(id, &position);
        return position == id.size();
    } catch (const exception&) {
        return false;
    }
}

// Checks if passed location is valid
bool CampusCompass::isValidLocationID(const string &id) {
    if (!isValidInteger(id)) return false;
    return adjList.find(stoi(id)) != adjList.end();
}

// Helper function of ParseCSV
int CampusCompass::parseTimeToMinutes(const string& time) {
    const size_t colonPosition = time.find(':');
    if (colonPosition == string::npos) return 0;

    const int hours = stoi(time.substr(0, colonPosition));
    const int minutes = stoi(time.substr(colonPosition + 1));
    return hours * 60 + minutes;
}

// Parses edges and classes csv
bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    ifstream edgeFile(edges_filepath);
    ifstream classFile(classes_filepath);

    if (!edgeFile.is_open() || !classFile.is_open()) return false;

    string line;

    // Remove header
    if (getline(edgeFile, line)) {}
    while (getline(edgeFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '/') continue;

        stringstream ssEdge(line);
        string location1, location2, name1, name2, time;

        getline(ssEdge, location1, ',');
        getline(ssEdge, location2, ',');
        getline(ssEdge, name1, ',');
        getline(ssEdge, name2, ',');
        getline(ssEdge, time);

        int u = stoi(location1);
        int v = stoi(location2);
        int weight = stoi(time);

        adjList[u].push_back({v, weight, 1});
        adjList[v].push_back({u, weight, 1});
    }

    // Remove header
    if (getline(classFile, line)) {}
    while (getline(classFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '/') continue;

        stringstream ssClass(line);
        string code, location, startTime, endTime;

        getline(ssClass, code, ',');
        getline(ssClass, location, ',');
        getline(ssClass, startTime, ',');
        getline(ssClass, endTime);

        int locationID= stoi(location);
        int startMin = parseTimeToMinutes(startTime);
        int endMin = parseTimeToMinutes(endTime);

        if (adjList.find(locationID) == adjList.end()) {
            adjList[locationID] = {};
        }

        classList[code].emplace_back(code, locationID, startMin, endMin);
    }
    return true;
}

bool hasExtraTokens(stringstream& ss) {
    string extra;
    return static_cast<bool>(ss >> extra);
}

bool CampusCompass::ParseCommand(const string &command) {
    stringstream ss(command);
    string commandType;

    if (!(ss >> commandType)) {
        cout << "unsuccessful\n";
        return false;
    }

    auto it = Commands.find(commandType);
    if (it == Commands.end()) {
        cout << "unsuccessful\n";
        return false;
    }

    switch (it->second) {
        case Command::insert: {
            ss >> ws;
            if (ss.peek() != '"') {
                cout << "unsuccessful\n";
                return false;
            }

            string name, studentID, residenceLocationID;
            int n;
            if (!(ss >> quoted(name) >> studentID >> residenceLocationID >> n)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (!isValidName(name) || !isValidStudentID(studentID) || !isValidLocationID(residenceLocationID) || n <= 0 || n > 6) {
                cout << "unsuccessful\n";
                return false;
            }

            vector<string> classes(n);
            for (int i = 0; i < n; ++i) {
                if (!(ss >> classes[i]) || !isValidClassCode(classes[i])) {
                    cout << "unsuccessful\n";
                    return false;
                }
            }
            if (hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }

            if (insert(name, stoi(studentID), stoi(residenceLocationID), classes)) {
                cout << "successful\n";
            } else {
                cout << "unsuccessful\n";
            }

            return true;
        }
        case Command::remove: {
            string studentID;
            if (!(ss >> studentID) || !isValidStudentID(studentID) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (remove(stoi(studentID))) {
                cout << "successful\n";
            } else {
                cout << "unsuccessful\n";
            }
            return true;
        }

        case Command::dropClass: {
            string studentID, classCode;
            if (!(ss >> studentID >> classCode) || !isValidStudentID(studentID) || !isValidClassCode(classCode) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (dropClass(stoi(studentID), classCode)) {
                cout << "successful\n";
            } else {
                cout << "unsuccessful\n";
            }
            return true;
        }

        case Command::replaceClass: {
            string studentID, class1, class2;
            if (!(ss >> studentID >> class1 >> class2) || !isValidStudentID(studentID) || !isValidClassCode(class1) || !isValidClassCode(class2) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (replaceClass(stoi(studentID), class1, class2)) {
                cout << "successful\n";
            } else {
                cout << "unsuccessful\n";
            }
            return true;
        }

        case Command::removeClass: {
            string classCode;
            if (!(ss >> classCode) || !isValidClassCode(classCode) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (auto [success, count] = removeClass(classCode); success) {
                cout << count << "\n";
            } else {
                cout << "unsuccessful\n";
            }
            return true;
        }

        case Command::toggleEdgesClosure: {
            int n;
            if (!(ss >> n) || n <= 0) {
                cout << "unsuccessful\n";
                return false;
            }

            vector<int> locationIDs;
            locationIDs.reserve(2 * n);
            for (int i = 0; i < 2 * n; ++i) {
                string id;
                if (!(ss >> id) || !isValidLocationID(id)) {
                    cout << "unsuccessful\n";
                    return false;
                }
                locationIDs.push_back(stoi(id));
            }

            if (hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }

            toggleEdgesClosure(locationIDs);
            cout << "successful\n";
            return true;
        }

        case Command::checkEdgeStatus: {
            string location1, location2;
            if (!(ss >> location1 >> location2) || !isValidInteger(location1) || !isValidInteger(location2) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (int status = checkEdgeStatus(stoi(location1), stoi(location2)); status == 1) cout << "open\n";
            else if (status == 0) cout << "closed\n";
            else cout << "DNE\n";
            return true;
        }

        case Command::isConnected: {
            string location1, location2;
            if (!(ss >> location1 >> location2) || !isValidLocationID(location1) || !isValidLocationID(location2) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            if (isConnected(stoi(location1), stoi(location2))) {
                cout << "successful\n";
            } else {
                cout << "unsuccessful\n";
            }
            return true;
        }

        case Command::printShortestEdges: {
            string studentID;
            if (!(ss >> studentID) || !isValidStudentID(studentID) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            int id = stoi(studentID);
            if (students.find(id) == students.end()) {
                cout << "unsuccessful\n";
                return false;
            }
            printShortestEdges(id);
            return true;
        }
        case Command::printStudentZone: {
            string studentID;
            if (!(ss >> studentID) || !isValidStudentID(studentID) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            int id = stoi(studentID);
            if (students.find(id) == students.end()) {
                cout << "unsuccessful\n";
                return false;
            }
            int cost = printStudentZone(id);
            cout << "Student Zone Cost For " << students[id].studentName << ": " << cost << "\n";
            return true;
        }

        case Command::verifySchedule: {
            string studentID;
            if (!(ss >> studentID) || !isValidStudentID(studentID) || hasExtraTokens(ss)) {
                cout << "unsuccessful\n";
                return false;
            }
            int id = stoi(studentID);
            if (students.find(id) == students.end() || students[id].studentClasses.size() <= 1) {
                cout << "unsuccessful\n";
                return false;
            }
            auto res = verifySchedule(id);
            cout << "Schedule Check for " << students[id].studentName << ":\n";
            for (const auto& [pairName, feasible] : res) {
                cout << pairName << ": " << (feasible ? "successful" : "unsuccessful") << "\n";
            }
            return true;
        }

        default:
            break;
    }
    cout << "unsuccessful\n";
    return false;
}


bool CampusCompass::insert(const string& name, const int studentID, const int residenceLocationID, const vector<string>& classNames) {
    if (students.find(studentID) != students.end()) return false;

    // Duplicate check
    unordered_set<string> seen;
    for (const string& code : classNames) {
        if (!seen.insert(code).second) return false;
    }

    vector<Class> classes;
    classes.reserve(classNames.size());
    for (const string& code : classNames) {
        auto it = classList.find(code);
        if (it == classList.end()) return false;
        classes.push_back(it->second[0]);
    }
    Student student(name, studentID, residenceLocationID, classes);
    students.emplace(studentID, student);
    return true;
}

bool CampusCompass::remove(const int studentID) {
    return students.erase(studentID);
}

bool CampusCompass::dropClass(const int studentID, const string& code) {
    if (students.find(studentID) == students.end()) return false;
    if (classList.find(code) == classList.end()) return false;

    vector<Class>& classes = students[studentID].studentClasses;

    for (int i = 0; i < classes.size(); i++) {
        if (classes[i].classCode == code) {
            classes.erase(classes.begin() + i);
            if (classes.empty()) remove(studentID);
            return true;
        }
    }
    return false;
}

bool CampusCompass::replaceClass(const int studentID, const string& code1, const string& code2) {
    if (students.find(studentID) == students.end()) return false;
    if (classList.find(code2) == classList.end()) return false;

    vector<Class>& classes = students[studentID].studentClasses;

    for (const auto& c : classes) {
        if (c.classCode == code2) return false;
    }

    for (auto& classToReplace : classes) {
        if (classToReplace.classCode == code1) {
            classToReplace = classList[code2][0];
            return true;
        }
    }
    return false;
}

pair<bool, int> CampusCompass::removeClass(const string& code) {
    if (classList.find(code) == classList.end()) return {false, 0};

    vector<int> studentIDs;
    studentIDs.reserve(students.size());
    for (const auto& [studentID, student] : students) {
        studentIDs.push_back(studentID);
    }

    int count = 0;
    for (const int studentID : studentIDs) {
        if (dropClass(studentID, code)) {
            count++;
        }
    }
    return {count > 0, count};
}

bool CampusCompass::toggleEdgesClosure(const vector<int>& locations) {
    for (int i = 0; i + 1 < locations.size(); i += 2) {
        int u = locations[i];
        int v = locations[i + 1];

        for (auto& edge : adjList[u]) {
            if (edge[0] == v) {
                edge[2] = !edge[2];
                break;
            }
        }
        for (auto& edge : adjList[v]) {
            if (edge[0] == u) {
                edge[2] = !edge[2];
                break;
            }
        }
    }
    return true;
}

int CampusCompass::checkEdgeStatus(const int location1, const int location2) {
    const auto it = adjList.find(location1);
    if (it == adjList.end()) {
        return 2;
    }

    for (const auto& [neighbor, weight, isOpen] : it->second) {
        if (neighbor == location2) {
            return isOpen == 0 ? 0 : 1;
        }
    }
    return 2;
}

bool CampusCompass::isConnected(const int location1, const int location2) {
    if (location1 == location2) return true;

    queue<int> frontier;
    unordered_set<int> visited;

    visited.insert(location1);
    frontier.push(location1);

    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();

        auto it = adjList.find(current);
        if (it == adjList.end()) continue;

        for (const auto& [neighbor, weight, isOpen] : it->second) {
            if (!isOpen) continue;
            if (neighbor == location2) return true;

            if (!visited.count(neighbor)) {
                visited.insert(neighbor);
                frontier.push(neighbor);
            }
        }
    }
    return false;
}

pair<unordered_map<int, int>, unordered_map<int, int>> CampusCompass::runDijkstra(int startLocationID) {
    unordered_map<int, int> distance;
    unordered_map<int, int> parent;
    priority_queue<pair<int, int>,
                   vector<pair<int, int>>,
                   greater<>> minHeap;

    distance[startLocationID] = 0;
    minHeap.emplace(0, startLocationID);

    while (!minHeap.empty()) {
        auto [currentDistance, u] = minHeap.top();
        minHeap.pop();

        if (currentDistance > distance[u]) continue;

        auto it = adjList.find(u);
        if (it == adjList.end()) continue;

        for (const auto& [v, weight, isOpenEdge] : it->second) {
            if (!isOpenEdge) continue;

            if (int newDistance = currentDistance + weight; !distance.count(v) || newDistance < distance[v]) {
                distance[v] = newDistance;
                parent[v] = u;
                minHeap.emplace(newDistance, v);
            }
        }
    }
    return {distance, parent};
}

vector<int> CampusCompass::printShortestEdges(const int studentID) {
    const auto& student = students[studentID];
    auto [distance, parent] = runDijkstra(student.residenceLocationID);

    vector<Class> sortedClasses = student.studentClasses;
    sort(sortedClasses.begin(), sortedClasses.end(),
         [](const Class& a, const Class& b) {
             return a.classCode < b.classCode;
         });
    cout << "Time For Shortest Edges: " << student.studentName << "\n";

    vector<int> shortestTimes;
    shortestTimes.reserve(sortedClasses.size());

    for (const Class& c : sortedClasses) {
        int time = distance.count(c.locationID) ? distance[c.locationID] : -1;
        cout << c.classCode << ": " << time << "\n";
        shortestTimes.push_back(time);
    }
    return shortestTimes;
}

int CampusCompass::printStudentZone(const int studentID) {
    const auto& student = students[studentID];
    int residenceLocationID = student.residenceLocationID;
    auto [distance, parent] = runDijkstra(residenceLocationID);

    unordered_set<int> zoneVertices;
    zoneVertices.insert(residenceLocationID);

    for (const Class& c : student.studentClasses) {
        if (distance.count(c.locationID) && distance[c.locationID] >= 0) {
            int current = c.locationID;
            while (current != residenceLocationID && parent.count(current)) {
                if (!zoneVertices.insert(current).second) {
                    break;
                }
                current = parent[current];
            }
        }
    }

    unordered_set<int> inMST;
    priority_queue<pair<int, int>,
                   vector<pair<int, int>>,
                   greater<>> mstHeap;

    mstHeap.emplace(0, residenceLocationID);
    int totalCost = 0;

    while (!mstHeap.empty()) {
        auto [w, u] = mstHeap.top();
        mstHeap.pop();

        if (inMST.count(u)) continue;
        inMST.insert(u);
        totalCost += w;

        auto it = adjList.find(u);
        if (it == adjList.end()) continue;

        for (const auto& [v, weight, isOpenEdge] : it->second) {
            if (isOpenEdge && zoneVertices.count(v) && !inMST.count(v)) {
                mstHeap.emplace(weight, v);
            }
        }
    }
    return totalCost;
}

vector<pair<string, bool>> CampusCompass::verifySchedule(const int studentID) {
    const auto& student = students[studentID];
    vector<Class> sortedClasses = student.studentClasses;

    if (sortedClasses.size() <= 1) {
        return {};
    }

    sort(sortedClasses.begin(), sortedClasses.end(),
         [](const Class& a, const Class& b) {
             if (a.startMinutes != b.startMinutes) {
                 return a.startMinutes < b.startMinutes;
             }
             if (a.endMinutes != b.endMinutes) {
                 return a.endMinutes < b.endMinutes;
             }
             return a.classCode < b.classCode;
         });

    vector<pair<string, bool>> results;
    results.reserve(sortedClasses.size() - 1);

    for (size_t i = 0; i + 1 < sortedClasses.size(); ++i) {
        const Class& current = sortedClasses[i];
        const Class& next = sortedClasses[i + 1];

        auto [distance, parent] = runDijkstra(current.locationID);
        const int timeGap = next.startMinutes - current.endMinutes;

        bool feasible = distance.count(next.locationID) && (timeGap >= distance[next.locationID]);
        results.emplace_back(current.classCode + " - " + next.classCode, feasible);
    }

    return results;
}
