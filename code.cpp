#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

class Vector {
public:
    double x, y;
    Vector(double x, double y) : x(x), y(y) {}
    Vector(Vector &o) : x(o.x), y(o.y) {}
    
    double norm() {
        return sqrt(x*x + y*y);
    }
    
    void normalize() {
        double n = norm();
        x /= n;
        y /= n;
    }
    
    double dot(Vector &o) {
        return x*o.x + y*o.y;
    }
};

int main()
{
    constexpr int maxThrust = 100;
    constexpr int boostDistThresh = 5000;
    constexpr int boostAngleTolerance = 7;
    
    int x, y;
    int nextCheckpointX, nextCheckpointY;
    int nextCheckpointDist;
    int nextCheckpointAngle;
    int opponentX, opponentY;
    
    int newThrust = 0;
    bool boost = false, boostInUse = false, boostUsed = false;
    int distToOpponent = 0;
    
    while (true) {
        /* --- Entrée --- */
        cin >> x >> y
            >> nextCheckpointX >> nextCheckpointY
            >> nextCheckpointDist
            >> nextCheckpointAngle
            >> opponentX >> opponentY;
        cin.ignore();
        
        /* --- Calcule des nouveaux paramètres --- */
        
        // Calcule la poussée en fonction de l'angle entre le sens du mouvement et le checkpoint
        newThrust = (int)min((double)maxThrust, .08*nextCheckpointDist*max(0.0, cos(M_PI*nextCheckpointAngle/180)));
        
        Vector podToChkp(nextCheckpointX - x, nextCheckpointY - y),
            podToOpp(opponentX - x, opponentY - y);
        distToOpponent = podToOpp.norm();
        if(distToOpponent < 1000) {
            podToChkp.normalize();
            podToOpp.normalize();
            // Calcule le cosinus de l'angle entre le vecteur Pod->Checkpoint et le vecteur Pod->Adversaire
            // Si l'ennemi est dans une zone proche à l'arrière du pod, le pod ralenti afin de tenter de le ralentir
            if(podToChkp.dot(podToOpp) < 0)
                newThrust = 25;
        }
        
        // active le boost sur une ligne droite et si le checkpoint est assez loin pour éviter les dérapages
        boost = nextCheckpointDist > boostDistThresh
              && abs(nextCheckpointAngle) < boostAngleTolerance;
              
        string thrustCmd;
        if(boost && !boostUsed)
        {
            thrustCmd = "BOOST";
            boostInUse = true;
        }
        else
        {
            thrustCmd = to_string(newThrust);
            if(boostInUse) boostUsed = true;
            boostInUse = false;
        }
        
        /* --- Sortie --- */
        
        cout << nextCheckpointX << " " << nextCheckpointY << " " << thrustCmd << endl;
    }
}