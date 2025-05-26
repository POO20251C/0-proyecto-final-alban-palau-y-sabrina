#pragma once
#include <string>
using namespace std;

class Character {
protected:
    string name;
    int hp, atk, def, spd, lck;

public:
    Character(string name, int hp, int atk, int def, int spd, int lck);
    virtual ~Character() {}
    virtual void atacar(Character& target) = 0;
    virtual void recibirDano(int damage) = 0;

    bool vivo();
    string getName();
    int getHP();
    int getATK();
    int getDEF();
    int getSPD();
    int getLCK();

    void setHP(int hp);
    void setATK(int atk);
    void setDEF(int def);
    void setSPD(int spd);
    void setLCK(int lck);
};
