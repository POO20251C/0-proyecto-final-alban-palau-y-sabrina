#include "items.h"

Item::Item(const std::string& nombre) : nombre(nombre) {}

std::string Item::getNombre() const {
    return nombre;
}
