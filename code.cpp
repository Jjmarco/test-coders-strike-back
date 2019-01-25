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
    Vector() : Vector(0., 0.) {}
    Vector(const Vector& o) : Vector(o.x, o.y) {}
    
    void set(double _x, double _y) {
        x = _x;
        y = _y;
    }
    
    void set(int _x, int _y) {
        x = _x;
        y = _y;
    }
    
    void set(const Vector& o) {
        set(o.x, o.y);
    }
    
    double norm() const {
        return sqrt(x*x + y*y);
    }
    
    void normalize() {
        double n = norm();
        x /= n;
        y /= n;
    }
    
    Vector getNormalized() const {
        Vector v(*this);
        v.normalize();
        return v;
    }
    
    void add(Vector& o) {
        x += o.x;
        y += o.y;
    }
    
    void scalar(double s) {
        x *= s;
        y *= s;
    }
    
    double dot(const Vector& o) const {
        return x*o.x + y*o.y;
    }
};

Vector operator+(const Vector& u, const Vector& v) {
    return Vector(u.x + v.x, u.y + v.y);
}

Vector operator*(double s, const Vector& u) {
    return Vector(s*u.x, s*u.y);
}

Vector operator-(const Vector& u, const Vector& v) {
    return operator+(u, -1*v);
}



int main()
{
    constexpr int maxThrust = 100;
    constexpr int boostDistThresh = 5000;
    constexpr int boostAngleTolerance = 7;
    constexpr double slowdownMult = .06;
    
    int x, y;
    int nextCheckpointX, nextCheckpointY;
    int nextCheckpointDist;
    int nextCheckpointAngle;
    int opponentX, opponentY;
    
    int newThrust = 0;
    bool boost = false, boostInUse = false, boostUsed = false;
    
    Vector podToChkp, podToOpp, moveDelta, newTarget;
    int lastX = 0, lastY = 0;
    
    while (true) {
        /* --- Entrée --- */
        cin >> x >> y
            >> nextCheckpointX >> nextCheckpointY
            >> nextCheckpointDist
            >> nextCheckpointAngle
            >> opponentX >> opponentY;
        cin.ignore();
        
        podToChkp.set(nextCheckpointX - x, nextCheckpointY - y);
        podToOpp.set(opponentX - x, opponentY - y);
        moveDelta.set(x - lastX, y - lastY);
        
        /* --- Calcule des nouveaux paramètres --- */
        
        // Calcule la poussée en fonction de l'angle entre le sens du mouvement et le checkpoint
        newThrust = min(maxThrust, max(0, (int)(slowdownMult*nextCheckpointDist*cos(M_PI*nextCheckpointAngle/180))));
        
        string thrustCmd;
        if(podToOpp.norm() < 1000) {
            // Calcule le cosinus de l'angle entre le vecteur Pod->Checkpoint et le vecteur Pod->Adversaire
            // Si l'ennemi est dans une zone proche à l'arrière du pod, le pod ralenti afin de tenter de le ralentir
            if(podToChkp.getNormalized().dot(podToOpp.getNormalized()) < -.7)
            {
                if(nextCheckpointDist > 1600)
                {
                    newThrust *= .3;
                }
                else
                {
                    thrustCmd = "SHIELD";
                }
            }
        }
        
        // Compensation de dérapage
        int targetX = nextCheckpointX,
            targetY = nextCheckpointY;
        if(lastX != 0 && lastY != 0)
        {  
            newTarget = podToChkp - 1.5*moveDelta;
            targetX = newTarget.x + x;
            targetY = newTarget.y + y;
        }
        
        // active le boost sur une ligne droite et si le checkpoint est assez loin pour éviter les dérapages
        boost = nextCheckpointDist > boostDistThresh
              && abs(nextCheckpointAngle) < boostAngleTolerance;
              
        if(boost && !boostUsed)
        {
            thrustCmd = "BOOST";
            boostInUse = true;
        }
        else if(thrustCmd != "SHIELD")
        {
            thrustCmd = to_string(newThrust);
            if(boostInUse) boostUsed = true;
            boostInUse = false;
        }
        
        /* --- Sortie --- */
        
        cout << targetX << " " << targetY << " " << thrustCmd << endl;
        lastX = x;
        lastY = y;
    }
}