#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

class Vector;
class Map;
class Checkpoint;
class Pod;

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

class Checkpoint {
private:
    Vector pos;

public:
    Checkpoint(int x, int y) : pos(x, y) {}
    Checkpoint() : Checkpoint(0, 0) {}
    
    const Vector& getPos() const {
        return pos;
    }
    
    static constexpr int checkpointRadius = 600;
};

class Map {
private:
    int laps;
    int checkpointCount;
    vector<Checkpoint> checkpoints;
    
    Pod *ally1, *ally2;
    Pod *opp1, *opp2;
    
    Map(int laps, int checkpointCount, Pod* ally1, Pod* ally2, Pod* opp1, Pod* opp2) : laps(laps),
                                                                                       checkpointCount(checkpointCount),
                                                                                       checkpoints(),
                                                                                       ally1(ally1),
                                                                                       ally2(ally2),
                                                                                       opp1(opp1),
                                                                                       opp2(opp2)
                                                                                       {}
    
    static Map* instance;
    
public:
    static Map* createInstance(int laps, int checkpointCount, Pod* ally1, Pod* ally2, Pod* opp1, Pod* opp2) {
        if(instance == nullptr)
            instance = new Map(laps, checkpointCount, ally1, ally2, opp1, opp2);
        return instance; 
    }
    
    static Map* getInstance() {
        return instance;   
    }
    
    static void destroyInstance() {
        delete instance;
        instance = nullptr;
    }
    
    
    void addCheckpoint(int x, int y) {
        if(checkpoints.size() < checkpointCount) {
            checkpoints.push_back(Checkpoint(x, y));
        }
    }
    
    int getCheckpointCount() const {
        return checkpointCount;   
    }
    
    const Checkpoint& getCheckpoint(int id) const {
        return checkpoints[id];   
    }
    
    const Pod* getAlly1() const {
        return ally1;   
    }
    
    const Pod* getAlly2() const {
        return ally2;   
    }
    
    const Pod* getOpponent1() const {
        return opp1;   
    }
    
    const Pod* getOpponent2() const {
        return opp2;   
    }
};

Map* Map::instance = nullptr;

class Pod {
private:
    int x, y,
        vx, vy,
        angle, 
        nextCheckPointId;
        
    bool boostUsed;
        
    Pod *buddy;
        
public:
    Pod() : x(0), y(0),
            vx(0), vy(0),
            angle(0),
            nextCheckPointId(0),
            boostUsed(false),
            buddy(nullptr)
            {}
    
    void input(int _x, int _y, int _vx, int _vy, int _angle, int _nextCheckPointId, Pod* _buddy) {
        x = _x;
        y = _y;
        vx = _vx;
        vy = _vy;
        angle = _angle;
        nextCheckPointId = _nextCheckPointId;
        buddy = _buddy;
    }
    
    void computeNextParams(int* targetX, int* targetY, string* thrustCmd) {
        Vector pos(x, y), v(vx, vy), posToChkpt, target, posToAlly, posToAfter;
        
        const Checkpoint& nextCheckpoint = Map::getInstance()->getCheckpoint(nextCheckPointId);
        
        posToChkpt.set(nextCheckpoint.getPos().x - pos.x, nextCheckpoint.getPos().y - pos.y);
        int distToChkpt = posToChkpt.norm();
        
        // Calcul de la nouvelle propulsion
        int thrust = min(100, (int)(slowdownMult*distToChkpt));
        
        // Compensation du déparage
        target = posToChkpt - 4*v;
        
        // Anticipation du prochain checkpoint
        if(distToChkpt < 4*v.norm()) {
            int chkptAfterId = (nextCheckPointId + 1) % Map::getInstance()->getCheckpointCount();
            const Checkpoint& checkpointAfter = Map::getInstance()->getCheckpoint(chkptAfterId);
            posToAfter.set(checkpointAfter.getPos().x - pos.x, checkpointAfter.getPos().y - pos.y);
            target = posToAfter - 4*v;
        }
        
        // Vérification de la position du pod allié
        posToAlly.set(buddy->getPos().x - pos.x, buddy->getPos().y - pos.y);
        double distToAlly = posToAlly.norm(), distToTarget = target.norm();
        double cosAllyTarget = posToAlly.dot(target)/(distToAlly*distToTarget);
        if(distToAlly < 2*forcefieldRadius + 300 && cosAllyTarget > .5)
        {
            thrust *= .25;
        }
        
        // Activation du boost
        if(!boostUsed && distToChkpt > 4000) {
            thrust = -1;
            boostUsed = true;
        }
        
        // Ecriture des nouveaux paramètres
        *targetX = target.x + pos.x;
        *targetY = target.y + pos.y;
        *thrustCmd = thrust == -1 ? "BOOST" : to_string(thrust);
    }
    
    Vector getPos() const {
        return Vector(x, y);   
    }
    
    static constexpr int forcefieldRadius = 400;
    static constexpr double slowdownMult = .0833;
};


int main()
{
    int laps;
    cin >> laps; cin.ignore();
    int checkpointCount;
    cin >> checkpointCount; cin.ignore();
    
        
    Pod pods[2];
    Pod opps[2];
    
    Map* mapInstance = Map::createInstance(laps, checkpointCount, pods, pods + 1, opps, opps + 1);
    
    for (int i = 0; i < checkpointCount; i++) {
        int checkpointX;
        int checkpointY;
        cin >> checkpointX >> checkpointY; cin.ignore();
        
        mapInstance->addCheckpoint(checkpointX, checkpointY);
    }

    while (true) {
        for (int i = 0; i < 2; i++) {
            int x; // x position of your pod
            int y; // y position of your pod
            int vx; // x speed of your pod
            int vy; // y speed of your pod
            int angle; // angle of your pod
            int nextCheckPointId; // next check point id of your pod
            cin >> x >> y >> vx >> vy >> angle >> nextCheckPointId; cin.ignore();
            pods[i].input(x, y, vx, vy, angle, nextCheckPointId, &pods[1-i]);
        }
       for (int i = 0; i < 2; i++) {
            int x2; // x position of the opponent's pod
            int y2; // y position of the opponent's pod
            int vx2; // x speed of the opponent's pod
            int vy2; // y speed of the opponent's pod
            int angle2; // angle of the opponent's pod
            int nextCheckPointId2; // next check point id of the opponent's pod
            cin >> x2 >> y2 >> vx2 >> vy2 >> angle2 >> nextCheckPointId2; cin.ignore();
        } 

        int targetX, targetY;
        string thrust;
        
        pods[0].computeNextParams(&targetX, &targetY, &thrust);
        cout << targetX << " " << targetY << " " << thrust << endl;
        pods[1].computeNextParams(&targetX, &targetY, &thrust);
        cout << targetX << " " << targetY << " " << thrust << endl;
    }
    
    Map::destroyInstance();
}
