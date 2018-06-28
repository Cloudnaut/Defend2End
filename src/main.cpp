#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include <sstream>
#include <time.h>
#include "Basics.hpp"
#include <list>
#include <math.h>
#define BULLETSPEED 20
#define STARTDAMAGE 2.25
#define STARTFIRERATE 4.25
#define DEFAULT_TOWER_RANGE 150
#define PI 3.14159

std::string FloatToString(float Input)
{
	std::stringstream ss;
	std::string TmpStr;
	ss << Input;
	ss >> TmpStr;
	return TmpStr;
}

class Bullet : public sf::Sprite
{
private:
	float Speed;
	float Damage;
	sf::Vector2f Direction;
public:
	float GetDamage()
	{
		return this->Damage;
	}

	void Update()
	{
		move(sf::Vector2f(Direction.x * this->Speed, Direction.y * this->Speed));
	}
	Bullet(sf::Texture &Texture, sf::Vector2f StartPos, sf::Vector2i MousePosition, float Damage)
	{
		this->setTexture(Texture);
		this->setOrigin(Texture.getSize().x/2, Texture.getSize().y/2);
		this->setPosition(StartPos);

		this->Speed = BULLETSPEED;
		this->Damage = Damage;

		sf::Vector2i Cursor = MousePosition;
		sf::Vector2f Direction = sf::Vector2f(Cursor.x, Cursor.y) - this->getPosition();
		this->setRotation(atan2(Direction.y, Direction.x) * 180 / PI);

		float DLen = sqrt(Direction.x * Direction.x + Direction.y * Direction.y);
		Direction.x /= DLen;
		Direction.y /= DLen;
		this->Direction = Direction;
	}
};

class Player : public sf::Sprite
{
private:
	sf::Clock FirerateCooldown;
	float Firerate;
	bool Ready;
public:
	float GetFirerate()
	{
		return this->Firerate;
	}
	void SetFirerate(float Value)
	{
		this->Firerate = Value;
	}
	bool IsReady()
	{
		return Ready;
	}
	void SetReady(bool Value)
	{
		this->Ready = Value;

		if(this->Ready == false)
			this->FirerateCooldown.restart();
	}
	void Update()
	{
		if(FirerateCooldown.getElapsedTime().asSeconds() >= 1 / Firerate)
		{
			this->Ready = true;
		}
	}
	Player(sf::Texture &Texture)
	{
		this->setTexture(Texture);
		this->Ready = true;
		this->Firerate = STARTFIRERATE;
	}
};

class Enemy : public sf::CircleShape
{
private:
	bool IsBoss;
	float Level;
	float Speed;
	float FullHP;
	float CurrentHP;
	float Loot;
	sf::CircleShape HealthCircle;
	sf::FloatRect CollisionArea;
public:

	bool GetIsBoss()
	{
		return this->IsBoss;
	}

	float GetLoot()
	{
		return this->Loot;
	}

	sf::CircleShape GetHealthCircle()
	{
		return this->HealthCircle;
	}

	void ComputeCollisionArea()
	{
		this->CollisionArea = sf::FloatRect(this->getPosition().x - this->getOrigin().x, this->getPosition().y - this->getOrigin().y, this->getRadius()*2, this->getRadius()*2);
	}

	sf::FloatRect GetCollisionArea()
	{
		return this->CollisionArea;
	}

	void DoDamage(float Value)
	{
		this->CurrentHP -= Value;
	}

	float GetCurrentHP()
	{
		return this->CurrentHP;
	}

	void Update()
	{
		move(0, Speed);
		this->HealthCircle.setRadius(((CurrentHP/FullHP)*100)*(this->getRadius()/100));
		this->HealthCircle.setOrigin(HealthCircle.getRadius(), HealthCircle.getRadius());
		this->HealthCircle.setPosition(this->getPosition());
		ComputeCollisionArea();
	}

	Enemy(float Level, sf::Vector2f StartPos)
	{
		this->setRadius(15);
		this->setOrigin(this->getRadius(), this->getRadius());
		this->setPosition(StartPos);

		if(rand()%10 == 0)
			IsBoss = true;
		else IsBoss = false;

		if(IsBoss)
		{
			this->setFillColor(sf::Color::Red);
			Speed = (Level/20)+(rand()%(int)Level)/10;
			FullHP = Level * 15 + (rand()%(int)Level)*5;
			Loot = Speed * FullHP;
		}
		else
		{
			this->setFillColor(sf::Color::Black);
			Speed = (Level/10)+(rand()%(int)Level)/10;
			FullHP = Level * 5 + (rand()%(int)Level)*2;
			Loot = Speed * FullHP;
		}

		CurrentHP = FullHP;

		//Healthbar erstellen

		HealthCircle.setFillColor(sf::Color::Blue);
		HealthCircle.setRadius(this->getRadius());
		HealthCircle.setOrigin(HealthCircle.getRadius(), HealthCircle.getRadius());
		HealthCircle.setPosition(this->getPosition());

		ComputeCollisionArea();
	}
};

class Button : public sf::RectangleShape
{
private:
	sf::FloatRect CollisionArea;
public:
	void ComputeCollisionArea()
	{
		this->CollisionArea = sf::FloatRect(this->getPosition().x, this->getPosition().y, this->getSize().x, this->getSize().y);
	}

	sf::FloatRect GetCollisionArea()
	{
		return CollisionArea;
	}

	bool IsClicked(sf::RenderWindow &Window, sf::Clock &ClickCooldown)
	{
		this->ComputeCollisionArea();

		if(this->CollisionArea.contains(sf::Vector2f(sf::Mouse::getPosition(Window).x, sf::Mouse::getPosition(Window).y)))
		{
			if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if(ClickCooldown.getElapsedTime().asMilliseconds() > 100)
				{	
					ClickCooldown.restart();
					return true;
				}
			}
		}
		return false;
	}

	Button()
	{
		this->setFillColor(sf::Color::Yellow);
		this->setSize(sf::Vector2f(90, 25));
		this->setPosition(sf::Vector2f(0, 0));
		ComputeCollisionArea();
	}

	Button(sf::Vector2f Position)
	{
		this->setFillColor(sf::Color::Yellow);
		this->setSize(sf::Vector2f(90, 25));
		this->setPosition(Position);
		ComputeCollisionArea();
	}
};

class PopUp : public sf::RectangleShape
{
private:
	sf::RenderWindow *Window;
	sf::Font *Font;
	Button UpgradeSpeed;
	Button UpgradeDamage;
public:
	void Update()
	{
		Window->draw(*this);
		Window->draw(UpgradeSpeed);

		sf::Text UpgradeSpeedTxt("Upgrade for ", *Font, 15); //Upgrade Speed Text
		UpgradeSpeedTxt.setPosition(UpgradeSpeed.getPosition().x+2, UpgradeSpeed.getPosition().y+3);
		UpgradeSpeedTxt.setString(UpgradeSpeedTxt.getString() + FloatToString(100));
		UpgradeSpeedTxt.setColor(sf::Color::Black);
		Window->draw(UpgradeSpeedTxt);

		Window->draw(UpgradeDamage);

		sf::Text UpgradeDamageTxt("Upgrade for ", *Font, 15); //Upgrade Damage Text
		UpgradeDamageTxt.setPosition(UpgradeDamage.getPosition().x+2, UpgradeDamage.getPosition().y+3);
		UpgradeDamageTxt.setString(UpgradeDamageTxt.getString() + FloatToString(100));
		UpgradeDamageTxt.setColor(sf::Color::Black);
		Window->draw(UpgradeDamageTxt);
	}

	PopUp(sf::Vector2f Position, sf::RenderWindow &Window, sf::Font &StdFnt)
	{
		this->Window = &Window;
		this->Font = &StdFnt;
		this->setPosition(Position);
		this->setSize(sf::Vector2f(150, 100));
		this->setFillColor(sf::Color::Red);
		this->setOutlineColor(sf::Color::Black);
		this->setOutlineThickness(2);
		UpgradeSpeed.setPosition(this->getPosition().x+5, this->getPosition().y+35);
		UpgradeDamage.setPosition(this->getPosition().x+5, this->getPosition().y+70);
	}
};

class Tower : public sf::Sprite
{
private:
	std::list<Bullet> *Bullets;
	std::list<Enemy> *Enemys;
	sf::Texture *BulletTexture;
	sf::RenderWindow *Window;
	float Range;
	float Firerate;
	float BulletDamage;
	bool Ready;
	bool Armed;
	sf::Clock Cooldown;
public:
	void SetArmed(bool Value)
	{
		this->Armed = Value;
	}

	float GetRange()
	{
		return this->Range;
	}

	float GetBulletDamage()
	{
		return this->BulletDamage;
	}

	void Update()
	{
		if(Armed)
		{
			if(Cooldown.getElapsedTime().asSeconds() >= 1 / Firerate)
			{
				Ready = true;
				Cooldown.restart();
			}

			for(std::list<Enemy>::iterator i = Enemys->begin(); i != Enemys->end(); i++)
			{
				Enemy *TmpEnemy = &*i;
				sf::Vector2f Direction = TmpEnemy->getPosition() - this->getPosition();

				if(sqrt(Direction.x * Direction.x + Direction.y * Direction.y) - Enemys->front().getRadius() <= this->Range)
				{
					this->setRotation(atan2(Direction.y, Direction.x) * 180 / PI); // In Richung drehen
					if(Ready)
					{
						Bullets->push_back(Bullet(*BulletTexture, this->getPosition(), sf::Vector2i(TmpEnemy->getPosition()), this->BulletDamage));
						Ready = false;
					}

					break;
				}
			}
		}
	}

	Tower(sf::Vector2f Position, std::list<Bullet> &Bullets, std::list<Enemy> &Enemys, sf::RenderWindow &Window, sf::Texture &Texture, sf::Texture &BulletTexture)
	{
		this->Bullets = &Bullets;
		this->Enemys = &Enemys;
		this->Window = &Window;
		this->BulletTexture = &BulletTexture;
		this->Range = DEFAULT_TOWER_RANGE;
		this->Ready = false;
		this->Armed = false;
		this->Firerate = STARTFIRERATE;
		this->BulletDamage = STARTDAMAGE;
		this->setTexture(Texture);
		this->setOrigin(Texture.getSize().x/2, Texture.getSize().y/2);
		this->setPosition(Position);
	}
};

class Healthbar : public sf::RectangleShape
{
private:
	sf::RectangleShape Inside;
public:
	sf::RectangleShape GetInside()
	{
		return Inside;
	}

	void SetValue(float Value)
	{
		Inside.setPosition(this->getPosition());
		Inside.setSize(sf::Vector2f(Value*(this->getSize().x/100), this->getSize().y));
		Inside.setFillColor(sf::Color::Green);
	}

	Healthbar(sf::Vector2f Position, sf::Vector2f Size)
	{
		this->setPosition(Position);
		this->setSize(Size);
		this->setFillColor(sf::Color::White);
		this->setOutlineColor(sf::Color::Black);
		this->setOutlineThickness(2);
	}
};

int main()
{
	// Systembasis erstellen
	sf::RenderWindow Window(sf::VideoMode(800, 600), "Defend2End");
	sf::Texture BackgroundImg;
	sf::Texture PlayerImg;
	sf::Texture BulletImg;

	// Spielrelevante Werte

	float Money = 150;
	float CurrentPlayerHealth = 100;
	float BulletDamage = STARTDAMAGE;
	float Level = 0;
	unsigned int WaveEnemys = 0;
	unsigned int EnemyCounter = 0;
	unsigned int KillCount = 0;
	bool WaveEnd = true;
	bool PlaceTowerMode = false;
	bool DEAD = false;
	sf::Clock EnemyCooldown;
	sf::Clock ClickCooldown;

	// Container erstellen

	std::list<Bullet> Bullets;
	std::list<Enemy> Enemys;
	std::list<Tower> Towers;
	

	// Bilder laden

	if(!BackgroundImg.loadFromFile(akilib::GetExecDirectory() + "Background.png"))
	{
		exit(0xDEAD);
	}
	if(!PlayerImg.loadFromFile(akilib::GetExecDirectory() + "Player.png"))
	{
		exit(0xDEAD);
	}
	if(!BulletImg.loadFromFile(akilib::GetExecDirectory() + "Bullet.png"))
	{
		exit(0xDEAD);
	}

	// Wichtige Variablen

	sf::Sprite Background(BackgroundImg);
	Player CurrentPlayer(PlayerImg);

	/////////////////Einmalige initialisierung z. B. Positionen///////////////////////////

	CurrentPlayer.setOrigin(PlayerImg.getSize().x / 2, PlayerImg.getSize().y / 2); //Spielermittelpunkt setzten
	CurrentPlayer.setPosition(Window.getSize().x/2, Window.getSize().y - 80); //Spielerposition setzten

	sf::Font StdFnt;

	if(!StdFnt.loadFromFile(akilib::GetExecDirectory() + "AGENCYR.ttf"))
	{
		exit(0xDEAD);
	}

	srand(time(NULL));

	//////////////////////////////////////////////////////////////////////////////////////

	Window.setFramerateLimit(60);

	while(Window.isOpen())
	{
		Window.clear(sf::Color::White);

		////////////////////Hauptspielablauf////////////////////////////////////////////

		sf::Event Event;

		while(Window.pollEvent(Event))
		{
			if(Event.type == sf::Event::Closed)
				Window.close();
		}

		//Überprüfen, ob die Bullet noch im Spielfeld ist

		sf::FloatRect GameArea(0, 0, Window.getSize().x, Window.getSize().y);
		
		for(std::list<Bullet>::iterator i = Bullets.begin(); i != Bullets.end(); i++)
		{
			Bullet *Tmp = &*i;
			if(!GameArea.contains(Tmp->getPosition()))
			{
				i = Bullets.erase(i);
			}
		}

		//Überprüfen ob gegner hinten durchgelaufen ist
		if(DEAD)
		{
			for(std::list<Tower>::iterator i = Towers.begin(); i != Towers.end(); i++)
			{
				Tower *TmpTower = &*i;
				TmpTower->SetArmed(false);
				
			}
		}
		else
		{
			for(std::list<Enemy>::iterator i = Enemys.begin(); i != Enemys.end(); i++)
			{
				Enemy *TmpEnemy = &*i;
				if(TmpEnemy->getPosition().y >= Window.getSize().y + TmpEnemy->getRadius())
				{
					if(TmpEnemy->GetIsBoss())
					{
						CurrentPlayerHealth -= 20;
					}
					else
					{
						CurrentPlayerHealth -= 10;
					}

					i = Enemys.erase(i);
					KillCount++;
				}
			}
		}

		//Überprüfen, ob die Bullets einen Gegner getroffen haben

		for(std::list<Bullet>::iterator i = Bullets.begin(); i != Bullets.end(); i++)
		{
			Bullet *TmpBullet = &*i;

			for(std::list<Enemy>::iterator j = Enemys.begin(); j != Enemys.end(); j++)
			{
				Enemy *TmpEnemy = &*j;

				if(TmpEnemy->GetCollisionArea().contains(TmpBullet->getPosition()))
				{
					TmpEnemy->DoDamage(TmpBullet->GetDamage());
					i = Bullets.erase(i);
					
					if(TmpEnemy->GetCurrentHP() <= 0)
					{
						Money += TmpEnemy->GetLoot();
						j = Enemys.erase(j);
						KillCount++;
					}
				}
			}
		}

		//Berechnungen

		//Wellenüberprüfung und Gegnererstellung

		if(EnemyCounter == WaveEnemys)
		{
			WaveEnd = true;
		}

		if(Enemys.size() == 0 && WaveEnd == true)
		{
			Level++;
			WaveEnemys = 20 + ((rand()%10)+1) + (rand()%(int)Level);
			WaveEnd = false;
			EnemyCounter = 0;
			KillCount = 0;
		}

		if(EnemyCounter < WaveEnemys)
		{
			if(EnemyCooldown.getElapsedTime().asSeconds() >= 1)
			{
				Enemys.push_back(Enemy(Level, sf::Vector2f((rand() % Window.getSize().x-20) + 20, 0)));
				EnemyCounter++;
				EnemyCooldown.restart();
			}
		}
		

		///////////////////////////////////////////////////////////////////

		CurrentPlayer.Update();

		for(std::list<Tower>::iterator i = Towers.begin(); i != Towers.end(); i++) //Tower updaten
		{
			Tower *Tmp = &*i;
			Tmp->Update();
		}

		for(std::list<Enemy>::iterator i = Enemys.begin(); i != Enemys.end(); i++) //Enemys updaten
		{
			Enemy *Tmp = &*i;
			Tmp->Update();
		}

		for(std::list<Bullet>::iterator i = Bullets.begin(); i != Bullets.end(); i++) //Bullets updaten
		{
			Bullet *Tmp = &*i;
			Tmp->Update();
		}

		sf::Vector2i Cursor = sf::Mouse::getPosition(Window);
		sf::Vector2f Direction = sf::Vector2f(Cursor.x, Cursor.y) - CurrentPlayer.getPosition();
		CurrentPlayer.setRotation(atan2(Direction.y, Direction.x) * 180 / PI);

		// Sprites Anzeigen

		Window.draw(Background); //Hinter- bzw. Untergrund zeichnen

		//Schießen und Buildings plazieren

		if(PlaceTowerMode)
		{
			Towers.back().setPosition(sf::Vector2f(sf::Mouse::getPosition(Window)));
			sf::CircleShape RangeCircle(Towers.back().GetRange());
			RangeCircle.setOrigin(RangeCircle.getRadius(), RangeCircle.getRadius());
			RangeCircle.setFillColor(sf::Color::Transparent);
			RangeCircle.setOutlineColor(sf::Color::Black);
			RangeCircle.setOutlineThickness(2);
			RangeCircle.setPosition(Towers.back().getPosition());
			Window.draw(RangeCircle);

			if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				if(ClickCooldown.getElapsedTime().asMilliseconds() >= 100)
				{
					PlaceTowerMode = false;
					Towers.back().SetArmed(true);
					ClickCooldown.restart();
				}
			}
		}

		if(GameArea.contains(sf::Vector2f(sf::Mouse::getPosition(Window).x, sf::Mouse::getPosition(Window).y)) && !DEAD)
		{
			if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if(CurrentPlayer.IsReady())
				{
					Bullets.push_back(Bullet(BulletImg, CurrentPlayer.getPosition(), sf::Mouse::getPosition(Window), BulletDamage));
					CurrentPlayer.SetReady(false);
				}
			}
		}
		


		// Zeichnen


		for(std::list<Bullet>::iterator i = Bullets.begin(); i != Bullets.end(); i++) //Bullets zeichnen
		{
			Window.draw(*i);
		}

		Window.draw(CurrentPlayer); //Spieler zeichnen

		for(std::list<Tower>::iterator i = Towers.begin(); i != Towers.end(); i++) //Towers zeichnen
		{
			Window.draw(*i);
		}

		for(std::list<Enemy>::iterator i = Enemys.begin(); i != Enemys.end(); i++) //Enemys zeichnen
		{
			Enemy *TmpEnemy = &*i;
			Window.draw(*i);
			Window.draw(TmpEnemy->GetHealthCircle());
		}

		if(CurrentPlayerHealth <= 0)
		{
			DEAD = true;
			sf::Text GameOverTxt("YOU ARE DEAD", StdFnt, 100); //tot
			GameOverTxt.setColor(sf::Color::Red);
			GameOverTxt.setPosition(180, 180);
			Window.draw(GameOverTxt);
		}

		//Menü anzeigen

		sf::Text MoneyTxt("Money: ", StdFnt, 20); //Geld
		MoneyTxt.setColor(sf::Color::Yellow);
		MoneyTxt.setPosition(20, 20);
		MoneyTxt.setString(MoneyTxt.getString() + FloatToString(Money));
		Window.draw(MoneyTxt);

		sf::Text LevelTxt("Level: ", StdFnt, 20); //Level
		LevelTxt.setPosition(Window.getSize().x-150, 20);
		LevelTxt.setString(LevelTxt.getString() + FloatToString(Level));
		Window.draw(LevelTxt);

		sf::Text RemainingEnemysTxt("Remaining Enemys: ", StdFnt, 20); //Verbleibende Gegner
		RemainingEnemysTxt.setPosition(Window.getSize().x-150, 40);
		RemainingEnemysTxt.setString(RemainingEnemysTxt.getString() + FloatToString(WaveEnemys-KillCount));
		Window.draw(RemainingEnemysTxt);

		sf::Text CopyrightTxt("Developed by AkiraSoul (Version 0.9)", StdFnt, 20); //Copyright
		CopyrightTxt.rotate(-90);
		CopyrightTxt.setPosition(10, Window.getSize().y - 10);
		Window.draw(CopyrightTxt);

		sf::Text BpS("Bullets / s: ", StdFnt, 20); //Feuerrate
		BpS.setPosition(180, Window.getSize().y - 30);
		BpS.setString(BpS.getString() + FloatToString(CurrentPlayer.GetFirerate()));
		Window.draw(BpS);

		sf::Text BulletDmgTxt("Damage / Bullet: ", StdFnt, 20); //Schaden pro Kugel
		BulletDmgTxt.setPosition(Window.getSize().x - 300, Window.getSize().y - 30);
		BulletDmgTxt.setString(BulletDmgTxt.getString() + FloatToString(BulletDamage));
		Window.draw(BulletDmgTxt);

		//Buttons anzeigen

		Button UpgradeFirerate(sf::Vector2f(200, Window.getSize().y - 60));
		Window.draw(UpgradeFirerate);
		sf::Text LabelUpgradeFirerate("Upgrade for ", StdFnt, 15); //Feuerrate upgraden
		LabelUpgradeFirerate.setPosition(UpgradeFirerate.getPosition().x+2, UpgradeFirerate.getPosition().y+3);
		LabelUpgradeFirerate.setString(LabelUpgradeFirerate.getString() + FloatToString(std::ceil(CurrentPlayer.GetFirerate()*25)));
		LabelUpgradeFirerate.setColor(sf::Color::Black);
		Window.draw(LabelUpgradeFirerate);
		
		if(UpgradeFirerate.IsClicked(Window, ClickCooldown))
		{
			if(Money >= std::ceil(CurrentPlayer.GetFirerate()*25))
			{
				Money -= std::ceil(CurrentPlayer.GetFirerate()*25);
				CurrentPlayer.SetFirerate(CurrentPlayer.GetFirerate() * 1.05);
			}
		}

		Button UpgradeBulletDamage(sf::Vector2f(Window.getSize().x - 310, Window.getSize().y - 60));
		Window.draw(UpgradeBulletDamage);
		sf::Text LabelUpgradeBulletDamage("Upgrade for ", StdFnt, 15); //Schaden pro Kugel upgraden
		LabelUpgradeBulletDamage.setPosition(UpgradeBulletDamage.getPosition().x+2, UpgradeBulletDamage.getPosition().y+3);
		LabelUpgradeBulletDamage.setString(LabelUpgradeBulletDamage.getString() + FloatToString(std::ceil(BulletDamage*50)));
		LabelUpgradeBulletDamage.setColor(sf::Color::Black);
		Window.draw(LabelUpgradeBulletDamage);

		if(UpgradeBulletDamage.IsClicked(Window, ClickCooldown))
		{
			if(Money >= std::ceil(BulletDamage*50))
			{
				Money -= std::ceil(BulletDamage*50);
				BulletDamage *= 1.05;
			}
		}

		Button BuildTower(sf::Vector2f(350, Window.getSize().y - 60)); //Turm bauen
		Window.draw(BuildTower);
		sf::Text BuildTowerTxt("Build Tower for ", StdFnt, 15);
		BuildTowerTxt.setPosition(BuildTower.getPosition().x+2, BuildTower.getPosition().y+3);
		BuildTowerTxt.setString(BuildTowerTxt.getString() + FloatToString(std::ceil(Towers.size() * 100 + 150 + Level * 1.25)));
		BuildTowerTxt.setColor(sf::Color::Black);
		Window.draw(BuildTowerTxt);

		if(BuildTower.IsClicked(Window, ClickCooldown))
		{
			if(Money >= std::ceil(Towers.size() * 100 + 150 + Level * 1.25))
			{
				Money -= std::ceil(Towers.size() * 100 + 150 + Level * 1.25);
				PlaceTowerMode = true;
				Towers.push_back(Tower(sf::Vector2f(sf::Mouse::getPosition(Window)), Bullets, Enemys, Window, PlayerImg, BulletImg));
			}
		}

		//HOTKEYS

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1) && ClickCooldown.getElapsedTime().asMilliseconds() >= 250) //Hotkey für Schussrate
		{
			if(Money >= std::ceil(CurrentPlayer.GetFirerate()*25))
			{
				Money -= std::ceil(CurrentPlayer.GetFirerate()*25);
				CurrentPlayer.SetFirerate(CurrentPlayer.GetFirerate() * 1.05);
			}
			ClickCooldown.restart();
		}

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2) && ClickCooldown.getElapsedTime().asMilliseconds() >= 250 && !PlaceTowerMode) //Hotkey für den Turmbau
		{
			if(Money >= std::ceil(Towers.size() * 100 + 150 + Level * 1.25))
			{
				Money -= std::ceil(Towers.size() * 100 + 150 + Level * 1.25);
				PlaceTowerMode = true;
				Towers.push_back(Tower(sf::Vector2f(sf::Mouse::getPosition(Window)), Bullets, Enemys, Window, PlayerImg, BulletImg));
			}
			ClickCooldown.restart();
		}

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3) && ClickCooldown.getElapsedTime().asMilliseconds() >= 250) //Hotkey für damage
		{
			if(Money >= std::ceil(BulletDamage*50))
			{
				Money -= std::ceil(BulletDamage*50);
				BulletDamage *= 1.05;
			}
			ClickCooldown.restart();
		}

		//Healthbar zeichnen

		Healthbar PlayerHealth(sf::Vector2f(Window.getSize().x/2 - 80, Window.getSize().y-25), sf::Vector2f(150, 20));
		PlayerHealth.SetValue(CurrentPlayerHealth);
		Window.draw(PlayerHealth);
		Window.draw(PlayerHealth.GetInside());

		////////////////////////////////////////////////////////////////////////////////

		Window.display();
	}
}