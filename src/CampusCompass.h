#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <regex>

using namespace std;

struct Class {
    string classCode;
    int startMinutes{};
    int endMinutes{};

    Class(string code, const int startMin, const int endMin) :
        classCode(std::move(code)),
        startMinutes(startMin),
        endMinutes(endMin) {}
};

struct Student {
    string name;
    string id;
    string residenceLocationID;
    vector<string> classes;
};

class CampusCompass {
    unordered_map<int, vector<pair<int, int>>> adjList;
    unordered_map<int, vector<Class>> classList;
public:
    CampusCompass();
    static bool isValidStudentID(const string &id);
    static bool isValidName(const string &name);
    static bool isValidClassCode(const string &classCode);
    static bool isValidInteger(const string &id);
    bool isValidLocationID(const string &id);
    static int parseTimeToMinutes(const string& time);
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    static bool ParseCommand(const string &command);

    // TODO not done yet
    bool insert(string name, string id, string locationID, vector<string> classes);
    bool remove(string id);
    bool dropClass(string id, int code);
    bool replaceClass(string id, string class1, string class2);
    bool removeClass(int code);
    bool toggleEdgesClosure(vector<string> locations);
    bool checkEdgeStatus(string loc1, string loc2);
    bool isConnected(string loc1, string loc2);
    void printShortestEdges(int id);
    void printStudentZone(int id);
    void verifySchedule(int ID);

};
