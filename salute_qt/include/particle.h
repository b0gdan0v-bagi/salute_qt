#pragma once

class Particle
{
	QVector2D m_pos;
	QVector2D m_speedV;

	int m_health = { 1000 };
	QColor m_color;
	int m_pSize = { 5 };
	bool m_life = { true };

public:
	Particle() {}
	Particle(QVector2D pos_, QVector2D speedV_, int health_, QColor color_, int pSize_) 
		:m_pos(pos_), m_speedV(speedV_), m_health(health_), m_color(color_), m_pSize(pSize_) {}

	

	const QVector2D getPos() { return m_pos; }
	const int getHealth() { return m_health; }
	const void setHealth(int health_) { m_health = health_; }
	const bool isAlive() { return m_life; }
	const QColor getColor() { return m_color; }
	const void reduceSize(int sizeToReduce) { m_pSize -= sizeToReduce; }
	const int getSize() { return m_pSize; }

	void update(const QVector2D &wind)
	{
		if (m_life) m_pos += m_speedV + wind;
		m_health--;

		// illumitantion
		int randLumi = QRandomGenerator::global()->bounded(0, 20);
		m_color = m_color.lighter(90 + randLumi);

		if (m_health <= 0) m_life = false;
	}
};

class FireLine
{
	
	int m_initLength;

	// save particles atributes from constructor to generate line
	Particle tailOldPos;

	bool m_life = { true };
	bool m_exploded = { false };
	bool m_parent; // true - can generate 2nd lines, false - cant
	int m_explosedHeight = { 0 }; // top of the screen
	bool m_toDelete = { false };
	int m_wind = { 0 };

public:

	QList<Particle*> line;

	
	FireLine(QVector2D headPos, QVector2D speedV_, int health_ , int initLength_, bool parent_, QColor color_, int pSize)
		:m_initLength(initLength_), m_parent(parent_)
	{
		tailOldPos = Particle(headPos, speedV_, health_, color_, pSize);
		line.push_back(new Particle(tailOldPos));
		if (pSize > 2) tailOldPos.reduceSize(2); // head is bigger than other particles
	}

	~FireLine()
	{
		for (auto it = line.begin(); it!=line.end();it++)
		{
			delete *it;
		}
	}

	const bool isAlive() { return m_life; }
	void setParentAtribute(bool isParent) { m_parent = isParent; }
	void setWind(int w) { m_wind = w; }
	const bool isParent() { return m_parent; }
	const bool isExploded() { return m_exploded; }
	const bool readyToDelete() { return m_toDelete; }

	void update()
	{
		//if (life)
		for (auto& l : line)
			l->update(QVector2D(m_wind,0));

		// make line after head
		if (m_initLength > 0)
		{
			line.push_back(new Particle(tailOldPos));
			//line.back().pSize == line.front().pSize - 3;
			tailOldPos = *line.back();
			m_initLength--;
		}

		// start explosion by timer
		if (line.front()->getHealth() == 0) m_exploded = true;
		// end explosion by timer
		if (line.back()->getHealth() == 0) m_life = false;

		// start explosion by height
		if (line.front()->getPos().y() <= m_explosedHeight && !m_exploded) {
			m_exploded = true;
			int healthCount = 0;
			for (auto& l : line)
			{
				//update health to make tail exploded at right timings
				l->setHealth(healthCount);
				healthCount++;
			}
				
		}
		if (line.front()->getPos().y() <= m_explosedHeight) m_life = false;
		if (!line.last()->isAlive()) m_toDelete = true;

	}
	void setExplodedHeight(int height_) { m_explosedHeight = height_; }
};