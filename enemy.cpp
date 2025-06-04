// ===== ENEMY CLASS =====
class Enemy : public Character {
private:
    string type;

public:
    Enemy(const string& name, int hp, int atk, int def, int spd, int lck, const string& type)
        : Character(name, hp, atk, def, spd, lck), type(type) {}
    
    string getType() const { return type; }
};