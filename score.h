#pragma once
#include <string>

class Score {
private:
    std::string alias;
    std::string fecha;
    int salaAlcanzada;
    int vidaTotalPerdida;

public:
    Score(std::string alias, int sala, int vidaPerdida);
    std::string getLineaArchivo() const;
};
