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

double radToDeg(double rad) {
    return 180*rad/M_PI;
}

double degToRad(double deg) {
    return M_PI*deg/180;   
}

class Point {
protected:
    int x, y;
    
public:
    Point(int x, int y) : x(x), y(y) {}
    Point() : Point(0, 0) {}
    
    int X() const {
        return x;   
    }
    
    int Y() const {
        return y;   
    }
    
    // Calcul le distance au carré sans racine (plus rapide)
    int distanceSq(const Point& o) const {
        int dx = o.x - x,
            dy = o.y - y;
        return dx*dx + dy*dy;
    }
    
    // Applique la racine carrée pour avoir la distance réelle si nécessaire
    int distance(const Point& o) const {
        return sqrt(distanceSq(o));
    }
};

class Vector {
public:
    double x, y;
    Vector(double x, double y) : x(x), y(y) {}
    Vector() : Vector(0., 0.) {}
    Vector(const Vector& o) : Vector(o.x, o.y) {}
    Vector(const Point& p1, const Point& p2) : Vector(p2.X() - p1.X(), p2.Y() - p1.Y()) {}
    
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
    
    void add(const Vector& o) {
        x += o.x;
        y += o.y;
    }
    
    void scalar(double s) {
        x *= s;
        y *= s;
    }
    
    bool isNull() const {
        return x == 0 && y == 0;   
    }
    
    double dot(const Vector& o) const {
        return x*o.x + y*o.y;
    }
    
    Vector& operator*=(double s) {
        this->scalar(s);
        return *this;   
    }
    
    Vector& operator+=(const Vector& o) {
        this->add(o);
        return *this;   
    }
    
    Vector& operator-=(const Vector& o) {
        x -= o.x;
        y -= o.y;
        return *this;   
    }
};

Vector operator+(const Vector& u, const Vector& v) {
    Vector res(u);
    res += v;
    return res;
}

Vector operator*(double s, const Vector& u) {
    Vector res(u);
    res *= s;
    return res;
}

Vector operator-(const Vector& u, const Vector& v) {
    return u + (-1*v);
}

class Entity : public Point {
protected:
    int radius;
    int vx, vy;
public:
    Entity(int x, int y, int radius) : Point(x, y), radius(radius), vx(0), vy(0) {}
    Entity() : Entity(0, 0, 0) {}
    
    void applySpeed() {
        x += vx;
        y += vy;
        // Application de la friction
        vx *= .85;
        vy *= .85;
    }
    
    static bool predictCollision(const Entity& e1, const Entity& e2, int turns) {
        Entity e1_c(e1),
               e2_c(e2);
        for(int i = 0; i < turns; i++)
        {
            e1_c.applySpeed();
            e2_c.applySpeed();
            if(e1_c.distance(e2_c) <= e1_c.radius + e2_c.radius)
                return true;
        }
        return false;
    }
};

class Checkpoint : public Entity {
public:
    Checkpoint(int x, int y) : Entity(x, y, checkpointRadius) {}
    Checkpoint() : Checkpoint(0, 0) {}
    
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

class Pod : public Entity {
private:
    int angle, 
        nextCheckPointId,
        lastCheckPointId,
        checkpointsPassed;
        
    bool boostUsed;
        
    Pod *buddy;
        
public:
    static constexpr int forcefieldRadius = 400;
    static constexpr double slowdownMult = .0833;
    static constexpr int BOOST = -1, SHIELD = -2;
    static constexpr int RACE = 0, ATTACK = 1;
    static constexpr double anticipationFactor = 5, compensationFactor = 3;

    Pod() : Entity(0, 0, forcefieldRadius),
            angle(0),
            nextCheckPointId(0),
            boostUsed(false),
            buddy(nullptr)
            {
                lastCheckPointId = 0;
                checkpointsPassed = 0;
            }
            
    bool isAhead(const Pod& other) const {
        const Checkpoint& nextCheckpointPod = Map::getInstance()->getCheckpoint(nextCheckPointId);
        const Checkpoint& nextCheckpointOther = Map::getInstance()->getCheckpoint(other.nextCheckPointId);
        
        if(checkpointsPassed == other.checkpointsPassed)
            return this->distanceSq(nextCheckpointPod) < other.distanceSq(nextCheckpointOther);
        return checkpointsPassed > other.checkpointsPassed;
    }
    
    void input(int _x, int _y, int _vx, int _vy, int _angle, int _nextCheckPointId, Pod* _buddy) {
        x = _x;
        y = _y;
        vx = _vx;
        vy = _vy;
        angle = _angle;
        
        if(_nextCheckPointId != nextCheckPointId)
        {
            checkpointsPassed++;
            lastCheckPointId = nextCheckPointId;
        }
        
        nextCheckPointId = _nextCheckPointId;
        buddy = _buddy;
    }
    
    void computeNextParams(int* targetX, int* targetY, string* thrustCmd, int behavior) {
        Map *map = Map::getInstance();
        Pod opp1 = *map->getOpponent1(),
            opp2 = *map->getOpponent2();
        Vector v(vx, vy), posToChkpt, target, posToAlly;
        int distToChkpt, thrust = 0;
        
        const Checkpoint& nextCheckpoint = map->getCheckpoint(nextCheckPointId);
        
        posToChkpt = Vector(*this, nextCheckpoint);
        distToChkpt = posToChkpt.norm();
        
        if(behavior == RACE) {
            // Anticipation du prochain checkpoint (sauf si checkpoint d'arrivée)
            if(distToChkpt < anticipationFactor*v.norm()
            && this->checkpointsPassed + 1 < 3*map->getCheckpointCount()) {
                int chkptAfterId = (nextCheckPointId + 1) % map->getCheckpointCount();
                target = Vector(*this, map->getCheckpoint(chkptAfterId));
            }
            else target = posToChkpt;
            // Compensation du déparage
            target -= compensationFactor*v;
        
            // Si une collision est iminente entre le pod et un ennemi,
            // et l'ennemi se trouve sur les côtés du pod, le pod active son bouclier
            // pour tenter se défendre d'une collsion ou pousser l'ennemi
            Vector posToOpp(0, 0);
            if(predictCollision(*this, opp1, 1))
                posToOpp = Vector(*this, opp1);
            else if(predictCollision(*this, opp2, 1))
                posToOpp = Vector(*this, opp2);
            if(!posToOpp.isNull())
            {
                double distToOpp = posToOpp.norm(), distToTarget = target.norm();
                double cosOppTarget = posToOpp.dot(target)/(distToOpp*distToTarget);
                if(cosOppTarget < .5 && cosOppTarget > -.5)
                    thrust = SHIELD;
            }
        }
        else if(behavior == ATTACK) {
            // Le pod d'attaque vise le pod ennemi en tête afin de tenter de le ralentir par des collisions
            if(opp2.isAhead(opp1))
                target = Vector(*this, opp2) + 3*compensationFactor*Vector(opp2.vx, opp2.vy);
            else
                target = Vector(*this, opp1) + 3*compensationFactor*Vector(opp1.vx, opp1.vy);
            
            // Activation du bouclier si prédiction de collision iminente
            if(predictCollision(*this, opp1, 1) || predictCollision(*this, opp2, 1))
                thrust = SHIELD;
        }
        
        // Calcul de l'angle entre le pod et la cible
        if(!thrust) {
            double angleTargetH = radToDeg(acos(target.x / target.norm()));
            if(target.y < 0) angleTargetH = 360. - angleTargetH;
            if(angle <= angleTargetH) angleTargetH = angleTargetH - angle;
            else                      angleTargetH = 360. - angle + angleTargetH;
            // Calcul de la nouvelle propulsion
            if(cos(degToRad(angleTargetH)) < -.1)
                thrust = 0;
            else
                thrust = min(100, max(0, (int)(slowdownMult*distToChkpt)));
        }
        
        // Vérification de la position du pod allié
        posToAlly = Vector(*this, *buddy);
        double distToAlly = posToAlly.norm(), distToTarget = target.norm();
        double cosAllyTarget = posToAlly.dot(target)/(distToAlly*distToTarget);
        if(distToAlly < 2*forcefieldRadius + 300 && cosAllyTarget > .5)
        {
            thrust *= .25;
        }
        
        /* Activation du boost si:
         * - Le pod est à une certaine distance du prochain checkpoint
         * - Pas de collison iminente avec un autre pod
         */
        if(behavior != ATTACK && !boostUsed && distToChkpt > 4000
        && !predictCollision(*this, *map->getOpponent1(), 1)
        && !predictCollision(*this, *map->getOpponent2(), 1)
        && !predictCollision(*this, *buddy, 1)) {
            thrust = BOOST;
            boostUsed = true;
        }
        
        // Ecriture des nouveaux paramètres
        *targetX = target.x + x;
        *targetY = target.y + y;
        switch(thrust) {
        case BOOST:
            *thrustCmd = "BOOST";
            break;
        case SHIELD:
            *thrustCmd = "SHIELD";
            break;
        default:
            *thrustCmd = to_string(thrust);
        }
    }
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
            opps[i].input(x2, y2, vx2, vy2, angle2, nextCheckPointId2, &opps[1-i]);
        } 

        int targetX, targetY;
        string thrust;
        
        pods[0].computeNextParams(&targetX, &targetY, &thrust, Pod::RACE);
        cout << targetX << " " << targetY << " " << thrust << endl;
        pods[1].computeNextParams(&targetX, &targetY, &thrust, Pod::ATTACK);
        cout << targetX << " " << targetY << " " << thrust << endl;
    }
    
    Map::destroyInstance();
}
