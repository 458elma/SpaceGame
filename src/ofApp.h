/**

	Author: Elston Ma
	CS134
	Project 1

*/
#pragma once

#include "ofMain.h"
#include "ofxGui.h"

typedef enum { MoveStop, MoveLeft, MoveRight, MoveUp, MoveDown } MoveDir;

// This is a base object that all drawable object inherit from
// It is possible this will be replaced by ofNode when we move to 3D
//
class BaseObject {
public:
	BaseObject();
	glm::vec3 trans, scale;
	float	rot;
	bool	bSelected;
	void setPosition(glm::vec3);

	// matrix to help with movement of object
	glm::mat4 getMatrix() {
		glm::mat4 translation = glm::translate(glm::mat4(1.0), glm::vec3(trans));
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(rot), glm::vec3(0, 0, 1));
		glm::mat4 scaling = glm::scale(glm::mat4(1.0), this->scale);

		return (translation * rotation * scaling);
	}
};

//  General Sprite class  (similar to a Particle)
//
class Sprite : public BaseObject {
public:
	Sprite();
	void draw();
	float age();
	void setImage(ofImage);
	float speed;    //   in pixels/sec
	glm::vec3 velocity; // in pixels/sec
	ofImage image;
	float birthtime; // elapsed time in ms
	float lifespan;  //  time in ms
	string name;
	bool haveImage;
	float width, height;
};

//  Manages all Sprites in a system.  You can create multiple systems
//
class SpriteSystem {
public:
	void add(Sprite);
	void remove(int);
	void update();
	void setBoom(ofSoundPlayer);
	int removeNear(glm::vec3 point, float dist);
	void draw();
	vector<Sprite> sprites;
	ofSoundPlayer boomSound;
	bool hasBoom = false;
};


//  General purpose Emitter class for emitting sprites
//  This works similar to a Particle emitter
//
class Emitter : public BaseObject {
public:
	Emitter(SpriteSystem *);
	void draw();
	void start();
	void stop();
	void setLifespan(float);
	void setVelocity(glm::vec3);
	void setChildImage(ofImage);
	void setChildSize(float w, float h) { childWidth = w; childHeight = h; }
	void setImage(ofImage);
	void setRate(float);
	void setFiringDir(float);
	void setFiringMat(float);
	void setEmitterMat(float);
	void setFireSound(ofSoundPlayer);
	void update();
	void integrate();

	glm::vec3 moveVelocity;
	glm::vec3 moveAcceleration;
	glm::vec3 moveForces;
	float moveRotVel;
	float moveRotAcc;
	float moveRotForces;
	float moveDamping;

	SpriteSystem *sys;
	float rate;
	float firingDir;
	glm::mat4 rotDir;
	glm::vec3 velocity;
	float lifespan;
	bool started;
	float lastSpawned;
	ofImage childImage;
	ofImage image;
	bool drawable;
	bool haveChildImage;
	bool haveImage;
	float width, height;
	float childWidth, childHeight;
	glm::mat4 emitterRot;
	ofSoundPlayer fireSound;
	bool playFireSound;
	bool hasSound;
};

// Particle class for explosion particles
class Particle : public BaseObject {
public:
	Particle(glm::vec3 dot);
	void draw();
	void integrate();

	glm::vec3 debrisVel;
	glm::vec3 debrisAccel;
	glm::vec3 debrisForces;
	float debrisDamping;
};

// actual class for explosion
class Explosion : public BaseObject {
public:
	Explosion(glm::vec3 boomSite, int pts, float life, float power, int dust);
	void draw();
	float age();
	void update();

	float lifespan;
	float birthtime;
	int debrisCount;
	int points;
	vector<Particle> particles;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void checkCollisions();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		// Explosion stuff
		void addBoom(glm::vec3 boomPos, int thePts);
		void removeBoom();
		vector<Explosion> booms;

		//--------------------
		Emitter* projectiles;
		int score;

		// invaders set up
		Emitter* invaders1;
		Emitter* invaders2;
		Emitter* invaders3;
		Emitter* invaders4;
		Emitter* invaderS;
		ofImage invaderImage;
		bool invaderLoaded;
		ofImage specInvImage;
		bool specInvLoaded;

		// needed variables to help with prediction of where
		// turret will travel in order to keep it in bounds
		//glm::vec3 predictionUp;
		//glm::vec3 predictionDown;
		//glm::vec3 predictionLeft;
		//glm::vec3 predictionRight;

		ofImage defaultImage;
		ofImage turretImage;
		glm::vec3 mouse_last;
		bool imageLoaded;

		ofSoundPlayer firingSound;
		bool soundLoaded = false;
		ofSoundPlayer invaderBoom;
		bool invBoomLoaded = false;

		ofImage bkgImg;
		bool validBkg = false;

		bool bHide;

		//ofxFloatSlider rate;
		//ofxFloatSlider fDir;
		ofxFloatSlider lifespan1;
		ofxFloatSlider lifespan2;
		ofxFloatSlider lifespan3;
		ofxFloatSlider lifespan4;
		ofxFloatSlider lifespanS;
		//ofxFloatSlider rate1;
		//ofxFloatSlider rate2;
		//ofxFloatSlider rate3;
		//ofxFloatSlider rate4;
		//ofxFloatSlider rateS;
		ofxFloatSlider shipThrust;
		ofxFloatSlider boomPower;
		ofxFloatSlider boomLife;
		ofxIntSlider boomDust;
		ofxLabel screenSize;

		ofxPanel gui;

		// scoring text
		ofTrueTypeFont scoreBoard;

		// game start check and text
		bool gameStarted = false;
		ofTrueTypeFont gameStartText;
};
