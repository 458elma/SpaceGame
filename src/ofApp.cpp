/**

	Author: Elston Ma
	CS134
	Project 1

*/
#include "ofApp.h"
#define MOVEMENT_SPEED 1000
#define ROT_SPEED 10
#define FIRING_SPEED -1000
#define LIFE 4000
#define FIRERATE 20

BaseObject::BaseObject() {
	trans = glm::vec3(0, 0, 1);
	scale = glm::vec3(1, 1, 1);
	rot = 0;
}

void BaseObject::setPosition(glm::vec3 pos) {
	trans = pos;
}

//
// Basic Sprite Object
//
Sprite::Sprite() {
	speed = 0;
	velocity = glm::vec3(0, 0, 0);
	lifespan = -1;      // lifespan of -1 => immortal 
	birthtime = 0;
	bSelected = false;
	haveImage = false;
	name = "UnamedSprite";
	width = 60;
	height = 80;
}

// Return a sprite's age in milliseconds
//
float Sprite::age() {
	return (ofGetElapsedTimeMillis() - birthtime);
}

//  Set an image for the sprite. If you don't set one, a rectangle
//  gets drawn.
//
void Sprite::setImage(ofImage img) {
	image = img;
	haveImage = true;
	width = image.getWidth();
	height = image.getHeight();
}


//  Render the sprite
//
void Sprite::draw() {

	ofSetColor(255, 255, 255, 255);

	// draw image centered and add in translation amount
	//
	if (haveImage) {
		image.draw(-width / 2.0 + trans.x, -height / 2.0 + trans.y);
	}
	else {
		// in case no image is supplied, draw something.
		// 
		ofSetColor(255, 0, 0);
		ofDrawRectangle(-width / 2.0 + trans.x, -height / 2.0 + trans.y, width, height);
	}
}

//  Add a Sprite to the Sprite System
//
void SpriteSystem::add(Sprite s) {
	sprites.push_back(s);
}

// Remove a sprite from the sprite system. Note that this function is not currently
// used. The typical case is that sprites automatically get removed when the reach
// their lifespan.
//
void SpriteSystem::remove(int i) {
	sprites.erase(sprites.begin() + i);
}

// set the collision sound loaded to true and load the collision sound
void SpriteSystem::setBoom(ofSoundPlayer theBoom) {
	hasBoom = true;
	boomSound = theBoom;
}

// remove sprites at a given distance from point
// return number removed
int SpriteSystem::removeNear(glm::vec3 point, float dist) {
	vector<Sprite>::iterator s = sprites.begin();
	vector<Sprite>::iterator tmp;
	int count = 0;

	while (s != sprites.end()) {
		glm::vec3 v = s->trans - point;
		if (glm::length(v) < dist) {
			tmp = sprites.erase(s);
			if (hasBoom) boomSound.play();
			count++;
			s = tmp;
		} else {
			s++;
		}
	}
	return count;
}

//  Update the SpriteSystem by checking which sprites have exceeded their
//  lifespan (and deleting).  Also the sprite is moved to it's next
//  location based on velocity and direction.
//
void SpriteSystem::update() {

	if (sprites.size() == 0) return;
	vector<Sprite>::iterator s = sprites.begin();
	vector<Sprite>::iterator tmp;

	// check which sprites have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, use an iterator.
	//
	while (s != sprites.end()) {
		if (s->lifespan != -1 && s->age() > s->lifespan) {
			//			cout << "deleting sprite: " << s->name << endl;
			tmp = sprites.erase(s);
			s = tmp;
		}
		else s++;
	}

	//  Move sprite
	//
	for (int i = 0; i < sprites.size(); i++) {
		sprites[i].trans += sprites[i].velocity / ofGetFrameRate();
	}
}

//  Render all the sprites
//
void SpriteSystem::draw() {
	for (int i = 0; i < sprites.size(); i++) {
		sprites[i].draw();
	}
}

//  Create a new Emitter - needs a SpriteSystem
//
Emitter::Emitter(SpriteSystem *spriteSys) {
	sys = spriteSys;
	lifespan = LIFE;    // milliseconds
	started = false;

	lastSpawned = 0;
	rate = 1;    // sprites/sec
	haveChildImage = false;
	haveImage = false;
	firingDir = 0;
	// store the matrix to use for rotating firing direction
	// multiplied to velocity
	rotDir = glm::rotate(glm::mat4(1.0), glm::radians(firingDir), glm::vec3(0, 0, 1));
	velocity = glm::vec4(glm::vec3(0, FIRING_SPEED, 1), 1);
	// set initial turret travel direction matrix using initial rotation amount
	emitterRot = glm::rotate(glm::mat4(1.0), glm::radians(rot), glm::vec3(0, 0, 1));
	drawable = true;
	width = 50;
	height = 50;
	childWidth = 60;
	childHeight = 80;
	bSelected = false;
	playFireSound = false;
	hasSound = false;

	// initialize integrator vectors and values
	moveVelocity = glm::vec3(0, 0, 0);
	moveAcceleration = glm::vec3(0, 0, 0);
	moveForces = glm::vec3(0, 0, 0);
	moveRotVel = 0.0;
	moveRotAcc = 0.0;
	moveRotForces = 0.0;
	moveDamping = 0.99;
}

//  Draw the Emitter if it is drawable. In many cases you would want a hidden emitter
//
//
void Emitter::draw() {
	// draw sprite system
	//
	sys->draw();

	if (drawable) {

		if (haveImage) {
			// make sure image of emitter can be tranformed with use of
			// matrix manipulation
			ofPushMatrix();
			ofMultMatrix(getMatrix());
			image.draw(-image.getWidth() / 2.0, -image.getHeight() / 2.0);
			ofPopMatrix();
		}
		else {
			ofSetColor(0, 0, 200);
			ofPushMatrix();
			ofMultMatrix(getMatrix());
			ofDrawRectangle(-width / 2, -height / 2, width, height);
			ofPopMatrix();
		}
	}
}

//  Update the Emitter. If it has been started, spawn new sprites with
//  initial velocity, lifespan, birthtime.
//
void Emitter::update() {
	if (!started) return;

	float time = ofGetElapsedTimeMillis();
	if ((time - lastSpawned) > (1000.0 / rate)) {
		// spawn a new sprite
		Sprite sprite;
		if (haveChildImage) sprite.setImage(childImage);
		// velocity keeps its original rate but is rotated by matrix
		sprite.velocity = rotDir * glm::vec4(velocity, 1);
		sprite.lifespan = lifespan;
		sprite.setPosition(trans);
		sprite.birthtime = time;
		sys->add(sprite);
		// utilizes established emitter update rate
		// to check if sound should be played when firing
		if (hasSound && playFireSound) fireSound.play();
		lastSpawned = time;
	}
	sys->update();
}

// Start/Stop the emitter.
//
void Emitter::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

void Emitter::stop() {
	started = false;
}


void Emitter::setLifespan(float life) {
	lifespan = life;
}

void Emitter::setVelocity(glm::vec3 v) {
	velocity = v;
}

void Emitter::setChildImage(ofImage img) {
	childImage = img;
	haveChildImage = true;
}

void Emitter::setImage(ofImage img) {
	image = img;
	haveImage = true;
	width = image.getWidth();
	height = image.getHeight();
}

void Emitter::setRate(float r) {
	rate = r;
}


// update the degree of rotation for firing direction
// and apply changes to the matrix
void Emitter::setFiringDir(float deg) {
	firingDir = deg;
}

void Emitter::setFiringMat(float deg) {
	rotDir = glm::rotate(glm::mat4(1.0), glm::radians(deg), glm::vec3(0, 0, 1));
}

// set the matrix that will be multiplied into the heading of travel 
// for turret when key pressed
void Emitter::setEmitterMat(float deg) {
	emitterRot = glm::rotate(glm::mat4(1.0), glm::radians(deg), glm::vec3(0, 0, 1));
}

// set firing sound
void Emitter::setFireSound(ofSoundPlayer sound) {
	hasSound = true;
	fireSound = sound;
}

// integrator for moving an emitter
void Emitter::integrate() {
	// linear thrust
	float dt = 1.0 / ofGetFrameRate();
	trans = trans + (moveVelocity * dt);
	glm::vec3 accel = moveAcceleration;
	accel = accel + moveForces;
	moveVelocity = moveVelocity + (accel * dt);
	moveVelocity = moveVelocity * moveDamping;

	// angular thrust
	rot = rot + (moveRotVel * dt);
	float rotAcceler = moveRotAcc;
	rotAcceler = rotAcceler + moveRotForces;
	moveRotVel = moveRotVel + (rotAcceler * dt);
	moveRotVel = moveRotVel * moveDamping;

	// adjust rotational matrix accordingly to guide turret travel heading
	setEmitterMat(rot);
	// adjust firing direction
	setFiringDir(rot);
	setFiringMat(rot);

	// reset forces after each integraion
	moveRotForces = 0.0;
	moveForces = glm::vec3(0, 0, 0);
}

//--------------------------------------------------------------
Particle::Particle(glm::vec3 dot) {
	this->setPosition(dot);
	debrisVel = glm::vec3(0, 0, 0);
	debrisAccel = glm::vec3(0, 0, 0);
	debrisForces = glm::vec3(0, 0, 0);
	debrisDamping = 0.99;
}

void Particle::draw() {
	ofSetColor(255, 0, 0);
	ofDrawRectangle(-5 + trans.x, -5 + trans.y, 5, 5);
}

void Particle::integrate() {
	float dt = 1.0 / ofGetFrameRate();
	trans = trans + (debrisVel * dt);
	glm::vec3 accel = debrisAccel;
	accel = accel + debrisForces;
	debrisVel = debrisVel + (accel * dt);
	debrisVel *= debrisDamping;
	debrisForces = glm::vec3(0, 0, 0);
}

Explosion::Explosion(glm::vec3 boomSite, int pts, float life, float power, int dust) {
	this->setPosition(boomSite);
	this->lifespan = life * 1000;// 1500;
	this->birthtime = ofGetElapsedTimeMillis();
	this->debrisCount = dust;// 20;
	this->points = pts;
	float addRot = 0.0;
	// set up each particle that is part of explosion
	for (int i = 0; i < debrisCount; i++) {
		Particle aDebris(boomSite);
		glm::mat4 debRotMat =
			glm::rotate(glm::mat4(1.0), glm::radians(addRot), glm::vec3(0, 0, 1));
		aDebris.debrisForces =
			debRotMat * glm::vec4(0, power * 1000, 0, 0);//15000, 0, 0);
		particles.push_back(aDebris);
		addRot += (360.0 / debrisCount);
	}
}

void Explosion::update() {
	for (Particle& p : particles) {
		p.integrate();
	}
}

void Explosion::draw() {
	ofSetColor(255, 0, 0);
	for (Particle& p : particles) {
		p.draw();
	}
	//ofDrawRectangle(-15 + trans.x, -15 + trans.y, 15, 15);
	string boomPts = "+" + std::to_string(points);
	ofDrawBitmapString(boomPts, ofPoint(trans.x, trans.y));
}

float Explosion::age() {
	return ofGetElapsedTimeMillis() - birthtime;
}

// helper method to add explosion to perform
void ofApp::addBoom(glm::vec3 boomPos, int thePts) {
	Explosion newBoom(boomPos, thePts, (float)boomLife, (float)boomPower, (int)boomDust);
	booms.push_back(newBoom);
}

// helper method to be put in update to remove expired booms
void ofApp::removeBoom() {
	if (booms.size() == 0) return;
	vector<Explosion>::iterator b = booms.begin();
	vector<Explosion>::iterator tmp;

	while (b != booms.end()) {
		if (b->lifespan != -1 && b->age() > b->lifespan) {
			tmp = booms.erase(b);
			b = tmp;
		}
		else b++;
	}
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);

	// load background image
	if (bkgImg.load("images/Project1_bkg.png")) {
		validBkg = true;
	}

	score = 0;

	// create an image for sprites being spawned by emitter
	//
	if (defaultImage.load("images/Project1_projectile.png")) {
		imageLoaded = true;
	} else {
		/*ofLogFatalError("can't load image: images/Project1_projectile.png");
		ofExit();*/
		imageLoaded = false;
	}
	// create image for invaders
	if (invaderImage.load("images/P1_enemy.png")) {
		invaderLoaded = true;
	} else {
		invaderLoaded = false;
	}
	if (specInvImage.load("images/P1_whitehot.png")) {
		specInvLoaded = true;
	} else {
		specInvLoaded = false;
	}

	projectiles = new Emitter(new SpriteSystem());
	projectiles->setPosition(glm::vec3(ofGetWindowWidth() / 2.0, ofGetWindowHeight() / 2.0, 1));
	projectiles->drawable = true;                // make emitter itself visible
	// set turret image, will be parent image for emitter
	if (turretImage.load("images/Project1_ship.png")) {
		projectiles->setImage(turretImage);
	} /*else {
		ofLogFatalError("can't load image: images/Project1_ship.png");
		ofExit();
	}*/
	if (imageLoaded) {
		projectiles->setChildImage(defaultImage);
		projectiles->setChildSize(defaultImage.getWidth(), defaultImage.getHeight());
	}

	// load the sound and set marker that sound loaded to true if successful
	if (firingSound.load("sounds/Project1_fireSound.wav")) {
		soundLoaded = true;
		projectiles->setFireSound(firingSound);
	}

	projectiles->setRate(0.001);

	projectiles->stop(); // game initially is in idle state

	// load collision sound
	if (invaderBoom.load("sounds/P1_collSound.wav")) {
		invBoomLoaded = true;
	}

	// set up first set of invaders (comes from top)
	invaders1 = new Emitter(new SpriteSystem());
	invaders1->setPosition(glm::vec3(ofGetWindowWidth() / 2.0, 0.0, 1));
	invaders1->drawable = false;
	if (invaderLoaded) {
		invaders1->setChildImage(invaderImage);
		invaders1->setChildSize(invaderImage.getWidth(), invaderImage.getHeight());
	}
	invaders1->setRate(0.5);
	invaders1->setVelocity(glm::vec3(0, 400, 1));
	if (invBoomLoaded) invaders1->sys->setBoom(invaderBoom);
	invaders1->stop();

	// set up second set of invaders (comes from left)
	invaders2 = new Emitter(new SpriteSystem());
	invaders2->setPosition(glm::vec3(0.0, ofGetWindowHeight() / 2.0, 1));
	invaders2->drawable = false;
	if (invaderLoaded) {
		invaders2->setChildImage(invaderImage);
		invaders2->setChildSize(invaderImage.getWidth(), invaderImage.getHeight());
	}
	invaders2->setRate(0.5);
	invaders2->setVelocity(glm::vec3(400, 0, 1));
	if (invBoomLoaded) invaders2->sys->setBoom(invaderBoom);
	invaders2->stop();

	// set up for third set of invaders (comes from right)
	invaders3 = new Emitter(new SpriteSystem());
	invaders3->setPosition(glm::vec3(ofGetWindowWidth(), ofGetWindowHeight() / 2.0, 1));
	invaders3->drawable = false;
	if (invaderLoaded) {
		invaders3->setChildImage(invaderImage);
		invaders3->setChildSize(invaderImage.getWidth(), invaderImage.getHeight());
	}
	invaders3->setRate(0.5);
	invaders3->setVelocity(glm::vec3(-400, 0, 1));
	if (invBoomLoaded) invaders3->sys->setBoom(invaderBoom);
	invaders3->stop();

	// set up for fourth set of invaders (comes from bottom)
	invaders4 = new Emitter(new SpriteSystem());
	invaders4->setPosition(glm::vec3(ofGetWindowWidth() / 2.0, ofGetWindowHeight(), 1));
	invaders4->drawable = false;
	if (invaderLoaded) {
		invaders4->setChildImage(invaderImage);
		invaders4->setChildSize(invaderImage.getWidth(), invaderImage.getHeight());
	}
	invaders4->setRate(0.5);
	invaders4->setVelocity(glm::vec3(0, -400, 1));
	if (invBoomLoaded) invaders4->sys->setBoom(invaderBoom);
	invaders4->stop();

	// set up for special invader (comes from corner)
	invaderS = new Emitter(new SpriteSystem());
	invaderS->setPosition(glm::vec3(0, 0, 1));
	invaderS->drawable = false;
	if (specInvLoaded) {
		invaderS->setChildImage(specInvImage);
		invaderS->setChildSize(specInvImage.getWidth(), specInvImage.getHeight());
	}
	invaderS->setRate(0.2);
	invaderS->setVelocity(glm::vec3(1500, 1500, 1));
	if (invBoomLoaded) invaderS->sys->setBoom(invaderBoom);
	invaderS->stop();

	// set up sliders
	gui.setup();
	//gui.add(rate.setup("Rate (turret)", 20, 1, 30)); // adjusts rate of fire
	//gui.add(fDir.setup("firing direction", 0, -360, 360)); // adjusts firing direction
	gui.add(lifespan1.setup("Lifespan (top)", 4, 0, 5));
	gui.add(lifespan2.setup("Lifespan (left)", 4, 0, 5));
	gui.add(lifespan3.setup("Lifespan (right)", 4, 0, 5));
	gui.add(lifespan4.setup("Lifespan (bottom)", 4, 0, 5));
	gui.add(lifespanS.setup("Lifespan (sp)", 4, 0, 5));
	gui.add(shipThrust.setup("Ship Thrust", 2000, 1000, 5000));
	//gui.add(rate1.setup("Rate (top)", 0.5, 0.5, 5));
	//gui.add(rate2.setup("Rate (left)", 0.5, 0.5, 5));
	//gui.add(rate3.setup("Rate (right)", 0.5, 0.5, 5));
	//gui.add(rate4.setup("Rate (bottom)", 0.5, 0.5, 5));
	//gui.add(rateS.setup("Rate (sp)", 0.2, 0.14, 1));
	gui.add(boomPower.setup("Boom Power", 15, 10, 30));
	gui.add(boomLife.setup("Boom Life", 1.5, 1, 4));
	gui.add(boomDust.setup("Boom Dust", 50, 10, 100));
	
	bHide = true;

	scoreBoard.load("fonts/verdana.ttf", 24);
	gameStartText.load("fonts/verdana.ttf", 18);
}

//--------------------------------------------------------------
void ofApp::update(){
	// update projectiles emitter to register
	// changes from sliders
	//projectiles->setRate(rate);
	//projectiles->setFiringDir(fDir);
	//projectiles->setFiringMat(fDir);

	// allows for ship to wrap around screen if goes out of bounds
	if (projectiles->trans.x < 0) {
		projectiles->setPosition(glm::vec3(ofGetWindowWidth() - 1, projectiles->trans.y, 1));
	}
	if (projectiles->trans.x > ofGetWindowWidth()) {
		projectiles->setPosition(glm::vec3(1, projectiles->trans.y, 1));
	}
	if (projectiles->trans.y < 0) {
		projectiles->setPosition(glm::vec3(projectiles->trans.x, ofGetWindowHeight() - 1, 1));
	}
	if (projectiles->trans.y > ofGetWindowHeight()) {
		projectiles->setPosition(glm::vec3(projectiles->trans.x, 1, 1));
	}

	if (projectiles->started) projectiles->integrate();
	projectiles->update();

	// updates first set of invaders
	// can launch from most of the top section of screen
	int inv1PosX = (int)ofRandom(ofGetWindowWidth() * 0.05, 0.95 * ofGetWindowWidth());
	invaders1->setPosition(glm::vec3(inv1PosX, 0.0, 1));
	// velocity increases with score increase, random within a range
	int inv1VelY = (int)ofRandom(400, 601);
	invaders1->setVelocity(glm::vec3(0, inv1VelY + (0.75 * score), 1));
	// random direction within a range to provide some fun
	int inv1Dir = (int)ofRandom(-45, 46);
	invaders1->setFiringDir((float)inv1Dir);
	invaders1->setFiringMat((float)inv1Dir);
	// random rate within a range, increases with score 
	float inv1Rate = ofRandom(0.25, 0.51);
	invaders1->setRate(inv1Rate + (score / 500.0));
	// set lifespan to slider amount
	invaders1->setLifespan(lifespan1 * 1000);
	// set rate to slider amount
	//invaders1->setRate(rate1);
	invaders1->update();

	// updates second set of invaders
	// can launch from most of the left section of screen
	int inv2PosY = (int)ofRandom(ofGetWindowHeight() * 0.05, 0.95 * ofGetWindowHeight());
	invaders2->setPosition(glm::vec3(0.0, inv2PosY, 1));
	// velocity increases with score increase, random within a range
	int inv2VelX = (int)ofRandom(400, 601);
	invaders2->setVelocity(glm::vec3(inv2VelX + (0.75 * score), 0, 1));
	// random direction within a range to provide some fun
	int inv2Dir = (int)ofRandom(-45, 46);
	invaders2->setFiringDir((float)inv2Dir);
	invaders2->setFiringMat((float)inv2Dir);
	// random rate within a range, increases with score
	float inv2Rate = ofRandom(0.25, 0.51);
	invaders2->setRate(inv2Rate + (score / 500.0));
	// set lifespan to slider amount
	invaders2->setLifespan(lifespan2 * 1000);
	// set rate to slider amount
	//invaders2->setRate(rate2);
	invaders2->update();

	// update third set of invaders
	// can launch from most of right section of screen
	int inv3PosY = (int)ofRandom(ofGetWindowHeight() * 0.05, 0.95 * ofGetWindowHeight());
	invaders3->setPosition(glm::vec3(ofGetWindowWidth(), inv3PosY, 1));
	// velocity increases with score increase, random within a range
	int inv3VelX = (int)ofRandom(-600, -399);
	invaders3->setVelocity(glm::vec3(inv3VelX - (0.75 * score), 0, 1));
	// random direction within a range to provide some fun
	int inv3Dir = (int)ofRandom(-45, 46);
	invaders3->setFiringDir((float)inv3Dir);
	invaders3->setFiringMat((float)inv3Dir);
	// random rate within a range, increases with score
	float inv3Rate = ofRandom(0.25, 0.51);
	invaders3->setRate(inv3Rate + (score / 500.0));
	// set lifespan to slider amount
	invaders3->setLifespan(lifespan3 * 1000);
	// set rate to slider amount
	//invaders3->setRate(rate3);
	invaders3->update();

	// update fourth set of invaders
	// can launch from most of bottom section of screen
	int inv4PosX = (int)ofRandom(ofGetWindowWidth() * 0.05, 0.95 * ofGetWindowWidth());
	invaders4->setPosition(glm::vec3(inv4PosX, ofGetWindowHeight(), 1));
	// velocity increases with score increase, random within a range
	int inv4VelY = (int)ofRandom(-600, -399);
	invaders4->setVelocity(glm::vec3(0, inv4VelY - (0.75 * score), 1));
	// random direction within a range to provide some fun
	int inv4Dir = (int)ofRandom(-45, 46);
	invaders4->setFiringDir((float)inv4Dir);
	invaders4->setFiringMat((float)inv4Dir);
	// random rate within a range, increases with score
	float inv4Rate = ofRandom(0.25, 0.51);
	invaders4->setRate(inv4Rate + (score / 500.0));
	// set lifespan to slider amount
	invaders4->setLifespan(lifespan4 * 1000);
	// set rate to slider amount
	//invaders4->setRate(rate4);
	invaders4->update();

	// update for special invader
	// can launch from any of the four corners, chosen randomly
	int invSPosStart = (int)ofRandom(0, 4);
	switch (invSPosStart) {
	case 0:
		invaderS->setPosition(glm::vec3(0, 0, 1));
		invaderS->setVelocity(glm::vec3(1500 + (score * 0.75), 1500 + (score * 0.75), 1));
		break;
	case 1:
		invaderS->setPosition(glm::vec3(ofGetWindowWidth(), 0, 1));
		invaderS->setVelocity(glm::vec3(-1500 - (score * 0.75), 1500 + (score * 0.75), 1));
		break;
	case 2:
		invaderS->setPosition(glm::vec3(0, ofGetWindowHeight(), 1));
		invaderS->setVelocity(glm::vec3(1500 + (score * 0.75), -1500 - (score * 0.75), 1));
		break;
	case 3:
		invaderS->setPosition(glm::vec3(ofGetWindowWidth(), ofGetWindowHeight(), 1));
		invaderS->setVelocity(glm::vec3(-1500 - (score * 0.75), -1500 - (score * 0.75), 1));
		break;
	default:
		break;
	}
	// random direction within a range to provide more fun
	int invSDir = (int)ofRandom(-45, 46);
	invaderS->setFiringDir(invSDir);
	invaderS->setFiringMat(invSDir);
	// random rate within a range to keep player guessing
	float invSRate = ofRandom(0.14, 0.21);
	invaderS->setRate(invSRate);
	// set lifespan to slider amount
	invaderS->setLifespan(lifespanS * 1000);
	// set rate to slider amount
	//invaderS->setRate(rateS);
	invaderS->update();

	// check collisions between projectiles and invaders
	checkCollisions();
	// only update explosions if game starts
	if (gameStarted) {
		for (Explosion& e : booms) {
			e.update();
		}
	}
	removeBoom();
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (validBkg) bkgImg.draw(0, 0); // draw background if valid
	projectiles->draw();
	invaders1->draw();
	invaders2->draw();
	invaders3->draw();
	invaders4->draw();
	invaderS->draw();

	// draw explosions here
	for (Explosion& e : booms) {
		e.draw();
	}
	ofSetColor(255, 255, 255, 255);

	// draw score
	string scoreText;
	scoreText += "Score: " + std::to_string(score);
	scoreBoard.drawString(scoreText, ofGetWindowWidth() / 2.0 - 80, 40);

	if (!gameStarted) {
		gameStartText.drawString("Press space to start the game", ofGetWindowWidth() / 2.0 - 200, ofGetWindowHeight() - 100);
	}

	if (!bHide) {
		gui.draw();
	}
}

// collision checking
void ofApp::checkCollisions() {
	// distances where projectile should count as collided with invader
	float collisionDist1 = projectiles->childHeight / 2 + invaders1->childHeight / 2;
	float collisionDist2 = projectiles->childHeight / 2 + invaders2->childHeight / 2;
	float collisionDist3 = projectiles->childHeight / 2 + invaders3->childHeight / 2;
	float collisionDist4 = projectiles->childHeight / 2 + invaders4->childHeight / 2;
	float collisionDistS = projectiles->childHeight / 2 + invaderS->childHeight / 2;

	// loop through projectiles, remove hit invaders
	for (int i = 0; i < projectiles->sys->sprites.size(); i++) {
		int oldScore = score;
		// regular invaders worth 1 point
		score += invaders1->sys->removeNear(projectiles->sys->sprites[i].trans, collisionDist1);
		// check for explosion here
		if (score > oldScore) {
			addBoom(projectiles->sys->sprites[i].trans, 1);
			oldScore = score;
		}
		score += invaders2->sys->removeNear(projectiles->sys->sprites[i].trans, collisionDist2);
		// check for explosion here
		if (score > oldScore) {
			addBoom(projectiles->sys->sprites[i].trans, 1);
			oldScore = score;
		}
		score += invaders3->sys->removeNear(projectiles->sys->sprites[i].trans, collisionDist3);
		// check for explosion here
		if (score > oldScore) {
			addBoom(projectiles->sys->sprites[i].trans, 1);
			oldScore = score;
		}
		score += invaders4->sys->removeNear(projectiles->sys->sprites[i].trans, collisionDist4);
		// check for explosion here
		if (score > oldScore) {
			addBoom(projectiles->sys->sprites[i].trans, 1);
			oldScore = score;
		}

		// special invader worth 4 points
		score += invaderS->sys->removeNear(projectiles->sys->sprites[i].trans, collisionDistS) * 4;
		// check for explosion here
		if (score > oldScore) {
			addBoom(projectiles->sys->sprites[i].trans, 4);
			oldScore = score;
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		// toggle visibility of sliders
		bHide = !bHide;
		break;
	case ' ':
		//cout << "space pressed" << endl;
		if (!gameStarted) gameStarted = true;
		// space starts the game by starting all emitters
		if (!projectiles->started) projectiles->start(); 
		if (!invaders1->started) invaders1->start();
		if (!invaders2->started) invaders2->start();
		if (!invaders3->started) invaders3->start();
		if (!invaders4->started) invaders4->start();
		if (!invaderS->started) invaderS->start();

		// resets the emitter lifespan, velocity, and rate to fire projectiles
		projectiles->setVelocity(glm::vec3(0, FIRING_SPEED, 1));
		projectiles->setRate(FIRERATE);
		projectiles->setLifespan(LIFE);

		// emit sound
		//cout << soundLoaded << endl;
		if (projectiles->started && soundLoaded) {
			projectiles->playFireSound = true;
		}

		break;
	// keys below to move the player turret
	case OF_KEY_UP:
		// only move up if game started and within bounds
		//predictionUp = projectiles->trans - (glm::vec3)(projectiles->emitterRot * glm::vec4(0, MOVEMENT_SPEED, 0, 0));
		//if (projectiles->started && predictionUp.y > 0 && predictionUp.y < ofGetWindowHeight()
			//&& predictionUp.x > 0 && predictionUp.x < ofGetWindowWidth())
			//projectiles->trans -= (glm::vec3)(projectiles->emitterRot * glm::vec4(0, MOVEMENT_SPEED, 0, 0));

		if (projectiles->started)
			projectiles->moveForces = (glm::vec3)(projectiles->emitterRot * glm::vec4(0, -(float)shipThrust, 0, 0));
		break;
	case OF_KEY_DOWN:
		// only move down if game started and within bounds
		//predictionDown = projectiles->trans + (glm::vec3)(projectiles->emitterRot * glm::vec4(0, MOVEMENT_SPEED, 0, 0));
		//if (projectiles->started && predictionDown.y > 0 && predictionDown.y < ofGetWindowHeight()
			//&& predictionDown.x > 0 && predictionDown.x < ofGetWindowWidth())
			//projectiles->trans += (glm::vec3)(projectiles->emitterRot * glm::vec4(0, MOVEMENT_SPEED, 0, 0));
		
		if (projectiles->started)
			projectiles->moveForces = (glm::vec3)(projectiles->emitterRot * glm::vec4(0, (float)shipThrust, 0, 0));
		break; 
	case OF_KEY_LEFT:
		// only move left if game started and within bounds
		//predictionLeft = projectiles->trans - (glm::vec3)(projectiles->emitterRot * glm::vec4(MOVEMENT_SPEED, 0, 0, 0));
		//if (projectiles->started && predictionLeft.y > 0 && predictionLeft.y < ofGetWindowHeight()
			//&& predictionLeft.x > 0 && predictionLeft.x < ofGetWindowWidth())
			//projectiles->trans -= (glm::vec3)(projectiles->emitterRot * glm::vec4(MOVEMENT_SPEED, 0, 0, 0));

		if (projectiles->started)
			projectiles->moveForces = (glm::vec3)(projectiles->emitterRot * glm::vec4(-(float)shipThrust, 0, 0, 0));
		break;
	case OF_KEY_RIGHT:
		// only move right if game started and within bounds
		//predictionRight = projectiles->trans + (glm::vec3)(projectiles->emitterRot * glm::vec4(MOVEMENT_SPEED, 0, 0, 0));
		//if (projectiles->started && predictionRight.y > 0 && predictionRight.y < ofGetWindowHeight()
			//&& predictionRight.x > 0 && predictionRight.x < ofGetWindowWidth())
			//projectiles->trans += (glm::vec3)(projectiles->emitterRot * glm::vec4(MOVEMENT_SPEED, 0, 0, 0));

		if (projectiles->started)
			projectiles->moveForces = (glm::vec3)(projectiles->emitterRot * glm::vec4((float)shipThrust, 0, 0, 0));
		break;
	case 'R':
	case 'r':
		// only rotate clockwise if game started
		if (projectiles->started) {
			//projectiles->rot += ROT_SPEED;
			projectiles->moveRotForces = (float)shipThrust;
			/*
			// adjust rotational matrix accordingly to guide turret travel heading
			projectiles->setEmitterMat(projectiles->rot);
			// adjust firing direction
			projectiles->setFiringDir(projectiles->rot);
			projectiles->setFiringMat(projectiles->rot);
			*/
		}
		break;
	case 'E':
	case 'e':
		// only rotate counterclockwise if game started
		if (projectiles->started) {
			//projectiles->rot -= ROT_SPEED;
			projectiles->moveRotForces = -(float)shipThrust;
			/*
			// adjust rotational matrix accordingly to guide turret travel heading
			projectiles->setEmitterMat(projectiles->rot);
			// adjust firing direction
			projectiles->setFiringDir(projectiles->rot);
			projectiles->setFiringMat(projectiles->rot);
			*/
		}
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch (key) {
	/*
	Overall idea is to hide emitted projectiles under the ship
	with no velocity and basically no lifespan
	to give illusion of stopped firing
	*/
	case ' ':
		//cout << "space released" << endl;

		// when space bar released
		// projectiles to have no velocity and extreme low rate
		projectiles->setVelocity(glm::vec3(0, 0, 0));
		projectiles->setRate(0.001);
		// lifespan needed to be reduced to avoid trailing 
		// projectiles as my implementation
		// does not stop drawing the projectiles
		projectiles->setLifespan(0);

		// stop sound
		projectiles->playFireSound = false;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	//	cout << "mouse( " << x << "," << y << ")" << endl;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	// only allow mouse to drag turret if game started
	// and mouse click is in bounds
	if (!projectiles->started) return;
	if (!projectiles->bSelected) return;

	glm::vec3 mouse = glm::vec3(x, y, 1);
	glm::vec3 delta = mouse - mouse_last; // distance to move turret

	// keep the ship in bounds
	if (mouse.x < 0 || mouse.x > ofGetWindowWidth() ||
		mouse.y < 0 || mouse.y > ofGetWindowHeight()) {
		projectiles->bSelected = false;
		return;
	}
		
	projectiles->trans += delta; // moving the turret

	mouse_last = mouse;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	glm::vec3 mouse = glm::vec3(x, y, 1);

	// check if mouse click is within the bounding circle of the turret
	if (glm::distance(projectiles->trans, mouse) < projectiles->width / 2.0) {
		projectiles->bSelected = true; // signal that turret can be moved
		mouse_last = mouse; // set mouse click location
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	// when mouse click released, it can no longer move turret
	projectiles->bSelected = false; 
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
