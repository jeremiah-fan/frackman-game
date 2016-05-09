#include "StudentWorld.h"
#include "GraphObject.h"
#include "GameConstants.h"
#include "Actor.h"
#include <algorithm>
#include <string>
#include <queue>
#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::~StudentWorld()
{
	for (int i = 0; i < 60; i++) {
		for (int j = 0; j < VIEW_WIDTH; j++) {
			delete m_dirt[i][j];
		}
	}

	delete m_player;
}

int StudentWorld::init()
{
	m_player = new FrackMan(this);

	for (int i = 0; i < VIEW_HEIGHT; i++) {
		for (int j = 0; j < VIEW_WIDTH; j++) {
			if (i > 3 && (j >= 30 && j <= 33) || (i >= 60 && i < 64))
				m_dirt[i][j] = nullptr;
			else
				m_dirt[i][j] = new Dirt(j, i,this);
		}
	}

	updateFrackManField();

	int curLevel = getLevel();
	m_barrelsLeft = min(2 + curLevel, 20);
	m_numProtesters = m_numTicks = 0;
	m_maxProtesters = min(15, 2 + static_cast<int>(curLevel * 1.5));
	m_maxTicks = max(25, 200 - curLevel);
	insertObjects();
	return 5;
}

int StudentWorld::move()
{
	setDisplayText();

	for (list <Actor*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
		(*it)->doSomething();

	m_player->doSomething();

	for (list <Actor*>::iterator it = m_objects.begin(); it != m_objects.end();) {
		if (!(*it)->isAlive()) {
			if ((*it)->canPickThingsUp())
				m_numProtesters--;
			delete *it;
			it = m_objects.erase(it);
		}
		else
			it++;
	}

	int randChance = randInt(0, getLevel() * 25 + 299);
	if (randChance == 0) {
		int randGoodie = randInt(0, 4);
		Actor* b;
		if (randGoodie == 0) {
			b = new Sonar(this);
		}
		else
			while (true) {
				int randX = randInt(0, 60);
				int randY = randInt(0, 60);
				bool isValid = true;
				for (int i = 0; i < 4; i++)
					for (int j = 0; j < 4; j++) {
						if (m_dirt[randY + j][randX + i] != nullptr) {
							isValid = false;
							break;
						}
					}

				if (isValid) {
					b = new WaterPool(randX, randY, this);
					break;
				}
			}
		addObject(b);
	}

	if (m_numProtesters < m_maxProtesters) {
		if (m_numTicks == m_maxTicks) {
			int curLevel = getLevel();
			int prob = min(90, curLevel * 10 + 30);
			int rand = randInt(1, 100);

			Actor *b;
			if (rand > prob)
				b = new Protester(IID_PROTESTER, 60, 60, this, 5);
			else
				b = new HardcoreProtester(60, 60, this);
			addObject(b);
			m_numProtesters++;
			m_numTicks = 0;
		}
		else
			m_numTicks++;
	}
	
	if (!m_player->isAlive()) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	if (m_barrelsLeft == 0)
		return GWSTATUS_FINISHED_LEVEL;

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (int i = 0; i < 60; i++) {
		for (int j = 0; j < VIEW_WIDTH; j++) {
			delete m_dirt[i][j];
			m_dirt[i][j] = nullptr;
		}
	}

	for (list <Actor*>::iterator it = m_objects.begin(); it != m_objects.end();) {
		delete *it;
		it = m_objects.erase(it);
	}

	delete m_player;
	m_player = nullptr;
}

void StudentWorld::setDisplayText()
{
	ostringstream oss;
	oss << "Src: ";
	oss.fill('0');
	oss << setw(6) << getScore() << "  ";
	oss << "Lvl: ";
	oss.fill(' ');
	oss << setw(2) << getLevel() << "  ";
	oss << "Lives: " << getLives() << "  ";
	oss << "Hlth: " << setw(3) << m_player->getHealth() * 10 << "%  ";
	oss << "Wtr: " << setw(2) << m_player->getWater() << "  ";
	oss << "Gld: " << setw(2) << m_player->getNugget() << "  ";
	oss << "Sonar: " << setw(2) << m_player->getSonar() << "  ";
	oss << "Oil Left: " << setw(2) << m_barrelsLeft;

	setGameStatText(oss.str());
}

void StudentWorld::insertObjects()
{
	int curLevel = getLevel();
	int prob = min(90, curLevel * 10 + 30);
	int rand = randInt(1, 100);

	Actor *b;
	if (rand > prob)
		b = new Protester(IID_PROTESTER, 60, 60, this, 5);
	else
		b = new HardcoreProtester(60, 60, this);
	addObject(b);
	m_numProtesters++;

	int numBoulders = min(curLevel / 2 + 2, 6);
	int numNuggets = max(5 - curLevel / 2, 2);
	int numBarrels = m_barrelsLeft;
	int order = 0; // 0 for Boulders, 1 for Gold, 2 for Oil
	while (numBoulders > 0 || numNuggets > 0 || numBarrels > 0) {
		int randX = randInt(0, 60);
		int randY = randInt(20, 56);
		bool isValid = true;
		if ((randX >= 27 && randX <= 33) && randY > 3)
			isValid = false;
		else
			for (list <Actor*> ::const_iterator it = m_objects.begin(); it != m_objects.end(); it++)
				if ((*it)->distanceFrom(randX, randY) <= 6)
					isValid = false;

		if (isValid) {
			if (numBoulders > 0) {
				b = new Boulder(randX, randY, this);
				numBoulders--;
				deleteDirt(b);
			}
			else if (numNuggets > 0) {
				b = new Nugget(randX, randY, false, 0, 0, this);
				numNuggets--;
			}
			else {
				b = new Barrel(randX, randY, this);
				numBarrels--;
			}
			addObject(b);
		}
	}
}

Actor* StudentWorld::findNearbyPickerUpper(Actor* a, int radius) const {
	for (list <Actor*>::const_iterator it = m_objects.begin(); it != m_objects.end(); it++)
		if ((*it)->canPickThingsUp() && (*it)->distanceFrom(a) <= radius)
			return *it;
	return nullptr;
}

void StudentWorld::addObject(Actor* object)
{
	m_objects.push_back(object);
}

void StudentWorld::activateSonar()
{
	for (list <Actor*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
		if (m_player->distanceFrom(*it) <= 12)
			(*it)->setVisible(true);
}

bool StudentWorld::checkDirt(int x, int y) const
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			if (m_dirt[y + j][x + i] != nullptr)
				return true;
	return false;
}

bool StudentWorld::checkBoulder(Actor* a, int x, int y) const {
	for (list <Actor*>::const_iterator it = m_objects.begin(); it != m_objects.end(); it++)
		if ((*it)->isObstacle() && (*it)->distanceFrom(x, y) <= 3 && a != (*it))
			return true;
	return false;
}

bool StudentWorld::canActorMoveTo(Actor* a, int x, int y) const {
	if (x < 0 || x > 60 || y < 0 || y > 60)
		return false;

	if (a != nullptr && a->canDigThroughDirt())
		return !checkBoulder(a, x, y);
	return !checkDirt(x, y) && !checkBoulder(a, x, y);
}

void StudentWorld::deleteDirt(Actor* object)
{
	bool deleted = false;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (m_dirt[object->getY() + j][object->getX() + i] != nullptr) {
				delete m_dirt[object->getY() + j][object->getX() + i];
				m_dirt[object->getY() + j][object->getX() + i] = nullptr;
				deleted = true;
			}
		}
	}

	if (deleted) {
		playSound(SOUND_DIG);
		updateExitField();
	}
}

int StudentWorld::annoyAllNearbyActors(Actor* annoyer, int points, int radius) {
	int num = 0;
	for (list<Actor*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
		if ((*it)->distanceFrom(annoyer) <= radius && (*it)->annoy(points))
			num++;

	if (m_player->distanceFrom(annoyer) <= radius && m_player->annoy(points))
		num++;
	return num;
}

void StudentWorld::updateExitField()
{
	updateField(toExit, 60, 60);
}

void StudentWorld::updateFrackManField()
{
	updateField(toFrackMan, m_player->getX(), m_player->getY());
}

void StudentWorld::updateField(int field[][VIEW_HEIGHT - 1], int x, int y) {
	for (int i = 0; i < VIEW_HEIGHT - 1; i++)
		for (int j = 0; j < VIEW_WIDTH - 1; j++)
			field[i][j] = 3000;

	queue <Coord> q;

	Coord c(y, x);
	q.push(c);
	while (!q.empty()) {
		Coord coord = q.front();
		q.pop();

		if (coord.r == y && coord.c == x)
			field[coord.r + 1][coord.c + 1] = 0;
		else {
			int minValue = fieldMinValue(field, coord.r, coord.c);
			field[coord.r + 1][coord.c + 1] = minValue + 1;
		}

		if (coord.r + 1 <= 60 && field[coord.r + 2][coord.c + 1] == 3000 && canActorMoveTo(nullptr, coord.c, coord.r + 1)) {// UP
			Coord toPush(coord.r + 1, coord.c);
			q.push(toPush);
		}

		if (coord.c + 1 <= 60 && field[coord.r + 1][coord.c + 2] == 3000 && canActorMoveTo(nullptr, coord.c + 1, coord.r)) { // RIGHT
			Coord toPush(coord.r, coord.c + 1);
			q.push(toPush);
		}

		if (coord.r - 1 >= 0 && field[coord.r][coord.c + 1] == 3000 && canActorMoveTo(nullptr, coord.c, coord.r - 1)) { // DOWN
			Coord toPush(coord.r - 1, coord.c);
			q.push(toPush);
		}

		if (coord.c - 1 >= 0 && field[coord.r + 1][coord.c] == 3000 && canActorMoveTo(nullptr, coord.c - 1, coord.r)) { // LEFT
			Coord toPush(coord.r, coord.c - 1);
			q.push(toPush);
		}
	}
}

GraphObject::Direction StudentWorld::determineFirstMoveToExit(int x, int y)
{
	int minValue = fieldMinValue(toExit, y, x);
	
	if (minValue == toExit[y + 2][x + 1])
		return GraphObject::up;
	else if (minValue == toExit[y][x + 1])
		return GraphObject::down;
	else if (minValue == toExit[y + 1][x])
		return GraphObject::left;
	else
		return GraphObject::right;
}

GraphObject::Direction StudentWorld::determineFirstMoveToFrackMan(int x, int y)
{
	if (toFrackMan[y + 1][x + 1] > 16 + static_cast<int>(getLevel()) * 2)
		return GraphObject::none;

	int minValue = fieldMinValue(toFrackMan, y, x);

	if (minValue == toFrackMan[y + 2][x + 1])
		return GraphObject::up;
	else if (minValue == toFrackMan[y][x + 1])
		return GraphObject::down;
	else if (minValue == toFrackMan[y + 1][x])
		return GraphObject::left;
	else
		return GraphObject::right;
}

GraphObject::Direction StudentWorld::lineOfSightToFrackMan(Actor* a) const
{
	for (int i = 0;; i++) { // LEFT
		if (!canActorMoveTo(a, a->getX() - i, a->getY()))
			break;

		if (a->getX() - i == m_player->getX() && a->getY() == m_player->getY())
			return GraphObject::left;
	}

	for (int i = 0;; i++) { // RIGHT
		if (!canActorMoveTo(a, a->getX() + i, a->getY()))
			break;

		if (a->getX() + i == m_player->getX() && a->getY() == m_player->getY())
			return GraphObject::right;
	}

	for (int i = 0;; i++) { // DOWN
		if (!canActorMoveTo(a, a->getX(), a->getY() - i))
			break;

		if (a->getY() - i == m_player->getY() && a->getX() == m_player->getX())
			return GraphObject::down;
	}

	for (int i = 0;; i++) { // UP
		if (!canActorMoveTo(a, a->getX(), a->getY() + i))
			break;

		if (a->getY() + i == m_player->getY() && a->getX() == m_player->getX())
			return GraphObject::up;
	}

	return GraphObject::none;
}

int StudentWorld::fieldMinValue(int field[][VIEW_HEIGHT - 1], int row, int col) const {
	int left = field[row + 1][col];
	int right = field[row + 1][col + 2];
	int down = field[row][col + 1];
	int up = field[row + 2][col + 1];

	return min(down, min(left, min(up, right)));
}

int randInt(int min, int max)
{
	if (max < min)
		swap(max, min);
	static random_device rd;
	static mt19937 generator(rd());
	uniform_int_distribution<> distro(min, max);
	return distro(generator);
}