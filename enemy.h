//Tipos de enemigos: Soldado, mini-jefe, jefe.
class Enemy : public Character {
    public:
      Enemy(string name, int hp, int atk, int def, int spd, int lck) : Character(name, hp, atk, def, spd, lck);
      void atacar(Character& target);
      void recibirDano(int damage);
}
