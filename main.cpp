# define PI 3.14159265358979323846
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <iostream>
#include <string>

using namespace std;

class Coord {

public:
	float x, y;
	
	// Random initialization is widely used, so for simplicity,
	// I added mode for random init.
	Coord(float x = 0, float y = 0, int mode = 0) {
		if (mode == 0) {
			this->x = x;
			this->y = y;
		}
		else if (mode == 1) {
			this->x = rand() % static_cast<int>(x);
			this->y = rand() % static_cast<int>(y);

		}
	}
};

enum WalkDirection {
	to_up = sf::Keyboard::Up,
	to_down = sf::Keyboard::Down,
	to_left = sf::Keyboard::Left,
	to_right = sf::Keyboard::Right,
	others,
};

// Base Class
class Object {
	sf::RenderWindow* window;
	sf::Texture texture;
	sf::Sprite sprite;
	Coord pos;

protected:
	string filename;

public:
	void init(sf::RenderWindow *window, string texturePath, Coord pos) {
		this->window = window;

		texturePath += filename;
		texture.loadFromFile(texturePath);

		sprite.setTexture(texture);
		this->pos = pos;
		sprite.setPosition(pos.x, pos.y);
	}

	sf::FloatRect getGlobalBounds() {
		return sprite.getGlobalBounds();
	}

	void paint() {
		sprite.setPosition(pos.x, pos.y);
		window->draw(sprite);
	}

};

class Barrel : public Object
{
	//To determine whether barrel is visible
	bool isVisible; 

public:
	Barrel() {
		Object::filename = "barrel.png";
		isVisible = true;
	}

	bool getVisible(void){
		return isVisible;
	} 
	void setVisible(bool visible) {
		isVisible = visible;
	}

};

class Sandbag: public Object{
public:
	Sandbag() {
		Object::filename = "bags.png";
	}
};

class Player
{
	// Inheritaning from Object class is very unnecessary since they have very little in common.

	sf::Texture textures[14];
	sf::Sprite sprite;
	sf::RenderWindow *window;
	Coord pos;
	int state;
	int s;
	int score;
		
public:
	void init(sf::RenderWindow *window, string textBasePath, int numTextures, Coord pos);
	void move(Coord pos, Barrel * barrels, Sandbag * sandbags, int nb, int ns);
	bool checkCollision(Barrel *barrels, Sandbag *sandbags, int nb, int ns);
	void walk(float speed, WalkDirection dir, Barrel *barrels, Sandbag *sandbags, int nb, int ns);
	void paint();

	// Required to detect initial collusion
	sf::Sprite& getSprite(){
		return sprite;
	};

	void incrementScore(void) {
		score += 1;
	}
	int getScore(void) {
		return score;
	}
	void setPosition(Coord pos) {
		state = 0;
		s = 1;
		this->pos = pos;
		sprite.setPosition(this->pos.x, this->pos.y);
	}
	Coord getPosition() {
		return this->pos;
	}
	int getState() {
		return this->state;
	}
};

void Player::init(sf::RenderWindow* window, string textBasePath, int numTextures, Coord pos)
{
	this->window = window;
	textBasePath += "soldier";
	this->pos = pos;

	for (int i = 0; i < 14; i++) 
		textures[i].loadFromFile(textBasePath + to_string(i) + ".png");
	

	score = 0;
	state = 0;
	s = 1;

	sprite.setTexture(textures[state]);
	sprite.setPosition(this->pos.x, this->pos.y);
}

void Player::move(Coord d, Barrel* barrels, Sandbag* sandbags, int nb, int ns)
{
	// If the collision occurs after the move, it will be taken back before drawing
	this->pos.x += d.x;
	this->pos.y += d.y;
	sprite.setPosition(this->pos.x, this->pos.y);

	if (checkCollision(barrels, sandbags, nb, ns)) {
		this->pos.x -= d.x;
		this->pos.y -= d.y;

		sprite.setPosition(pos.x, pos.y);
	}
}

bool Player::checkCollision(Barrel* barrels, Sandbag* sandbags, int nb, int ns)
{
	// Since defualt collusion detection function is not aware of the texture itself,
	// I did my own function, and reduced the rectangle of player by s_p pixel.

	bool collusion = false;
	float s_p = 25.f;

	sf::FloatRect r_player = sprite.getGlobalBounds();

	r_player.left += s_p;
	r_player.top += s_p;
	r_player.width -= 2 * s_p;
	r_player.height -= 2 * s_p;

	for (size_t i = 0; i < nb; i++)
	{
		// Skip the barrel if it is already hit
		if (!barrels[i].getVisible())
			continue;

		sf::FloatRect bar = barrels[i].getGlobalBounds();

		// Emprically I change the rectangle of the objects to get more realistic occlusion detector
		sf::FloatRect bound(bar.left + 10.f, bar.top, bar.width - 10.f, -45.f + bar.height);

		collusion = r_player.intersects(bound);

		if (collusion) {
			cout << "Collision Detected" << endl;
			return true;
		}
	}


	for (size_t i = 0; i < ns; i++)
	{
		sf::FloatRect bag = sandbags[i].getGlobalBounds();

		// Emprically I change the rectangle of the objects to get more realistic occlusion detector
		sf::FloatRect bound(bag.left, bag.top, bag.width, -40.f + bag.height);

		collusion = r_player.intersects(bound);

		if (collusion) {
			cout << "Collision Detected" << endl;
			return true;
		}
	}

	return false;
}

void Player::walk(float speed, WalkDirection dir, Barrel* barrels, Sandbag* sandbags, int nb, int ns)
{
	const float speedPlayerCoef = 5.f;

	switch (state)
	{
	case 0:

		if (dir == to_right)
			state = 1;

		else if (dir == to_left) {
			state = 7;
			s = 1;
		}
		else if (dir == to_up && s == 1) {
			// up
			state = 8;
			s = 0;
			move(Coord(0, -speedPlayerCoef * speed), barrels, sandbags, nb, ns);
		}
		else if (dir == to_up && s == 0) {
			// up
			state = 7;
			s = 1;
			move(Coord(0, -speedPlayerCoef * speed), barrels, sandbags, nb, ns);
		}

		break;

	case 1:

		if (dir == to_right)
			state = 2;

		else if (dir == to_left)
			state = 0;

		break;

	case 2:

		if (dir == to_up && s == 0) {
			s = 1;
			state = 10;
			// right
			move(Coord(speedPlayerCoef*speed, 0), barrels, sandbags, nb, ns);

		}
		else if (dir == to_up && s == 1) {
			s = 0;
			state = 9;
			// right
			move(Coord(speedPlayerCoef*speed, 0), barrels, sandbags, nb, ns);
		}
		else if (dir == to_left) {
			state = 1;
		}
		else if (dir == to_right) {
			state = 3;
			s = 1;
		}

		break;

	case 3:

		if (dir == to_left) {
			state = 2;
		}
		else if (dir == to_up || dir == to_right) {
			state = 4;
			// down
			//move(Coord(0 ,step*speed));	// ???
		}

		break;

	case 4:

		if (dir == to_up && s == 0) {
			s = 1;
			state = 3;
			// down
			move(Coord(0, speedPlayerCoef*speed), barrels, sandbags, nb, ns);
		}
		else if (dir == to_up && s == 1) {
			state = 11;
			s = 0;
			// down
			move(Coord(0, speedPlayerCoef*speed), barrels, sandbags, nb, ns);
		}
		else if (dir == to_right) {
			state = 5;
		}
		else if (dir == to_left) {
			state = 3;
			s = 1;
		}
		break;

	case 5:

		if (dir == to_right)
			state = 6;

		else if (dir == to_left)
			state = 4;

		break;

	case 6:

		if (dir == to_right) {
			state = 7;
			s = 1;
		}
		else if (dir == to_up && s == 0) {
			state = 13;
			s = 1;
			// left
			move(Coord(-speedPlayerCoef * speed, 0), barrels, sandbags, nb, ns);

		}
		else if (dir == to_up && s == 1) {
			state = 12;
			s = 0;
			//left
			move(Coord(-speedPlayerCoef * speed, 0), barrels, sandbags, nb, ns);
		}
		else if (dir == to_left)
			state = 5;

		break;

	case 7:

		if (dir == to_up || dir == to_right)
			state = 0;

		else if (dir == to_left)
			state = 6;

		break;


	case 8:	 state = 0;	break;

	case 9:	 state = 2;	break;

	case 10: state = 2;	break;

	case 11: state = 4;	break;

	case 12: state = 6;	break;

	case 13: state = 6;	break;

	default:
		cout << "Unknown State" << endl;
		break;
	}

}

void Player::paint()
{
	sprite.setTexture(textures[state]);
	window->draw(sprite);
}

class Bullet {

	float speed;
	float angle;
	Coord pos;
	sf::Texture textures;
	sf::Sprite sprite;
	
	// Bullet id to detect which player shoot it. This could be good for scalibility of the game.
	int id;

	// This angles are emprically acquired by using Paint3D program
	int state_to_angle[14] = { 90, 66, 0, 287, 270, 246, 180, 107, 90, 0, 17, 270, 180, 199 };
	
	// To mark the bullets that will be removed in the next update
	bool removeIt;

public:
	Bullet* next = nullptr;

	Bullet(Coord pos, string& path, int state, int id, float speed) {

		const float speedBulletCoef = 1.f;

		removeIt = false;

		this->id = id;
		this->pos = pos;
		this->speed = speed * speedBulletCoef;
		this->angle = state_to_angle[state];

		textures.loadFromFile(path + "bullet.png");
		sprite.setTexture(textures);
		sprite.setRotation(90-angle);

	}
	void move(float delta) {
		
		pos.x += speed * cos(angle * PI / 180);
		pos.y -= speed * sin(angle * PI / 180);
		sprite.setPosition(sf::Vector2f(pos.x, pos.y));
	}

	void setSpeed(float speed) { this->speed = speed; }

	sf::Sprite getSprite() { return this->sprite; }

	bool getRemoveStatus() { return removeIt; }

	void setRemoveStatus(bool status) { removeIt = status; }

	int getBulletId() { return id; }
};

class BulletList {
	Bullet* list;
	sf::RenderWindow * window;
	string path;

public:

	BulletList(sf::RenderWindow* window, string path) {
		this->window = window;
		this->path = path;
		list = nullptr;
	}
	  
	void add(Coord pos, int state, int id, float speed) {
		Bullet *ptr = new Bullet(pos, path, state, id, speed);
		ptr->next = list;
		list = ptr;
		cout << "Bullet added" << endl;
	}

	// Call this function before the update so that 
	// it deletes the all bullets that are pointed as removeIt=true
	void remove() {

		// Just to print out total number of bullet in the list
		// This 8-line of code is unnecessary.
		int i=0;
		Bullet *temp = list;
		while (temp!=nullptr)
		{
			i=i+1;
			temp = temp->next;
		}
		cout << "Total number of Bullet: "<<i << endl;

		// Skip the first node
		Bullet *prevptr = list;
		Bullet *ptr = list->next;

		while (ptr != nullptr)
		{

			if (ptr->getRemoveStatus()) {
				prevptr->next = ptr->next;
				delete ptr;
				ptr = prevptr->next;
				cout << "Bullet deleted" << endl;
			}
			else {
				prevptr = ptr;
				ptr = ptr->next;
			}

		}

		// To check first node
		ptr = list;

		if (ptr->getRemoveStatus()) {
			list = ptr->next;
			delete ptr;
			cout << "Bullet deleted" << endl;
		}
		
	}

	void updateAndPaint(float delta) {
		Bullet* ptr=list;
		
		while (ptr != nullptr) {

			// Update its position
			ptr->move(delta);

			// To remove the bullets that are out of screen
			int bx = ptr->getSprite().getPosition().x;
			int by = ptr->getSprite().getPosition().y;

			if (bx > window->getSize().x + 10 || by > window->getSize().y + 10 || bx < -10 || by < -10) {
				ptr->setRemoveStatus(true);
			}

			// Draw
			window->draw(ptr->getSprite());

			ptr = ptr->next;

		}

	}

	// This is similar to the Player::checkCollision() function.
	void checkCollision(Player* players, Barrel* barrels, Sandbag* sandbags, int np, int nb, int ns, int w, int h) {

		Bullet* ptr = list;

		while (ptr->next != nullptr) {

			for (size_t i = 0; i < nb; i++)
			{
				// Skip the barrel if it is already hit
				if (!barrels[i].getVisible())
					continue;

				sf::FloatRect bar = barrels[i].getGlobalBounds();

				// Emprically I change the rectangle of the objects to get more realistic occlusion detector
				sf::FloatRect bound(bar.left + 10.f, bar.top, bar.width - 10.f, -45.f + bar.height);

				if (ptr->getSprite().getGlobalBounds().intersects(bound) == true) {
					cout << "Bullet hit the barrel" << endl;
					barrels[i].setVisible(false);

					ptr->setRemoveStatus(true);
				}

			}

			for (size_t i = 0; i < ns; i++)
			{
				sf::FloatRect bag = sandbags[i].getGlobalBounds();

				// Emprically I change the rectangle of the objects to get more realistic occlusion detector
				sf::FloatRect bound(bag.left, bag.top, bag.width, -40.f + bag.height);

				if (ptr->getSprite().getGlobalBounds().intersects(bound) == true) {

					ptr->setRemoveStatus(true);

					cout << "Bullet hit the sandbag" << endl;
				}
			}

			for (size_t i = 0; i < np; i++)
			{
				sf::FloatRect pl = players[i].getSprite().getGlobalBounds();

				// Emprically I change the rectangle of the objects to get more realistic occlusion detection
				sf::FloatRect bound(pl.left+25.f, pl.top+25.f, pl.width-50.f, pl.height - 50.f);

				if (ptr->getSprite().getGlobalBounds().intersects(bound) == true) {
					cout << "Bullet hit the player "<< i+1 << endl;
					
					// First player shoot it
					if ((ptr->getBulletId() == 0) && (i != 0)) {
						players[0].incrementScore();

						//Spawn other player
						players[1].setPosition(Coord(w - 50, h - 50, 1));
						while (players[1].checkCollision(barrels, sandbags, nb, ns)) {
							players[1].setPosition(Coord(w - 50, h - 50, 1));
						}

						ptr->setRemoveStatus(true);
					}

					// Second player shoot it
					else if ((ptr->getBulletId() == 1) && (i != 1)) {
						players[1].incrementScore();
						 
						//Spawn other player
						players[0].setPosition(Coord(w - 50, h - 50, 1));
						while (players[0].checkCollision(barrels, sandbags, nb, ns)) {
							players[0].setPosition(Coord(w - 50, h - 50, 1));
						}

						ptr->setRemoveStatus(true);
					}
				}
			}

			ptr = ptr->next;
		}


	}
	~BulletList(){
		Bullet *ptr = list;
		Bullet *prevptr = nullptr;

		if (list == nullptr)
			return;

		while (ptr->next != nullptr)
		{
			prevptr = ptr;
			ptr = ptr->next;

			delete prevptr;
			list = ptr;
		}
		delete ptr;

		list = nullptr;

	}

	bool isNotNull() {
		if (list == nullptr) 
			return false;
		else
			return true;
	}
};

class Game
{
	// Since setFramerateLimit is not perfect, I used clock to make movements more smooth.
	// Such that time between consecutive frames effects the amount of pixel to move.
	sf::Clock clock;
	float delta = 0.05;

	float speed;
	int numBarrels;
	int numSandbags;
	int numPlayers;

	// This is will be the main window of the game so to use isOpen and pollEvent functions,
	// I added running() and eventPolling() functions to keep the main function simple and clean
	sf::RenderWindow* window;

	// One main path to the image file, specific filenames are inside the corresponded classes.
	string texturePath;

	sf::Texture bgTexture;
	sf::Sprite bgSprite;
	Barrel *barrels;
	Sandbag *sandbags;
	Player* players;
	sf::Event ev;

	int height;
	int width;

	BulletList *bullets;
	sf::Text text;
	sf::Font font;

	short int first_v, second_v, first_h, second_h;
	bool first_shoot, second_shoot;

	// To restart game if asked
	bool gameActive;

public:
	Game(float speed, int w, int h, int nb, int ns, int np, string &path);
	~Game();

	void drawBackground(void);
	void update();

	// Check whether window is closed
	bool running();

	// Handles the events
	void eventPolling();

	bool getActive() {
		return gameActive;
	}

};

Game::Game(float speed, int w, int h, int nb, int ns, int np, string &path)
{
	this->speed = speed;
	this->numBarrels = nb;
	this->numSandbags = ns;
	this->numPlayers = np;
	this->width = w;
	this->height = h;

	// This is the main path
	texturePath = path;

	// Rescaling the window was removed.
	window = new sf::RenderWindow(sf::VideoMode(width, height), "Half Life 2 REMASTERED", sf::Style::Close | sf::Style::Titlebar);

	window->setFramerateLimit(30);

	barrels = new Barrel[numBarrels];
	sandbags = new Sandbag[numSandbags];
	players = new Player[numPlayers];

	// I substracted 50 pixel from width and height, to make the object completely inside the window
	for (size_t i = 0; i < numBarrels; i++)
		barrels[i].init(this->window, texturePath, Coord(width - 50, height - 50, 1));

	for (size_t i = 0; i < numSandbags; i++)
		sandbags[i].init(this->window, texturePath, Coord(width - 50, height - 50, 1));


	for (size_t i = 0; i < numPlayers; i++) {

		players[i].init(this->window, texturePath, 14, Coord(width - 50, height - 50, 1));

		// Randomly resets the location of players until it finds empty place
		while (players[i].checkCollision(barrels, sandbags, nb, ns))
		{
			players[i].setPosition(Coord(width - 50, height - 50, 1));
		}

	}

	bullets=new BulletList(this->window, texturePath);

	first_v = 0;
	second_v = 0;
	first_h = 0;
	second_h = 0;
	first_shoot = false;
	second_shoot = false;

	font.loadFromFile(texturePath + "font.ttf");
	text.setFont(font);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color(220,220,220));

	gameActive = true;

	cout << "Game objects successfully initilized" << endl;
}

Game::~Game()
{
	delete window;
	delete[] players;
	delete[] barrels;
	delete[] sandbags;
	delete bullets;
	cout << "Game object is destructed" << endl;
}

void Game::drawBackground(void)
{
	bgTexture.loadFromFile(texturePath + "grass.png");

	// Instead of scaling the background to the size of window, I used setRepeated function
	bgTexture.setRepeated(true);

	bgSprite.setTexture(bgTexture);
	bgSprite.setTextureRect(sf::IntRect(0, 0, this->width, this->height));

	window->draw(bgSprite);
}

void Game::update()
{

	delta = clock.restart().asSeconds();

	// Clear the previous screen
	window->clear();

	// Draw the background
	drawBackground();

	// Draw all barrels
	for (size_t i = 0; i < numBarrels; i++) {
		if (barrels[i].getVisible())
			barrels[i].paint();
	}

	// Draw all sandbags
	for (size_t i = 0; i < numSandbags; i++)
		sandbags[i].paint();


	// Draw Bullets
	if (bullets->isNotNull()) {

		bullets->updateAndPaint(delta);

		// Controls the visibility of barrels, marks the bullets that will be removed, calculates the scores
		bullets->checkCollision(players, barrels, sandbags, numPlayers, numBarrels, numSandbags, width, height);

		// Removes the marked bullets
		bullets->remove();
	}

	// Draw all players
	for (size_t i = 0; i < numPlayers; i++)
		players[i].paint();


	// Check whether the game is over or not
	if (players[0].getScore() >= 10 || players[1].getScore() >= 10) {

		window->clear(sf::Color::Black);

		text.setScale(2.0, 2.0);

		string winner = (players[0].getScore() > players[1].getScore()) ? "First" : "Second";

		text.setString(winner + " Player won!\n\n(ESC) Exit \n(R) Restart the game");

		text.setOrigin(text.getGlobalBounds().width/4.0, text.getGlobalBounds().height / 4.0);
		text.setPosition(window->getSize().x / 2, window->getSize().y / 2);
		window->draw(text);

		window->display();
		
		if (window->waitEvent(ev)) {
			if (ev.type == sf::Event::KeyPressed) {

				if (ev.key.code == sf::Keyboard::Escape) {
					gameActive = false;
					window->close();
				}
				else if (ev.key.code == sf::Keyboard::R) {
					window->close();
				}
			}
		}
	}
	else {
		text.setString("Score:\n1st Player: "+to_string(players[0].getScore()) + "\n2nd Player: " + to_string(players[1].getScore()));
		text.setOrigin(text.getGlobalBounds().width / 2.0, text.getGlobalBounds().height / 2.0);
		text.setPosition(window->getSize().x / 2, window->getSize().y-60);
		window->draw(text);
		window->display();
	}

}

bool Game::running()
{
	return window->isOpen();
}

void Game::eventPolling()
{
	WalkDirection dir;
	float cx, cy;

	//	// Taking the input	//	//

	while (window->pollEvent(ev)) {
		
		if (ev.type == sf::Event::Closed) {
			gameActive = false;
			window->close();
		}
		// KEY PRESSED
		if (ev.type == sf::Event::KeyPressed) {

			if (ev.key.code == sf::Keyboard::Escape) {
				gameActive = false;
				window->close();
			}

			switch (ev.key.code){

				// Player 1
				case sf::Keyboard::Enter:	first_shoot = true;	break;
				
				case sf::Keyboard::Up:		first_v = 1;	break;
				
				case sf::Keyboard::Right:	first_h = 1;	break;
				
				case sf::Keyboard::Down:	first_v = -1;	break;
				
				case sf::Keyboard::Left:	first_h = -1;	break;
				
				// Player 2
				case sf::Keyboard::Space:	second_shoot = true;	break;

				case sf::Keyboard::W:	second_v = 1;	break;

				case sf::Keyboard::A:	second_h = -1;	break;

				case sf::Keyboard::S:	second_v = -1;	break;

				case sf::Keyboard::D:	second_h = 1;	break;
				
				default:
					break;
				}

		}
		// KEY RELEASED
		if (ev.type == sf::Event::KeyReleased) {

			switch (ev.key.code) {

			// Player 1
			case sf::Keyboard::Enter:	first_shoot = false;	break;

			case sf::Keyboard::Up:		first_v = 0;	break;

			case sf::Keyboard::Right:	first_h = 0;	break;

			case sf::Keyboard::Down:	first_v = 0;	break;

			case sf::Keyboard::Left:	first_h = 0;	break;

			// Player 2
			case sf::Keyboard::Space:	second_shoot = false;	break;

			case sf::Keyboard::W:	second_v = 0;	break;

			case sf::Keyboard::D:	second_h = 0;	break;

			case sf::Keyboard::S:	second_v = 0;	break;

			case sf::Keyboard::A:	second_h = 0;	break;

			default:
				break;
			}

		}

	}

	//	// Processing the input	//	//

	// Although frame rate is constant, event polling is continuosly working.
	// To prevent that I added a short delay
	// Since I used delta to managed the speed. I believe this delay becomes negligible.
	sf::sleep(sf::milliseconds(50));


	// Player 1

	if (first_shoot) {
		cx = players[0].getPosition().x + players[0].getSprite().getGlobalBounds().width / 2.0;
		cy = players[0].getPosition().y + players[0].getSprite().getGlobalBounds().height / 2.0;
		bullets->add(Coord(cx, cy), players[0].getState(), 0, speed);
	}

	if (first_v==1) 
		players[0].walk(this->speed*delta, to_up, barrels, sandbags, numBarrels, numSandbags);
	
	if (first_v == -1) 
		players[0].walk(this->speed*delta, to_down, barrels, sandbags, numBarrels, numSandbags);
	
	if (first_h == -1) 
		players[0].walk(this->speed*delta, to_left, barrels, sandbags, numBarrels, numSandbags);
	
	if (first_h == 1) 
		players[0].walk(this->speed*delta, to_right, barrels, sandbags, numBarrels, numSandbags);
	

	// Player 2
	if (second_shoot) {
		cx = players[1].getPosition().x + players[1].getSprite().getGlobalBounds().width / 2.0;
		cy = players[1].getPosition().y + players[1].getSprite().getGlobalBounds().height / 2.0;
		bullets->add(Coord(cx, cy), players[1].getState(), 1, speed);
	}

	if (second_v == 1) 
		players[1].walk(this->speed*delta, to_up, barrels, sandbags, numBarrels, numSandbags);
	
	if (second_v == -1) 
		players[1].walk(this->speed*delta, to_down, barrels, sandbags, numBarrels, numSandbags);
	
	if (second_h == -1) 
		players[1].walk(this->speed*delta, to_left, barrels, sandbags, numBarrels, numSandbags);
	
	if (second_h == 1)
		players[1].walk(this->speed*delta, to_right, barrels, sandbags, numBarrels, numSandbags);

}

int main()
{
	cout << "Loading..." << endl;

	int w = 1200, h = 800, nb = 8, ns = 8, np = 2;

	// Path for all files including font and images
	string path = R"(textures\)";

	// As pixel per second
	float speed = 50;

	// To check whether the game will be restarted or not
	bool active = true;
	
	Game *gm = new Game(speed, w, h, nb, ns, np, path);

	while (active){

		while (gm->running()) // check whether window.isOpen() 
		{
			// Contains the event loop
			gm->eventPolling();

			// Clears the window, draws the background, renders barrels, sandbags and players. 
			gm->update();	

			// If true, delete the game object and restarts the game
			active = gm->getActive();
		}

		if (active) {
			cout << "Restarting..." << endl;
			delete gm;
			Game *gm = new Game(speed, w, h, nb, ns, np, path);
		}

	}

	delete gm;
	cout << "\nGAME OVER" << endl;

	return 0;
}