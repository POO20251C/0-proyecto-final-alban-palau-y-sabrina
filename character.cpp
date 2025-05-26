#include "character.h"

Character::Character(string name, int hp, int atk, int def, int spd, int lck)
    : name(name), hp(hp), atk(atk), def(def), spd(spd), lck(lck) {}

bool Character::vivo() { return hp > 0; }

string Character::getName() { return name; }
int Character::getHP() { return hp; }
int Character::getATK() { return atk; }
int Character::getDEF() { return def; }
int Character::getSPD() { return spd; }
int Character::getLCK() { return lck; }

void Character::setHP(int hp) { this->hp = hp; }
void Character::setATK(int atk) { this->atk = atk; }
void Character::setDEF(int def) { this->def = def; }
void Character::setSPD(int spd) { this->spd = spd; }
void Character::setLCK(int lck) { this->lck = lck; }
