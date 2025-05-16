#hero.h
#Tipos de héroes:


class Hero : public Character {
    public:
      Hero(string name, int hp, int atk, int def, int spd, int lck) : Character(name, hp, atk, def, spd, lck);
      void atacar(Character& target);
      void recibirDano(int damage);
      void usarItem(Item& item);
      void subirNivel();
      void agregarItem(Item& item);
      void mostrarInventario();
}
