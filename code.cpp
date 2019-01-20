#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;


int main()
{
    constexpr int maxThrust = 100;
    constexpr int boostDistThresh = 7200; // 12 * rayon d'un checkpoint
    constexpr int boostAngleTolerance = 10;
    
    int thrust = maxThrust;
    bool boost = false, boostInUse = false, boostUsed = false;
    
    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY; cin.ignore();
        
        // ralenti aux virages
        thrust = (nextCheckpointAngle > 90 || nextCheckpointAngle < -90) ? 0 : maxThrust;
        // active le boost sur une ligne droite et si le checkpoint est assez loin pour éviter les dérapages
        boost = nextCheckpointDist > boostDistThresh
              && abs(nextCheckpointAngle) < boostAngleTolerance;

        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        cout << nextCheckpointX << " " << nextCheckpointY << " ";
        if(boost && !boostUsed)
        {
            cout << "BOOST";
            boostInUse = true;
        }
        else
        {
            cout << thrust;
            if(boostInUse) boostUsed = true;
            boostInUse = false;
        }
        cout << endl;
    }
}
