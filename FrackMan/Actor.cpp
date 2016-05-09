#include "Actor.h"
#include "GraphObject.h"
#include "GameConstants.h"
#include "StudentWorld.h"
#include <algorithm>
#include <cmath>
using namespace std;

bool Actor::attemptMove(int dir)
{
	if (checkObstacle())
		return false;

	switch (dir) {
	case 1: // UP
		if (getY() + 1 <= 60) {
			moveTo(getX(), getY() + 1);
			return true;
		}
		break;
	case 2: // DOWN
		if (getY() - 1 >= 0) {
			moveTo(getX(), getY() - 1);
			return true;
		}
		break;
	case 3: // LEFT
		if (getX() - 1 >= 0) {
			moveTo(getX() - 1, getY());
			return true;
		}
		break;
	case 4: // RIGHT
		if (getX() + 1 <= 60) {
			moveTo(getX() + 1, getY());
			return true;
		}
		break;
	}

	moveTo(getX(), getY());
	return false;
}

bool Actor::checkObstacle()
{
	switch (getDirection())
	{
	case up:
		return !getWorld()->canActorMoveTo(this, getX(), getY() + 1);
	case down:
		return !getWorld()->canActorMoveTo(this, getX(), getY() - 1);
	case left:
		return !getWorld()->canActorMoveTo(this, getX() - 1, getY());
	default:
		return !getWorld()->canActorMoveTo(this, getX() + 1, getY());
	}
}

bool Actor::changeDirection(int inputDir) {
	switch (inputDir) {
	case KEY_PRESS_UP:
	case up:
		if (getDirection() != up) {
			setDirection(up);
			return true;
		}
		break;
	case KEY_PRESS_DOWN:
	case down:
		if (getDirection() != down) {
			setDirection(down);
			return true;
		}
		break;
	case KEY_PRESS_LEFT:
	case left:
		if (getDirection() != left) {
			setDirection(left);
			return true;
		}
		break;
	case KEY_PRESS_RIGHT:
	case right:
		if (getDirection() != right) {
			setDirection(right);
			return true;
		}
	}
	return false;
}

bool Actor::annoy(int amt) {
	return huntsFrackMan();
}

double Actor::distanceFrom(Actor* object) const
{
	return sqrt(pow(getX() - object->getX(), 2) + pow(getY() - object->getY(), 2));
}

double Actor::distanceFrom(int xCoord, int yCoord) const
{
	return sqrt(pow(getX() - xCoord, 2) + pow(getY() - yCoord, 2));
}

StudentWorld* Actor::getWorld() const
{
	return m_world;
}

///////////////////////////////
//////////MOVEABLE//ACTOR/////////////
///////////////////////////////

bool MoveableActor::annoy(int amt) {
	m_health -= amt;
	return true;
}

///////////////////////////////
//////////FRACKMAN/////////////
///////////////////////////////
FrackMan::FrackMan(StudentWorld* world) :MoveableActor(IID_PLAYER, 30, 60, right, 1, 0, world, 10)
{
	m_water = 5;
	m_sonar = 1;
	m_nugget = 0;
}

void FrackMan::doSomething()
{
	if (!isAlive())
		return;

	int c;
	if (getWorld()->getKey(c)) {
		switch (c) {
		case KEY_PRESS_LEFT:
		case KEY_PRESS_RIGHT:
		case KEY_PRESS_DOWN:
		case KEY_PRESS_UP:
			if (!changeDirection(c)) {
				attemptMove(getDirection());
				getWorld()->updateFrackManField();
				getWorld()->deleteDirt(this);
			}
			break;
		case KEY_PRESS_ESCAPE:
			setDead();
			return;
		case KEY_PRESS_SPACE:
			fireSquirt();
			break;
		case KEY_PRESS_TAB:
			if (m_nugget > 0) {
				Actor* nugget = new Nugget(getX(), getY(), true, 1, 1, getWorld());
				getWorld()->addObject(nugget);
				m_nugget--;
			}
			break;
		case 'z':
		case 'Z':
			if (m_sonar > 0) {
				getWorld()->activateSonar();
				getWorld()->playSound(SOUND_SONAR);
				m_sonar--;
			}
			break;
		}
	}
}

void FrackMan::fireSquirt()
{
	if (m_water <= 0)
		return;
	m_water--;
	getWorld()->playSound(SOUND_PLAYER_SQUIRT);

	Actor* squirt;
	switch (getDirection())
	{
	case up:
		if (!getWorld()->canActorMoveTo(nullptr, getX(), getY() + 4))
			return;
		squirt = new Squirt(getX(), getY() + 4, up, getWorld());
		break;
	case down:
		if (!getWorld()->canActorMoveTo(nullptr, getX(), getY() - 4))
			return;
		squirt = new Squirt(getX(), getY() - 4, down, getWorld());
		break;
	case left:
		if (!getWorld()->canActorMoveTo(nullptr, getX() - 4, getY()))
			return;
		squirt = new Squirt(getX() - 4, getY(), left, getWorld());
		break;
	case right:
		if (!getWorld()->canActorMoveTo(nullptr,getX() + 4, getY()))
			return;
		squirt = new Squirt(getX() + 4, getY(), right, getWorld());
		break;
	}
 
	getWorld()->addObject(squirt);
}

bool FrackMan::annoy(int amt)
{
	MoveableActor::annoy(amt);
	if (getHealth() <= 0)
		setDead();
	return true;
}

////////////////////////////////
//////////PROTESTER/////////////
////////////////////////////////
Protester::Protester(int ID, int startX, int startY, StudentWorld* world, int health) : MoveableActor(ID, startX, startY, left, 1, 0, world, health)
{
	m_leaveState = false;
	setTicksToNextMove();
	m_numTicks = m_nonRestingTicks = m_turnNonRestingTicks = 0;
	m_numStepsToTake = randInt(8, 60);
}

bool Protester::annoy(int amt)
{
	if (m_leaveState)
		return false;
	MoveableActor::annoy(amt);
	if (getHealth() > 0) {
		getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
		int curLevel = getWorld()->getLevel();
		m_ticksPerMove = max(50, 100 - curLevel * 10);
	}
	else {
		changeState();
		getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
		getWorld()->updateExitField();
		setTicksToNextMove();
		m_numTicks = m_ticksPerMove;
	}
	return true;
}

void Protester::setTicksToNextMove()
{
	int curLevel = getWorld()->getLevel();
	m_ticksPerMove = max(0, 3 - curLevel / 4);
}

void Protester::doSomething()
{
	if (!isAlive())
		return;

	if (m_numTicks != m_ticksPerMove) {
		m_numTicks++;
		return;
	}

	m_numTicks = 0;
	if (m_leaveState) {
		if (getX() == 60 && getY() == 60)
			setDead();
		else {
			Direction dir = getWorld()->determineFirstMoveToExit(getX(), getY());
			changeDirection(dir);
			attemptMove(dir);
		}
		return;
	}

	if (distanceFrom(getWorld()->getFrackMan()) <= 4 && m_nonRestingTicks == 15) {
		getWorld()->playSound(SOUND_PROTESTER_YELL);
		getWorld()->getFrackMan()->annoy(2);
		m_nonRestingTicks = 0;
		setTicksToNextMove(max(50, 100 - static_cast<int>(getWorld()->getLevel()) * 10));
		return;
	}

	m_nonRestingTicks++;
	setTicksToNextMove();

	if (honingMove())
		return;

	Direction dir = getWorld()->lineOfSightToFrackMan(this);
	if (dir != none && distanceFrom(getWorld()->getFrackMan()) > 4) {
		changeDirection(dir);
		attemptMove(getDirection());
		return;
	}

	m_numStepsToTake--;
	if (m_numStepsToTake <= 0) {
		do {
			setDirection(static_cast<Direction>(randInt(1, 4)));
		} while (!attemptMove(getDirection()));
		m_numStepsToTake = randInt(8, 60);
	}
	else if (m_turnNonRestingTicks >= 200) {
		Direction dir = canTurn();
		if (dir != none) {
			setDirection(dir);
			m_turnNonRestingTicks = 0;
			m_numStepsToTake = randInt(8, 60);
		}
	}else
		m_turnNonRestingTicks++;

	if (!attemptMove(getDirection()))
		m_numStepsToTake = 0;

}

GraphObject::Direction Protester::canTurn()
{
	switch (getDirection())
	{
	case up:
	case down:
	{
		bool moveLeft = getWorld()->canActorMoveTo(this, getX() - 1, getY());
		bool moveRight = getWorld()->canActorMoveTo(this, getX() + 1, getY());
		if (moveLeft && moveRight)
			if (randInt(1, 2) == 1)
				return left;
			else
				return right;
		else if (moveRight)
			return right;
		else if (moveLeft)
			return left;
		else
			return none;
	}
	case left:
	case right:
	{
		bool moveDown = getWorld()->canActorMoveTo(this, getX() - 1, getY());
		bool moveUp = getWorld()->canActorMoveTo(this, getX() + 1, getY());
		if (moveDown && moveUp)
			if (randInt(1, 2) == 1)
				return down;
			else
				return up;
		else if (moveDown)
			return down;
		else if (moveUp)
			return up;
		else
			return none;
	}
	}
}

bool HardcoreProtester::honingMove() {
	Direction dir = getWorld()->determineFirstMoveToFrackMan(getX(), getY());
	if (dir != none) {
		setDirection(dir);
		attemptMove(getDirection());
		return true;
	}

	return false;
}

void HardcoreProtester::addGold() {
	getWorld()->increaseScore(25);
	setTicksToNextMove(max(50, 100 - static_cast<int>(getWorld()->getLevel()) * 10));
}

//////////////////////////////
//////////BOULDER/////////////
//////////////////////////////
void Boulder::doSomething()
{
	if (!isAlive())
		return;

	switch (m_state)
	{
	case 0:
		if(getWorld()->canActorMoveTo(this, getX(), getY() - 1))
			m_state++;
		break;
	case 1:
		if (m_numTicks == 30) {
			m_state++;
			getWorld()->playSound(SOUND_FALLING_ROCK);
		}
		else
			m_numTicks++;
		break;
	case 2:
		if (!attemptMove(getDirection()))
			setDead();
		else {
			getWorld()->increaseScore(getWorld()->annoyAllNearbyActors(this, 10, 3) * 500);
		}
		break;
	}
}

/////////////////////////////
//////////SQUIRT/////////////
/////////////////////////////
void Squirt::doSomething()
{
	if (!isAlive())
		return;

	if (getWorld()->annoyAllNearbyActors(this, 2, 3) > 0) {
		Protester *a = dynamic_cast<Protester*>(getWorld()->findNearbyPickerUpper(this, 3));
		if (a != nullptr && a->getHealth() <= 0)
			getWorld()->increaseScore(a->getPoints());
		setDead();
		return;
	}

	if (m_distanceTraveled >= 4) {
		setDead();
		return;
	}

	if (attemptMove(getDirection()))
		m_distanceTraveled++;
	else
		setDead();
}

void Goodie::pickedUp()
{
	if (getWorld()->getFrackMan()->distanceFrom(this) <= 3) {
		setDead();
		increaseScore();
	}
}

/////////////////////////////
//////////TEMPGOODIE/////////////
/////////////////////////////
TempGoodie::TempGoodie(int ID, int startX, int startY, StudentWorld* world) :Goodie(ID, startX, startY, world)
{
	int curLevel = getWorld()->getLevel();
	m_maxTicks = min(300 - 10 * curLevel, 100);
	m_curTicks = 0;
}

void TempGoodie::doSomething()
{
	if (!isAlive())
		return;

	pickedUp();

	if (m_curTicks == m_maxTicks)
		setDead();
	else
		m_curTicks++;
}


/////////////////////////////
//////////BARREL/////////////
/////////////////////////////
void Barrel::doSomething()
{
	if (!isAlive())
		return;

	if (getWorld()->getFrackMan()->distanceFrom(this) <= 4) {
		setVisible(true);
	}

	pickedUp();
}

void Barrel::increaseScore()
{
	getWorld()->playSound(SOUND_FOUND_OIL);
	getWorld()->increaseScore(1000);
	getWorld()->decreaseBarrel();
}

/////////////////////////////
//////////NUGGET/////////////
/////////////////////////////
void Nugget::doSomething()
{
	if (!isAlive())
		return;
	
	if (getWorld()->getFrackMan()->distanceFrom(this) <= 4) {
		setVisible(true);
	}

	if (m_state == 0 && m_pickable == 0)
		pickedUp();
	else if (m_state == 1 && m_pickable == 1) {
		Actor* a = getWorld()->findNearbyPickerUpper(this, 3);
		if (a != nullptr) {
			setDead();
			getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
			getWorld()->increaseScore(25);
			dynamic_cast<Protester*>(a)->addGold();
		}
	}
	else if (m_ticks == 0)
		setDead();
	else
		m_ticks--;
}

void Nugget::increaseScore()
{
	getWorld()->playSound(SOUND_GOT_GOODIE);
	getWorld()->increaseScore(10);
	getWorld()->getFrackMan()->addGold();
}

////////////////////////////
//////////SONAR/////////////
////////////////////////////
void Sonar::increaseScore() {
	getWorld()->playSound(SOUND_GOT_GOODIE);
	getWorld()->getFrackMan()->increaseSonar();
	getWorld()->increaseScore(75);
}

/////////////////////////////
//////////WATERPOOL//////////
/////////////////////////////
void WaterPool::increaseScore() {
	getWorld()->playSound(SOUND_GOT_GOODIE);
	getWorld()->getFrackMan()->increaseWater();
	getWorld()->increaseScore(100);
}