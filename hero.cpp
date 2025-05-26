#include "hero.h"
#include <iostream>
#include <cmath>

Hero::Hero(std::string name, int hp, int atk, int def, int spd, int lck)
    : Character(name, hp, atk, def, spd, lck) {}

void Hero::atacar(Character& target) {
    int dano = atk - target.getDEF();
    if (dano < 1) dano = 1;
    std::cout << name << " ataca a " << target.getName() << " causando " << dano << " de daño.\n";
    target.recibirDano(dano);
}

void Hero::recibirDano(int damage) {
    hp -= damage;
    if (hp < 0) hp = 0;
}

void Hero::usarItem(int index) {
    if (index >= 0 && index < inventario.size()) {
        inventario[index]->usar(*this);
        inventario.erase(inventario.begin() + index);
    } else {
        std::cout << "Índice de ítem inválido.\n";
    }
}

void Hero::agregarItem(std::shared_ptr<Item> item) {
    inventario.push_back(item);
}

void Hero::mostrarInventario() {
    std::cout << "Inventario de " << name << ":\n";
    for (int i = 0; i < inventario.size(); ++i) {
        std::cout << i << ". " << inventario[i]->getNombre() << "\n";
    }
}

void Hero::subirNivel() {
    atk += std::round(atk * 0.02);
    def += std::round(def * 0.02);
}
