#include "enemy.h"
#include <iostream>

Enemy::Enemy(std::string name, int hp, int atk, int def, int spd, int lck)
    : Character(name, hp, atk, def, spd, lck) {}

void Enemy::atacar(Character& target) {
    int dano = atk - target.getDEF();
    if (dano < 1) dano = 1;
    std::cout << name << " ataca a " << target.getName() << " causando " << dano << " de daño.\n";
    target.recibirDano(dano);
}

void Enemy::recibirDano(int damage) {
    hp -= damage;
    if (hp < 0) hp = 0;
}