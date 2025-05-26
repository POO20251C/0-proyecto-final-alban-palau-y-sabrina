#include "player.h"

Player::Player(std::string alias) : alias(alias), salaActual(1) {}

void Player::agregarHeroe(const Hero& heroe) {
    if (equipo.size() < 3)
        equipo.push_back(heroe);
}

std::vector<Hero>& Player::getEquipo() {
    return equipo;
}

std::string Player::getAlias() const {
    return alias;
}

int Player::getSala() const {
    return salaActual;
}

void Player::avanzarSala() {
    salaActual++;
}

void Player::reiniciarMazmorra() {
    salaActual = 1;
    equipo.clear();
}