#pragma once
#include "character.h"
#include "items.h"
#include <vector>
#include <memory>

class Hero : public Character {
private:
    std::vector<std::shared_ptr<Item>> inventario;

public:
    Hero(std::string name, int hp, int atk, int def, int spd, int lck);
    void atacar(Character& target) override;
    void recibirDano(int damage) override;

    void usarItem(int index);
    void agregarItem(std::shared_ptr<Item> item);
    void mostrarInventario();
    void subirNivel(); // Aumenta stats 2%
};
