#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <array>

using namespace std;

struct Class {
    string classCode;
    int locationID{};
    int startMinutes{};
    int endMinutes{};

    Class(string code, const int locationID, const int startMinutes, const int endMinutes) :
        classCode(std::move(code)),
        locationID(locationID),
        startMinutes(startMinutes),
        endMinutes(endMinutes) {}
};

struct Student {
    string studentName;
    int studentID{};
    int residenceLocationID{};
    vector<Class> studentClasses;

    Student() = default;

    Student(string name, const int id, const int residenceLocationID, const vector<Class> &classes) :
        studentName(std::move(name)),
        studentID(id),
        residenceLocationID(residenceLocationID),
        studentClasses(classes) {}
};

class CampusCompass {
    unordered_map<int, Student> students;
    unordered_map<int, vector<array<int, 3>>> adjList;
    unordered_map<string, vector<Class>> classList;
public:
    CampusCompass();
    static bool isValidStudentID(const string &id);
    static bool isValidName(const string &name);
    static bool isValidClassCode(const string &classCode);
    static bool isValidInteger(const string &id);
    bool isValidLocationID(const string &id);
    static int parseTimeToMinutes(const string& time);
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
    bool insert(const string& name, int studentID, int residenceLocationID, const vector<string>& classNames);
    bool remove(int studentID);
    bool dropClass(int studentID, const string& code);
    bool replaceClass(int studentID, const string& code1, const string& code2);
    pair<bool, int> removeClass(const string& code);
    bool toggleEdgesClosure(const vector<int>& locations);
    int checkEdgeStatus(int location1, int location2);
    bool isConnected(int location1, int location2);

    pair<unordered_map<int, int>, unordered_map<int, int>> runDijkstra(int startLocationID);
    vector<int> printShortestEdges(int studentID);
    int printStudentZone(int studentID);
    vector<pair<string, bool>> verifySchedule(int studentID);

};
