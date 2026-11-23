#include "CampusCompass.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <iomanip>

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

    if (!edgeFile.is_open()) return false;
    if (!classFile.is_open()) return false;

    string line;

    // Remove header
    if (getline(edgeFile, line)) {}
    while (getline(edgeFile, line)) {
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

        adjList[u].emplace_back(v, weight);
        adjList[v].emplace_back(u, weight);
    }

    // Remove header
    if (getline(classFile, line)) {}
    while (getline(classFile, line)) {
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

        classList[locationID].emplace_back(code, startMin, endMin);
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

    if (!(ss >> commandType)) return false;

    auto it = Commands.find(commandType);
    if (it == Commands.end()) return false;

    switch (it->second) {
        case Command::insert: {
            ss >> ws;
            if (ss.peek() != '"') return false;

            string name, studentID, residenceLocationID;
            int n;
            if (!(ss >> quoted(name) >> studentID >> residenceLocationID >> n)) return false;
            if (!isValidName(name) || !isValidStudentID(studentID) || !isValidLocationID(residenceLocationID) || n <= 0 || n > 6) {
                return false;
            }

            vector<string> classes(n);
            for (int i = 0; i < n; ++i) {
                if (!(ss >> classes[i]) || !isValidClassCode(classes[i])) return false;
            }
            if (hasExtraTokens(ss)) return false;

            insert(name, studentID, residenceLocationID, classes);
            return true;
        }

        case Command::removeClass: {
            string classCode;
            if (!(ss >> classCode) || !isValidClassCode(classCode) || hasExtraTokens(ss)) return false;

            removeClass(classCode);
            return true;
        }

        case Command::remove:
        case Command::printShortestEdges:
        case Command::printStudentZone:
        case Command::verifySchedule: {
            string studentID;
            if (!(ss >> studentID) || !isValidStudentID(studentID) || hasExtraTokens(ss)) return false;

            if (it->second == Command::remove)             remove(studentID);
            if (it->second == Command::printShortestEdges) printShortestEdges(studentID);
            if (it->second == Command::printStudentZone)   printStudentZone(studentID);
            if (it->second == Command::verifySchedule)     verifySchedule(studentID);
            return true;
        }

        case Command::dropClass: {
            string studentID, classCode;
            if (!(ss >> studentID >> classCode)) return false;
            if (!isValidStudentID(studentID) || !isValidClassCode(classCode) || hasExtraTokens(ss)) return false;

            dropClass(studentID, classCode);
            return true;
        }

        case Command::replaceClass: {

            string studentID, class1, class2;
            if (!(ss >> studentID >> class1 >> class2)) return false;
            if (!isValidStudentID(studentID) || !isValidClassCode(class1) || !isValidClassCode(class2) || hasExtraTokens(ss)) {
                return false;
            }

            replaceClass(studentID, class1, class2);
            return true;
        }

        case Command::toggleEdgesClosure: {
            int n;
            if (!(ss >> n) || n <= 0) return false;

            vector<string> locationIDs(n);
            for (int i = 0; i < 2 * n; i++) {
                string id;
                if (!(ss >> id) || !isValidLocationID(id)) return false;
                locationIDs.push_back(id);
            }
            if (hasExtraTokens(ss)) return false;

            toggleEdgesClosure(locationIDs);
            return true;
        }

        case Command::checkEdgeStatus: {
            string location1, location2;
            if (!(ss >> location1 >> location2)) return false;
            if (!isValidInteger(location1) || !isValidInteger(location2) || hasExtraTokens(ss)) return false;

            checkEdgeStatus(location1, location2);
            return true;
        }

        case Command::isConnected: {
            string location1, location2;
            if (!(ss >> location1 >> location2)) return false;
            if (!isValidLocationID(location1) || !isValidLocationID(location2) || hasExtraTokens(ss)) return false;

            isConnected(location1, location2);
            return true;
        }
    }
    return false;
}
