#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <list>
#include <map>
#include <cmath>
#include <string>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class Dirt;
class FrackMan;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir) :GameWorld(assetDir) {}
	~StudentWorld();
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	FrackMan* getFrackMan() const { return m_player; }
	void decreaseBarrel() { m_barrelsLeft--; }
	void addObject(Actor* object);
	void activateSonar();

	bool canActorMoveTo(Actor* a, int x, int y) const;
	Actor* findNearbyPickerUpper(Actor* a, int radius) const;
	int annoyAllNearbyActors(Actor* annoyer, int points, int radius);
	void deleteDirt(Actor* object);
	void updateExitField();
	void updateFrackManField();
	GraphObject::Direction lineOfSightToFrackMan(Actor* a) const;
	GraphObject::Direction determineFirstMoveToExit(int x, int y);
	GraphObject::Direction determineFirstMoveToFrackMan(int x, int y);
private:
	Actor *m_dirt[VIEW_HEIGHT][VIEW_WIDTH];
	FrackMan *m_player;
	std::list <Actor*> m_objects;

	int m_barrelsLeft, m_numProtesters, m_maxProtesters, m_numTicks, m_maxTicks;
	int toExit[VIEW_HEIGHT - 1][VIEW_WIDTH - 1];
	int toFrackMan[VIEW_HEIGHT - 1][VIEW_WIDTH - 1];
	
	bool checkDirt(int x, int y) const;
	bool checkBoulder(Actor* a, int x, int y) const;
	int fieldMinValue(int field[][VIEW_HEIGHT - 1], int row, int col) const;
	void updateField(int field[][VIEW_HEIGHT - 1], int x, int y);
	void insertObjects();
	void setDisplayText();

	struct Coord
	{
		Coord(int rr, int cc) :r(rr), c(cc) {}
		int r, c;
	};
};

int randInt(int min, int max);
#endif // STUDENTWORLD_H_
