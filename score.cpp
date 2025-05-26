#include "score.h"
#include <ctime>
#include <sstream>

Score::Score(std::string alias, int sala, int vidaPerdida)
    : alias(alias), salaAlcanzada(sala), vidaTotalPerdida(vidaPerdida) {
    time_t now = time(0);
    fecha = ctime(&now);
    if (!fecha.empty()) fecha.pop_back(); // quitar \n
}

std::string Score::getLineaArchivo() const {
    std::ostringstream out;
    out << alias << "," << fecha << "," << salaAlcanzada << "," << vidaTotalPerdida;
    return out.str();
} 
