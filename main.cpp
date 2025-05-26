# Avances main.cpp
#include <iostream>
#include "Jugador.h"
#include "Enemigo.h"
#include "item.h"

int main() {
    Jugador jugador("Heroe");
    Enemigo enemigo("Goblin", 30, 5, 1);

    std::cout << "Comienza el combate!\n";

    while (jugador.estaVivo() && enemigo.estaVivo()) {
        jugador.atacar(enemigo);
        if (enemigo.estaVivo())
            enemigo.atacar(jugador);
    }

    std::cout << (jugador.estaVivo() ? "Ganaste!" : "Has sido derrotado...") << std::endl;
    return 0;
}