#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include "CampusCompass.h"

using namespace std;

TEST_CASE("Command parser rejections", "[parser]") {
    CampusCompass compass;
    compass.ParseCSV("data/edges.csv", "data/classes.csv");

    // Number in name
    REQUIRE_FALSE(compass.ParseCommand("insert \"A11y Brier\" 12345678 1 1 COP3530"));

    // Seven digit student ID
    REQUIRE_FALSE(compass.ParseCommand("insert \"Ally Brier\" 1234567 1 1 COP3530"));

    // Lowercase class code
    REQUIRE_FALSE(compass.ParseCommand("insert \"Ally Brier\" 12345678 1 1 cop3530"));

    // No existing student
    REQUIRE_FALSE(compass.ParseCommand("deleteStudent 12345678"));

    // Extra parameter
    REQUIRE_FALSE(compass.ParseCommand("remove 12345678 extraParameter"));

    // More than six classes
    REQUIRE_FALSE(compass.ParseCommand("insert \"Ally Brier\" 12345678 1 7 CDA3101 CEN3031 CIS4301 CIS4360 COP3530 COT3100 MAC2311"));
}

TEST_CASE("Edge cases and boundary handling", "[edge_cases]") {
    CampusCompass compass;
    compass.ParseCSV("data/edges.csv", "data/classes.csv");

    // Large student ID
    REQUIRE_FALSE(compass.remove(99887766));

    // Insert, dropClass, and remove check
    REQUIRE(compass.insert("Ally Brier", 12345678, 1, {"COP3530"}));
    REQUIRE(compass.dropClass(12345678, "COP3530"));
    REQUIRE_FALSE(compass.remove(12345678));

    // Checks does not exist is returned
    REQUIRE(compass.checkEdgeStatus(99999, 88888) == 2);

    // Verify should return an empty schedule since only one class
    compass.insert("Coby Davidson", 23456789, 1, {"COP3530"});
    auto schedule = compass.verifySchedule(23456789);
    REQUIRE(schedule.empty());
}

TEST_CASE("Class adjustments and bulk removal operations", "[operations]") {
    CampusCompass compass;
    compass.ParseCSV("data/edges.csv", "data/classes.csv");

    // Insert three students
    compass.insert("Ally Brier", 12345678, 1, {"COP3530", "CDA3101"});
    compass.insert("Coby Davidson", 23456789, 1, {"COP3530", "MAC2311"});
    compass.insert("Ethan Gold", 87654321, 1, {"COP3530"});

    // Ally class replace checks
    REQUIRE_FALSE(compass.replaceClass(12345678, "COP3502", "MAC2311"));
    REQUIRE_FALSE(compass.replaceClass(12345678, "COP3530", "CDA3101"));
    REQUIRE(compass.replaceClass(12345678, "COP3530", "MAC2311"));

    // Ally drop class checks
    REQUIRE_FALSE(compass.dropClass(12345678, "COP3530"));
    REQUIRE(compass.dropClass(12345678, "MAC2311"));

    // COP3530 remove class checks
    auto [success, count] = compass.removeClass("COP3530");
    REQUIRE(success);
    REQUIRE(count == 2);

    // Coby and Ethan remove checks
    REQUIRE_FALSE(compass.remove(87654321));
    REQUIRE(compass.remove(23456789));
}

TEST_CASE("Shortest edges path reachability with edge closures", "[routing]") {
    CampusCompass compass;
    compass.ParseCSV("data/edges.csv", "data/classes.csv");

    // Insert Ally Brier
    compass.insert("Ally Brier", 12345678, 1, {"COP3530"});

    // Verify reachability
    vector<int> times = compass.printShortestEdges(12345678);
    REQUIRE_FALSE(times.empty());
    REQUIRE(times[0] != -1);

    // Close three edges
    compass.toggleEdgesClosure({1, 4, 1, 2, 1, 50});

    // Verify unreachability
    times = compass.printShortestEdges(12345678);
    REQUIRE(times[0] == -1);

    // Verify edge reopening and reachability
    compass.toggleEdgesClosure({1, 4, 1, 2, 1, 50});
    times = compass.printShortestEdges(12345678);
    REQUIRE(times[0] != -1);
}