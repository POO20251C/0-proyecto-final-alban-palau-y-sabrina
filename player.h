#pragma once
#include "hero.h"
#include <vector>
#include <string>

class Player {
private:
    std::string alias;
    std::vector<Hero> equipo;
    int salaActual;

public:
    Player(std::string alias);

    void agregarHeroe(const Hero& heroe);
    std::vector<Hero>& getEquipo();

    std::string getAlias() const;
    int getSala() const;
    void avanzarSala();
    void reiniciarMazmorra();
};
