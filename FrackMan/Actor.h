#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"

class StudentWorld;

class Actor : public GraphObject
{
public:
	Actor(int ID, int startX, int startY, Direction dir, double size, int depth, StudentWorld* world) :GraphObject(ID, startX, startY, dir, size, depth)
	{ 
		setVisible(true);
		m_alive = true;
		m_world = world;
	}
	virtual ~Actor() {}
	virtual void doSomething() = 0;
	virtual bool isAlive() const{ return m_alive; }
	StudentWorld* getWorld() const;
	double distanceFrom(Actor* object) const;
	double distanceFrom(int xCoord, int yCoord) const;

	virtual bool isObstacle() const{ return false; }
	virtual bool huntsFrackMan() const { return false; }
	virtual bool canPickThingsUp() const { return false; }
	virtual bool canDigThroughDirt() const { return false; }

	void setDead() { m_alive = false; }
	virtual bool annoy(int amt);
	bool changeDirection(int inputDir);
	bool attemptMove(int dir);
private:
	StudentWorld* m_world;
	bool m_alive;
	bool checkObstacle();
};

//ACTORS THAT CAN MOVE NEED A STUDENTWORLD POINTER IN ORDER TO INTERACT WITH DIRT AND OTHER OBJECTS
class MoveableActor : public Actor
{
public:
	MoveableActor::MoveableActor(int ID, int startX, int startY, Direction dir, double size, int depth, StudentWorld* world, int health) :Actor(ID, startX, startY, dir, size, depth, world) { m_health = health; }
	
	int getHealth() const { return m_health; }
	virtual bool annoy(int amt);
	
private:
	int m_health;
};

class FrackMan : public MoveableActor
{
public:
	FrackMan(StudentWorld *world);
	virtual void doSomething();
	void fireSquirt();
	int getWater() const{ return m_water; }
	int getSonar() const{ return m_sonar; }
	int getNugget() const{ return m_nugget; }
	virtual void addGold() { m_nugget++; }
	void increaseWater() { m_water += 5; }
	void increaseSonar() { m_sonar++; }
	virtual bool annoy(int amt);

	virtual bool canDigThroughDirt() const { return true; }
	
private:
	int m_water, m_sonar, m_nugget;
};

class Protester : public MoveableActor
{
public:
	Protester(int ID, int startX, int startY, StudentWorld* world, int health);
	virtual void doSomething();
	virtual bool annoy(int amt);

	void changeState(){ m_leaveState = true; }

	void setTicksToNextMove();
	void setTicksToNextMove(int amt) { m_ticksPerMove = amt; }

	virtual void addGold() { changeState(); }
	virtual int getPoints() { return 100; }
	virtual bool huntsFrackMan() const { return true; }
	virtual bool canPickThingsUp() const { return true; }
	virtual bool honingMove() { return false; }
	GraphObject::Direction canTurn();
private:
	bool m_leaveState;
	int m_ticksPerMove, m_numTicks, m_numStepsToTake, m_nonRestingTicks, m_turnNonRestingTicks;
};

class HardcoreProtester : public Protester
{
public:
	HardcoreProtester(int startX, int startY, StudentWorld* world) :Protester(IID_HARD_CORE_PROTESTER, startX, startY, world, 20) {}
	virtual void addGold();
	virtual int getPoints(){ return 250; }
	virtual bool honingMove();
};

class Boulder : public Actor
{
public:
	Boulder(int startX, int startY, StudentWorld* world) :Actor(IID_BOULDER, startX, startY, down, 1, 1, world), m_state(0), m_numTicks(0) {}
	virtual void doSomething();
	virtual bool isObstacle() const{ return true; }
private:
	int m_state;
	int m_numTicks;
};

class Squirt : public Actor
{
public:
	Squirt(int startX, int startY, Direction d, StudentWorld* world) : Actor(IID_WATER_SPURT, startX, startY, d, 1, 1, world) { m_distanceTraveled = 0; }
	virtual void doSomething();
private:
	int m_distanceTraveled;
};

// ACTORS THAT DON'T MOVE
class Dirt : public Actor
{
public:
	Dirt(int startX, int startY, StudentWorld* world) :Actor(IID_DIRT, startX, startY, right, 0.25, 3, world) { }
	virtual void doSomething() { }
};

class Goodie :public Actor {
public:
	Goodie(int ID, int startX, int startY, StudentWorld* world) :Actor(ID, startX, startY, right, 1, 2, world) {}
	void pickedUp();
private:
	virtual void increaseScore() = 0;

};

// DERIVED FROM GOODIE

class Barrel :public Goodie
{
public:
	Barrel(int startX, int startY, StudentWorld* world) :Goodie(IID_BARREL, startX, startY, world) { 
		setVisible(false); 
	}
	virtual void doSomething();
	
private:
	virtual void increaseScore();
};

class Nugget : public Goodie
{
public:
	Nugget(int startX, int startY, bool visibility, int state, int pickable, StudentWorld* world) :Goodie(IID_GOLD, startX, startY, world) {
		setVisible(visibility);
		m_state = state; // 0 for perm, 1 for temp
		m_pickable = pickable; // 0 for frack, 1 for protestor
		m_ticks = 100;
	}
	virtual void doSomething();
private:
	int m_state, m_pickable, m_ticks;
	virtual void increaseScore();
};


class TempGoodie :public Goodie
{
public:
	TempGoodie(int ID, int startX, int startY, StudentWorld* world);
	virtual void doSomething();
private:
	int m_maxTicks, m_curTicks;
};

// DERIVED FROM TEMPGOODIE

class Sonar : public TempGoodie
{
public:
	Sonar(StudentWorld* world) :TempGoodie(IID_SONAR, 0, 60, world) {}
private:
	virtual void increaseScore();
};

class WaterPool : public TempGoodie
{
public:
	WaterPool(int startX, int startY, StudentWorld* world) :TempGoodie(IID_WATER_POOL, startX, startY, world) {}
private:
	virtual void increaseScore();
};

#endif // ACTOR_H_
