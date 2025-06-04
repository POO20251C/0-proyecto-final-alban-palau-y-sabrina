#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <memory> // Para smart pointers si decidimos usarlos, aunque por ahora no se usan directamente para ownership de personajes/items en vectors.

using namespace std;

// Forward declarations
class Character;
class Hero;
class Enemy;
class Item;
class Weapon;
class Armor;
class Potion;
class Inventory;
class Battle;
class Room;
class Score;
class ScoreManager;
class Game;


// Utility function for user input
int getValidatedInput(int min, int max) {
    int choice;
    while (!(cin >> choice) || choice < min || choice > max) {
        wcout << "Entrada inválida. Por favor, ingresa un número entre " << min << " y " << max << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear remaining input
    return choice;
}

// ===== CHARACTER CLASS (BASE ABSTRACT CLASS) =====
class Character {
protected:
    string name;
    int hp;
    int maxHp;
    int atk;
    int def;
    int spd;
    int lck;

public:
    Character(const string& name, int hp, int atk, int def, int spd, int lck)
        : name(name), hp(hp), maxHp(hp), atk(atk), def(def), spd(spd), lck(lck) {}
    
    virtual ~Character() = default;
    
    // Getters
    string getName() const { return name; }
    int getHp() const { return hp; }
    int getMaxHp() const { return maxHp; }
    int getAtk() const { return atk; }
    int getDef() const { return def; }
    int getSpd() const { return spd; }
    int getLck() const { return lck; }
    
    // Setters (for stat boosts)
    void setAtk(int val) { atk = val; }
    void setDef(int val) { def = val; }
    void setSpd(int val) { spd = val; }
    void setLck(int val) { lck = val; }
    void setHp(int val) { hp = val; }
    void setMaxHp(int val) { maxHp = val; }


    // Health management
    virtual void takeDamage(int damage) {
        hp = max(0, hp - damage);
    }
    
    void heal(int amount) {
        hp = min(maxHp, hp + amount);
    }
    
    bool isAlive() const {
        return hp > 0;
    }
    
    // Combat methods
    bool calculateHitChance(const Character* defender) const {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 100);
        
        int hitChance = 85 + (lck - defender->lck) * 2;
        hitChance = max(10, min(95, hitChance)); // Clamp between 10-95%
        
        return dis(gen) <= hitChance;
    }
    
    int calculateDamage(const Character* defender) const {
        int baseDamage = max(1, atk - defender->def);
        
        // Critical hit chance based on luck
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> critRoll(1, 100);
        
        if (critRoll(gen) <= lck) {
            baseDamage = static_cast<int>(baseDamage * 1.5);
            wcout << "¡Golpe crítico!" << endl;
        }
        
        return baseDamage;
    }
    
    void displayStats() const {
        cout << name << " - HP: " << hp << "/" << maxHp 
             << " ATK: " << atk << " DEF: " << def 
             << " SPD: " << spd << " LCK: " << lck << endl;
    }
};

// ===== ITEM CLASS (BASE ABSTRACT CLASS) =====
class Item {
protected:
    string name;
    string rarity;
    int statBoost1;
    int statBoost2;
    string affectedStat1;
    string affectedStat2;

public:
    Item(const string& name, const string& rarity, int boost1, int boost2, 
         const string& stat1, const string& stat2)
        : name(name), rarity(rarity), statBoost1(boost1), statBoost2(boost2),
          affectedStat1(stat1), affectedStat2(stat2) {}
    
    virtual ~Item() = default;
    
    // Getters
    string getName() const { return name; }
    string getRarity() const { return rarity; }
    int getStatBoost1() const { return statBoost1; }
    int getStatBoost2() const { return statBoost2; }
    string getAffectedStat1() const { return affectedStat1; }
    string getAffectedStat2() const { return affectedStat2; }
    
    virtual void displayInfo() const {
        cout << name << " (" << rarity << ") - " 
             << affectedStat1 << " +" << statBoost1;
        if (statBoost2 > 0 && !affectedStat2.empty()) { // Ensure second stat is meaningful
            cout << ", " << affectedStat2 << " +" << statBoost2;
        }
        wcout << endl;
    }
};

// ===== WEAPON CLASS =====
class Weapon : public Item {
public:
    Weapon(const string& name, const string& rarity, int atkBoost, int secondaryBoost, const string& secondaryStat)
        : Item(name, rarity, atkBoost, secondaryBoost, "ATK", secondaryStat) {}
    
    string getWeaponType() const { return "Weapon"; }
};

// ===== ARMOR CLASS =====
class Armor : public Item {
public:
    Armor(const string& name, const string& rarity, int defBoost, int secondaryBoost, const string& secondaryStat)
        : Item(name, rarity, defBoost, secondaryBoost, "DEF", secondaryStat) {}
    
    string getArmorType() const { return "Armor"; }
};

// ===== POTION CLASS =====
class Potion : public Item {
private:
    bool used;

public:
    Potion(const string& name, int boost1, int boost2, const string& stat1, const string& stat2)
        : Item(name, "Consumible", boost1, boost2, stat1, stat2), used(false) {}
    
    bool isUsed() const { return used; }
    void setUsed(bool usedState) { used = usedState; }
    void resetUsage() { used = false; }
};

// ===== HERO CLASS =====
class Hero : public Character {
private:
    Weapon* weapon;
    Armor* armor;
    vector<Potion*> potions;
    int totalHealthLost;
    vector<int> originalStats; // To store initial stats for potion removal

public:
    Hero(const string& name, int hp, int atk, int def, int spd, int lck)
        : Character(name, hp, atk, def, spd, lck), weapon(nullptr), armor(nullptr), totalHealthLost(0) {
            originalStats = {hp, atk, def, spd, lck}; // Store initial stats
        }
    
    // Equipment management
    void equipWeapon(Weapon* newWeapon) {
        // Remove old weapon bonuses if exists
        if (weapon) {
            removeItemBonuses(weapon);
        }
        weapon = newWeapon;
        if (weapon) {
            applyItemBonuses(weapon);
        }
    }
    
    void equipArmor(Armor* newArmor) {
        // Remove old armor bonuses if exists
        if (armor) {
            removeItemBonuses(armor);
        }
        armor = newArmor;
        if (armor) {
            applyItemBonuses(armor);
        }
    }
    
    void addPotion(Potion* potion) {
        potions.push_back(potion);
    }
    
    void usePotion(int index) {
        if (index >= 0 && index < potions.size() && !potions[index]->isUsed()) {
            applyItemBonuses(potions[index]);
            potions[index]->setUsed(true);
            cout << name << " usa " << potions[index]->getName() << "!" << endl;
        } else {
             wcout << "No puedes usar esa poción." << endl;
        }
    }

    // This method is crucial to reset potion effects after battle or specific events
    void resetPotionEffects() {
        for (Potion* p : potions) {
            if (p->isUsed()) {
                // For simplicity, we just reset the usage flag.
                // A more robust system would involve tracking temporary boosts.
                p->resetUsage();
            }
        }
        // Reapply equipment bonuses to ensure they are active after potion reset
        // This is a simplified approach, a more complex one would involve storing base stats
        // and only applying/removing temporary buffs.
        // For this current implementation, we'll assume stats reset to their *base*
        // and then equipped items are reapplied.
        
        // Reset to original character stats
        hp = originalStats[0];
        maxHp = originalStats[0];
        atk = originalStats[1];
        def = originalStats[2];
        spd = originalStats[3];
        lck = originalStats[4];

        // Reapply equipment bonuses
        if (weapon) {
            applyItemBonuses(weapon);
        }
        if (armor) {
            applyItemBonuses(armor);
        }
    }
    
    // Getters
    Weapon* getEquippedWeapon() const { return weapon; }
    Armor* getEquippedArmor() const { return armor; }
    vector<Potion*> getPotions() const { return potions; }
    int getTotalHealthLost() const { return totalHealthLost; }
    
    // Stat management
    void takeDamage(int damage) override {
        int healthBefore = hp;
        Character::takeDamage(damage);
        totalHealthLost += (healthBefore - hp);
    }
    
    void boostStats(float percentage) {
        atk = static_cast<int>(atk * (1.0f + percentage / 100.0f));
        def = static_cast<int>(def * (1.0f + percentage / 100.0f));
        cout << name << " ha mejorado sus estadísticas (ATK y DEF +" << percentage << "%)." << endl;
    }
    
    void displayEquipment() const {
        cout << "\n=== Equipamiento de " << name << " ===" << endl;
        if (weapon) {
            wcout << "Arma: ";
            weapon->displayInfo();
        } else {
            wcout << "Arma: Ninguna" << endl;
        }
        
        if (armor) {
            wcout << "Armadura: ";
            armor->displayInfo();
        } else {
            wcout << "Armadura: Ninguna" << endl;
        }
        
        wcout << "Pociones: " << potions.size() << endl;
        bool hasPotions = false;
        for (size_t i = 0; i < potions.size(); ++i) {
            if (!potions[i]->isUsed()) {
                cout << "  " << (i + 1) << ". ";
                potions[i]->displayInfo();
                hasPotions = true;
            }
        }
        if (!hasPotions) {
            cout << "  Ninguna poción disponible." << endl;
        }
        cout << "--------------------------------" << endl;
    }

private:
    void applyItemBonuses(Item* item) {
        if (item->getAffectedStat1() == "ATK") atk += item->getStatBoost1();
        else if (item->getAffectedStat1() == "DEF") def += item->getStatBoost1();
        else if (item->getAffectedStat1() == "HP") { 
            maxHp += item->getStatBoost1(); 
            hp += item->getStatBoost1(); 
        }
        else if (item->getAffectedStat1() == "SPD") spd += item->getStatBoost1();
        else if (item->getAffectedStat1() == "LCK") lck += item->getStatBoost1();
        
        if (item->getStatBoost2() > 0 && !item->getAffectedStat2().empty()) {
            if (item->getAffectedStat2() == "ATK") atk += item->getStatBoost2();
            else if (item->getAffectedStat2() == "DEF") def += item->getStatBoost2();
            else if (item->getAffectedStat2() == "HP") { 
                maxHp += item->getStatBoost2(); 
                hp += item->getStatBoost2(); 
            }
            else if (item->getAffectedStat2() == "SPD") spd += item->getStatBoost2();
            else if (item->getAffectedStat2() == "LCK") lck += item->getStatBoost2();
        }
    }
    
    void removeItemBonuses(Item* item) {
        if (item->getAffectedStat1() == "ATK") atk -= item->getStatBoost1();
        else if (item->getAffectedStat1() == "DEF") def -= item->getStatBoost1();
        else if (item->getAffectedStat1() == "HP") { 
            maxHp -= item->getStatBoost1(); 
            hp = min(hp, maxHp); // Cap current HP at new maxHp
        }
        else if (item->getAffectedStat1() == "SPD") spd -= item->getStatBoost1();
        else if (item->getAffectedStat1() == "LCK") lck -= item->getStatBoost1();
        
        if (item->getStatBoost2() > 0 && !item->getAffectedStat2().empty()) {
            if (item->getAffectedStat2() == "ATK") atk -= item->getStatBoost2();
            else if (item->getAffectedStat2() == "DEF") def -= item->getStatBoost2();
            else if (item->getAffectedStat2() == "HP") { 
                maxHp -= item->getStatBoost2(); 
                hp = min(hp, maxHp); // Cap current HP at new maxHp
            }
            else if (item->getAffectedStat2() == "SPD") spd -= item->getStatBoost2();
            else if (item->getAffectedStat2() == "LCK") lck -= item->getStatBoost2();
        }
    }
};

// ===== ENEMY CLASS =====
class Enemy : public Character {
private:
    string type;

public:
    Enemy(const string& name, int hp, int atk, int def, int spd, int lck, const string& type)
        : Character(name, hp, atk, def, spd, lck), type(type) {}
    
    string getType() const { return type; }
};

// ===== INVENTORY CLASS =====
class Inventory {
private:
    vector<Weapon*> weapons;
    vector<Armor*> armors;
    vector<Potion*> potions;
    random_device rd;
    mt19937 gen;

public:
    Inventory() {
        gen.seed(rd());
        initializeItems();
    }
    
    ~Inventory() {
        for (auto weapon : weapons) delete weapon;
        for (auto armor : armors) delete armor;
        for (auto potion : potions) delete potion;
    }
    
    void initializeItems() {
        // Weapon names
        vector<string> weaponNames = {
            "Machete del Llanero", "Lanza de Totumo", "Botella vacía", "Caña de Pescar Oxidada",
            "Argolla de sapo", "Chancla Voladora", "Cruceta del carro", "Cuchillo de carnicero",
            "Pesa de gimnasio", "Sombrero Vueltiao Cortante", "Paraguas de TransMilenio", 
            "Látigo del Amazonas", "Lapicero PaperMate", "Balón de Microfútbol Golty",
            "Arepa Caliente", "Alcancía", "Destapador Inmortal", "Bandera de la Selección",
            "Acordeón Bendecido", "El florero de Llorente"
        };
        
        // Armor names
        vector<string> armorNames = {
            "Chaleco de Moto Ratera", "Ruana de la Abuela", "Capa de Lluvia Bogotana",
            "Camisa de Lino Carretero", "Protección de Baloto", "Armadura de Buseta",
            "Gabán Antiviral", "Camiseta de Fútbol Sudada", "Chubasquero del Amazonas",
            "Coraza de Tusa", "Poncho Cafetero", "Protección de Tambores", "Overol de Obrero",
            "Chaleco de Pesca Urbana", "Cáscara de Bananazo", "Traje de Baile del Carnaval",
            "Placa de TransMi", "Camiseta del Once Caldas", "Uniforme Escolar Inmortal",
            "Protector de Cerveza Artesanal"
        };
        
        // Potion names
        vector<string> potionNames = {
            "Aguapanela Hirviente", "Vive100", "Lechona Mágica", "Juan Váldez Carga Triple",
            "Jugo de tomate de arbol", "Changua Bendita", "Aguardiente del Valle",
            "Aguardiente Antioqueño", "Cholado Energético", "Lulada Espiritual"
        };
        
        vector<string> secondaryStats = {"HP", "DEF", "SPD", "LCK"};
        
        // Create weapons (10 common, 10 rare)
        for (int i = 0; i < 20; ++i) {
            string rarity = (i < 10) ? "Common" : "Rare";
            // Common: +4-5 ATK + boost menor = 6-7 points total
            // Rare: +5-7 ATK + boost fuerte = 8-10 points total
            int atkBoost = (rarity == "Common") ? (4 + (gen() % 2)) : (5 + (gen() % 3));
            int secondaryBoost = (rarity == "Common") ? (2 + (gen() % 2)) : (3 + (gen() % 3));
            
            uniform_int_distribution<> statDis(0, secondaryStats.size() - 1);
            string secondaryStat = secondaryStats[statDis(gen)];
            
            weapons.push_back(new Weapon(weaponNames[i], rarity, atkBoost, secondaryBoost, secondaryStat));
        }
        
        // Create armors (10 common, 10 rare)
        for (int i = 0; i < 20; ++i) {
            string rarity = (i < 10) ? "Common" : "Rare";
            // Common: +4-5 DEF + boost menor = 6-7 points total
            // Rare: +5-7 DEF + boost fuerte = 8-10 points total
            int defBoost = (rarity == "Common") ? (4 + (gen() % 2)) : (5 + (gen() % 3));
            int secondaryBoost = (rarity == "Common") ? (2 + (gen() % 2)) : (3 + (gen() % 3));
            
            uniform_int_distribution<> statDis(0, secondaryStats.size() - 1);
            string secondaryStat = secondaryStats[statDis(gen)];
            
            armors.push_back(new Armor(armorNames[i], rarity, defBoost, secondaryBoost, secondaryStat));
        }
        
        // Create potions
        vector<string> potionStats = {"HP", "ATK", "DEF", "SPD", "LCK"};
        for (int i = 0; i < 10; ++i) {
            uniform_int_distribution<> statDis(0, potionStats.size() - 1);
            uniform_int_distribution<> boostDis(3, 5); // 6-9 points total
            
            string stat1 = potionStats[statDis(gen)];
            string stat2;
            do {
                stat2 = potionStats[statDis(gen)];
            } while (stat2 == stat1); // Ensure two different stats
            
            int boost1 = boostDis(gen);
            int boost2 = boostDis(gen);
            
            potions.push_back(new Potion(potionNames[i], boost1, boost2, stat1, stat2));
        }
    }
    
    Weapon* getRandomWeapon(const string& rarity) {
        vector<Weapon*> availableWeapons;
        for (auto weapon : weapons) {
            if (weapon->getRarity() == rarity) {
                availableWeapons.push_back(weapon);
            }
        }
        
        if (availableWeapons.empty()) return nullptr;
        
        uniform_int_distribution<> dis(0, availableWeapons.size() - 1);
        return availableWeapons[dis(gen)];
    }
    
    Armor* getRandomArmor(const string& rarity) {
        vector<Armor*> availableArmors;
        for (auto armor : armors) {
            if (armor->getRarity() == rarity) {
                availableArmors.push_back(armor);
            }
        }
        
        if (availableArmors.empty()) return nullptr;
        
        uniform_int_distribution<> dis(0, availableArmors.size() - 1);
        return availableArmors[dis(gen)];
    }
    
    Potion* getRandomPotion() {
        vector<Potion*> availablePotions;
        for (auto potion : potions) {
            if (!potion->isUsed()) { // Only return unused potions
                availablePotions.push_back(potion);
            }
        }

        if (availablePotions.empty()) return nullptr;
        
        uniform_int_distribution<> dis(0, availablePotions.size() - 1);
        return availablePotions[dis(gen)];
    }

    vector<Weapon*> getAllWeapons() const { return weapons; }
    vector<Armor*> getAllArmors() const { return armors; }
    vector<Potion*> getAllPotions() const { return potions; }

    // This method is for "giving" an item to a hero, not for removing from global pool.
    // The current design implies the Inventory *owns* all items, and heroes get pointers.
    // If an item is truly "taken" from the market, it would need to be removed from Inventory's vectors.
    // For now, heroes just get pointers to existing items.
};

//Battle class

class Battle {
private:
    vector<Hero*> heroes;
    vector<Enemy*> enemies;
    mt19937 rng; 

    size_t heroIndex = 0;
    size_t enemyIndex = 0;
    bool heroesTurn = true; // Indica qué equipo ataca ahora

public:
    Battle(vector<Hero*> heroes, vector<Enemy*> enemies)
        : heroes(heroes), enemies(enemies) {
        random_device rd;
        rng.seed(rd());
    }

    bool startBattle() {
        cout << "\n--- ¡Una batalla ha comenzado! ---" << endl;

        // Decide quién inicia (más SPD entre héroes y enemigos vivos)
        heroesTurn = decideFirstTurn();

        heroIndex = 0;
        enemyIndex = 0;

        while (!checkBattleEnd()) {
            cout << "\n--- TURNO ---" << endl;

            if (heroesTurn) {
                if (!performHeroTurn()) break;
            } else {
                if (!performEnemyTurn()) break;
            }

            heroesTurn = !heroesTurn;
        }

        displayBattleStatus();

        if (getWinner() == "Heroes") {
            cout << "\n¡Los héroes han ganado la batalla!" << endl;
            return true;
        } else {
            cout << "\n¡Los enemigos han ganado la batalla! Has perdido la partida." << endl;
            return false;
        }
    }
    
    // Displays current HP status of all combatants
    void displayBattleStatus() const { //Muestra en consola el estado actual de la batalla: Vida y estadísticas de héroes y enemigos.
        cout << "\n--- Estado de la Batalla ---" << endl;
        cout << "Héroes:" << endl;
        for (Hero* hero : heroes) {
            hero->displayStats();
        }
        cout << "Enemigos:" << endl;
        for (Enemy* enemy : enemies) {
            enemy->displayStats();
        }
        cout << "--------------------------" << endl;
    }

private:
    bool decideFirstTurn() {
        // Encuentra el héroe más rápido vivo
        int maxHeroSpd = -1;
        for (auto h : heroes) {
            if (h->isAlive() && h->getSpd() > maxHeroSpd)
                maxHeroSpd = h->getSpd();
        }
        // Encuentra el enemigo más rápido vivo
        int maxEnemySpd = -1;
        for (auto e : enemies) {
            if (e->isAlive() && e->getSpd() > maxEnemySpd)
                maxEnemySpd = e->getSpd();
        }
        // Empieza el equipo con mayor SPD
        return maxHeroSpd >= maxEnemySpd;
    }

    bool performHeroTurn() {
        Hero* currentHero = getNextAliveHero();
        if (!currentHero) return false; // No quedan héroes vivos

        cout << "\nEs el turno de " << currentHero->getName() << "." << endl;
        heroAction(currentHero);

        return true;
    }

    bool performEnemyTurn() {
        Enemy* currentEnemy = getNextAliveEnemy();
        if (!currentEnemy) return false; // No quedan enemigos vivos

        cout << "\nEs el turno de " << currentEnemy->getName() << "." << endl;
        enemyAction(currentEnemy);

        return true;
    }

    Hero* getNextAliveHero() {
        size_t startIndex = heroIndex;
        do {
            if (heroes[heroIndex]->isAlive()) {
                Hero* h = heroes[heroIndex];
                heroIndex = (heroIndex + 1) % heroes.size();
                return h;
            }
            heroIndex = (heroIndex + 1) % heroes.size();
        } while (heroIndex != startIndex);
        return nullptr; // No hay héroes vivos
    }

    Enemy* getNextAliveEnemy() {
        size_t startIndex = enemyIndex;
        do {
            if (enemies[enemyIndex]->isAlive()) {
                Enemy* e = enemies[enemyIndex];
                enemyIndex = (enemyIndex + 1) % enemies.size();
                return e;
            }
            enemyIndex = (enemyIndex + 1) % enemies.size();
        } while (enemyIndex != startIndex);
        return nullptr; // No hay enemigos vivos
    }

    void heroAction(Hero* hero) {
        int choice; //Método que maneja la decisión del jugador con su héroe (atacar o usar poción).
        while (true) {
            cout << hero->getName() << ", ¿qué quieres hacer?" << endl;
            cout << "1. Atacar" << endl;
            cout << "2. Usar poción" << endl;
            cout << "Opción: ";
            choice = getValidatedInput(1, 2); //Imprime opciones y obtiene la elección del jugador (entrada validada).

            if (choice == 1) {
                // Si elige atacar, primero busca enemigos vivos.
                vector<Enemy*> aliveEnemies;
                for(Enemy* e : enemies) {
                    if (e->isAlive()) {
                        aliveEnemies.push_back(e);
                    }
                }

                if (aliveEnemies.empty()) { //Si no hay enemigos vivos, lo informa y reinicia la elección.
                    cout << "No hay enemigos a quien atacar." << endl;
                    continue; // Re-prompt hero action
                }

                cout << "Selecciona un enemigo para atacar:" << endl;
                for (size_t i = 0; i < aliveEnemies.size(); ++i) {
                    cout << (i + 1) << ". " << aliveEnemies[i]->getName() << " (HP: " << aliveEnemies[i]->getHp() << ")" << endl;
                }
                int targetIndex;
                cout << "Objetivo: ";
                targetIndex = getValidatedInput(1, aliveEnemies.size()); //Muestra los enemigos disponibles y pide al usuario seleccionar uno.

                Enemy* targetEnemy = aliveEnemies[targetIndex - 1];
                if (hero->calculateHitChance(targetEnemy)) {
                    int damage = hero->calculateDamage(targetEnemy);
                    targetEnemy->takeDamage(damage);
                    cout << hero->getName() << " ataca a " << targetEnemy->getName() << " por " << damage << " de daño." << endl;
                    if (!targetEnemy->isAlive()) {
                        cout << targetEnemy->getName() << " ha sido derrotado!" << endl;
                    }
                } else {
                    cout << hero->getName() << " falló el ataque a " << targetEnemy->getName() << "." << endl;
                }
                break; // Ejecuta el ataque: calcula si acierta y el daño. Si el enemigo muere, lo informa. Finaliza el turno del héroe.

            } else if (choice == 2) { //Revisa qué pociones no han sido usadas por el héroe.
                // Use potion logic
                vector<Potion*> availablePotions;
                for (Potion* p : hero->getPotions()) {
                    if (!p->isUsed()) {
                        availablePotions.push_back(p);
                    }
                }

                if (availablePotions.empty()) { //Si no hay ninguna disponible, vuelve a pedir una acción.
                    cout << "No tienes pociones disponibles para usar." << endl;
                    continue; // Re-prompt hero action
                }

                cout << "Selecciona una poción para usar:" << endl;
                for (size_t i = 0; i < availablePotions.size(); ++i) {
                    cout << (i + 1) << ". ";
                    availablePotions[i]->displayInfo();
                }
                int potionIndex;
                cout << "Poción: ";
                potionIndex = getValidatedInput(1, availablePotions.size()); //Muestra la lista de pociones disponibles y deja al jugador elegir una.

                // Find the original index of the potion in the hero's main potions vector
                // This is necessary because hero->usePotion expects the original index
                int originalIndex = -1;
                for (size_t i = 0; i < hero->getPotions().size(); ++i) {
                    if (hero->getPotions()[i] == availablePotions[potionIndex - 1]) {
                        originalIndex = i;
                        break; //Busca el índice original de la poción elegida en el vector del héroe (porque usePotion necesita ese índice).
                    }
                }
                if (originalIndex != -1) {
                     hero->usePotion(originalIndex);
                     break; // Action completed
                } else {
                    cout << "Error interno al usar poción." << endl; // Should not happen
                    continue; //Si encuentra el índice, la poción es usada. Si no, se indica error y vuelve a empezar.
                }
            }
        }
    }


    void enemyAction(Enemy* enemy) { //Busca héroes vivos para atacar.
        // Find a random alive hero to attack
        vector<Hero*> aliveHeroes;
        for (Hero* hero : heroes) {
            if (hero->isAlive()) { 
                aliveHeroes.push_back(hero);
            }
        }

        if (aliveHeroes.empty()) {
            // This should ideally not happen if checkBattleEnd works correctly
            return; //Si no hay héroes vivos, termina sin hacer nada.
        }
        
        uniform_int_distribution<> dis(0, aliveHeroes.size() - 1);
        Hero* targetHero = aliveHeroes[dis(rng)]; //Elige un héroe aleatoriamente como objetivo.
        
        if (enemy->calculateHitChance(targetHero)) {
            int damage = enemy->calculateDamage(targetHero);
            targetHero->takeDamage(damage);
            cout << enemy->getName() << " ataca a " << targetHero->getName() << " por " << damage << " de daño." << endl;
            if (!targetHero->isAlive()) {
                cout << targetHero->getName() << " ha sido derrotado!" << endl;
            }
        } else {
            cout << enemy->getName() << " falló el ataque a " << targetHero->getName() << "." << endl;
        } //Ataca con la misma lógica que el héroe: calcula si acierta, daño, aplica daño y muestra resultado.
    }
    bool checkBattleEnd() const {
        bool heroesAlive = false;
        for (auto h : heroes)
            if (h->isAlive()) {
                heroesAlive = true;
                break;
            }

        bool enemiesAlive = false;
        for (auto e : enemies)
            if (e->isAlive()) {
                enemiesAlive = true;
                break;
            }

        return !(heroesAlive && enemiesAlive);
    }


    string getWinner() const {
        bool heroesAlive = false;
        for (auto h : heroes)
            if (h->isAlive()) {
                heroesAlive = true;
                break;
            }
        bool enemiesAlive = false;
        for (auto e : enemies)
            if (e->isAlive()) {
                enemiesAlive = true;
                break;
            }
        if (heroesAlive && !enemiesAlive) return "Heroes";
        if (!heroesAlive && enemiesAlive) return "Enemies";
        return "Ongoing";
    }
};


// ===== ROOM CLASS =====
class Room {
private:
    int roomNumber;
    vector<Enemy*> enemies;
    string roomType;
    bool isCleared;
    random_device rd;
    mt19937 gen;

public:
    Room(int number, const string& type) 
        : roomNumber(number), roomType(type), isCleared(false) {
        gen.seed(rd());
    }

    ~Room() {
        for (auto enemy : enemies) {
            delete enemy; // Rooms own their enemies
        }
    }

    void addEnemy(Enemy* enemy) {
        enemies.push_back(enemy);
    }

    vector<Enemy*> getEnemies() const { return enemies; }
    int getRoomNumber() const { return roomNumber; }
    string getRoomType() const { return roomType; }
    bool isRoomCleared() const { return isCleared; }
    void clearRoom() { isCleared = true; }

    void displayRoomInfo() const {
        cout << "\n--- Estás en la Sala " << roomNumber << " ---" << endl;
        cout << "Tipo de sala: " << roomType << endl;
        if (!enemies.empty()) {
            cout << "¡Enemigos a la vista!" << endl;
            for (const auto& enemy : enemies) {
                if (enemy->isAlive()) {
                    enemy->displayStats();
                }
            }
        } else {
            cout << "La sala parece tranquila por ahora..." << endl;
        }
    }

    // Example for getting a random item reward (needs an Inventory instance)
    // This method would typically be called by the Game class
    Item* getItemReward(Inventory* inventory, const string& rarity = "Common") {
        uniform_int_distribution<> dis(0, 2); // 0: Weapon, 1: Armor, 2: Potion
        int itemType = dis(gen);

        if (itemType == 0) {
            return inventory->getRandomWeapon(rarity);
        } else if (itemType == 1) {
            return inventory->getRandomArmor(rarity);
        } else {
            return inventory->getRandomPotion();
        }
    }
};

// ===== SCORE CLASS =====
struct Score {
    string playerName;
    int roomReached;
    int totalHealthLost;
    string timestamp;

    Score(string name = "", int room = 0, int health = 0, string ts = "") 
        : playerName(name), roomReached(room), totalHealthLost(health), timestamp(ts) {}

    // Operator for sorting: higher roomReached is better. If same room, lower healthLost is better.
    bool operator<(const Score& other) const {
        if (roomReached != other.roomReached) {
            return roomReached > other.roomReached; // Descending order for roomReached
        }
        return totalHealthLost < other.totalHealthLost; // Ascending order for totalHealthLost
    }
};

// ===== SCOREMANAGER CLASS =====
class ScoreManager {
private:
    vector<Score> scores;
    string filename;

public:
    ScoreManager(const string& fn = "leaderboard.txt") : filename(fn) {
        loadScores();
    }

    void loadScores() {
        scores.clear();
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string name, roomStr, healthStr, ts;
                getline(ss, name, ',');
                getline(ss, roomStr, ',');
                getline(ss, healthStr, ',');
                getline(ss, ts);

                scores.emplace_back(name, stoi(roomStr), stoi(healthStr), ts);
            }
            file.close();
            sortScores();
        } else {
            cout << "Advertencia: No se pudo abrir el archivo de leaderboard. Se creará uno nuevo si se guarda una puntuación." << endl;
        }
    }

    void saveScore(const string& playerName, int roomReached, int healthLost) {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        string timestamp(buffer);

        scores.emplace_back(playerName, roomReached, healthLost, timestamp);
        sortScores();

        ofstream file(filename);
        if (file.is_open()) {
            for (const auto& score : scores) {
                file << score.playerName << "," 
                     << score.roomReached << "," 
                     << score.totalHealthLost << "," 
                     << score.timestamp << endl;
            }
            file.close();
            cout << "Puntuación guardada exitosamente." << endl;
        } else {
            cout << "Error: No se pudo guardar la puntuación en el archivo." << endl;
        }
    }

    void displayLeaderboard(int limit = 10) const {
        cout << "\n--- TABLA DE CLASIFICACIÓN ---" << endl;
        if (scores.empty()) {
            cout << "No hay puntuaciones registradas aún." << endl;
            return;
        }
        
        cout << left << setw(3) << "#"
             << setw(20) << "Jugador"
             << setw(10) << "Salas"
             << setw(15) << "Vida Perdida"
             << setw(20) << "Fecha" << endl;
        cout << string(68, '-') << endl;

        for (size_t i = 0; i < min((size_t)limit, scores.size()); ++i) {
            cout << left << setw(3) << (i + 1)
                 << setw(20) << scores[i].playerName
                 << setw(10) << scores[i].roomReached
                 << setw(15) << scores[i].totalHealthLost
                 << setw(20) << scores[i].timestamp << endl;
        }
        cout << "------------------------------" << endl;
    }

    void sortScores() {
        sort(scores.begin(), scores.end());
    }
};

// ===== GAME CLASS (MAIN GAME LOGIC) =====
class Game {
private:
    vector<Hero*> availableHeroes;
    vector<Hero*> playerTeam;
    vector<Enemy*> availableEnemies;
    vector<Room*> dungeon;
    Inventory* inventory;
    string playerName;
    int currentRoomNumber;
    ScoreManager* scoreManager;
    random_device rd;
    mt19937 gen;

public:
    Game() : inventory(nullptr), currentRoomNumber(0) {
        gen.seed(rd());
        initializeAvailableCharacters();
        inventory = new Inventory(); // Initialize global inventory
        scoreManager = new ScoreManager();
    }

    ~Game() {
        for (auto hero : availableHeroes) delete hero;
        for (auto hero : playerTeam) delete hero; // Ensure playerTeam heroes are also deleted if they were copied
        for (auto enemy : availableEnemies) delete enemy;
        // Dungeon rooms own their enemies, so deleting rooms deletes enemies.
        for (auto room : dungeon) delete room; 
        delete inventory;
        delete scoreManager;
    }

    void startGame() {
        showMainMenu();
    }

    void showMainMenu() {
        int choice;
        do {
            cout << "\n=== SISAS: Natal Combat ===" << endl;
            cout << "1. Empezar Nueva Partida" << endl;
            cout << "2. Ver Tabla de Clasificacion" << endl;
            cout << "3. Salir" << endl;
            cout << "Opción: ";
            choice = getValidatedInput(1, 3);

            switch (choice) {
                case 1:
                    setupNewGame();
                    playGame();
                    break;
                case 2:
                    scoreManager->displayLeaderboard();
                    break;
                case 3:
                    cout << "¡Gracias por jugar SISAS! ¡Nos vemos!" << endl;
                    break;
            }
        } while (choice != 3);
    }

private:
    void initializeAvailableCharacters() {
        // Hero base stats (HP, ATK, DEF, SPD, LCK)
        availableHeroes.push_back(new Hero("Caleño", 100, 15, 10, 8, 7));
        availableHeroes.push_back(new Hero("Costeño", 110, 12, 12, 6, 9));
        availableHeroes.push_back(new Hero("Paisa", 90, 18, 8, 10, 5));
        availableHeroes.push_back(new Hero("Amazonas", 95, 14, 11, 7, 8));
        availableHeroes.push_back(new Hero("Llanero", 120, 13, 13, 5, 6));
        availableHeroes.push_back(new Hero("Chocoano", 85, 17, 9, 9, 7));

        // Available enemies (these will be copied for each room)
        // Soldier (HP, ATK, DEF, SPD, LCK)
        availableEnemies.push_back(new Enemy("El Mindo", 40, 8, 5, 7, 6, "Soldado"));
        availableEnemies.push_back(new Enemy("Betty la Fea", 45, 7, 6, 8, 7, "Soldado"));
        availableEnemies.push_back(new Enemy("Carlos Vives", 50, 9, 6, 6, 5, "Soldado"));
        availableEnemies.push_back(new Enemy("Diva Jessurum", 42, 9, 4, 9, 8, "Soldado"));
        availableEnemies.push_back(new Enemy("Falcao García", 55, 11, 7, 7, 6, "Soldado"));
        availableEnemies.push_back(new Enemy("Shakira", 48, 10, 5, 8, 7, "Soldado"));
        availableEnemies.push_back(new Enemy("Juanes", 52, 10, 6, 7, 6, "Soldado"));
        availableEnemies.push_back(new Enemy("Maluma", 47, 8, 7, 9, 5, "Soldado"));
        availableEnemies.push_back(new Enemy("J Balvin", 49, 9, 6, 8, 6, "Soldado"));
        availableEnemies.push_back(new Enemy("Karol G", 46, 10, 5, 7, 8, "Soldado"));
        availableEnemies.push_back(new Enemy("Gabo", 44, 8, 6, 9, 7, "Soldado"));
        availableEnemies.push_back(new Enemy("El Pibe Valderrama", 51, 10, 7, 6, 5, "Soldado"));


        // Mini-Boss (HP, ATK, DEF, SPD, LCK)
        availableEnemies.push_back(new Enemy("Pablo Escobar", 80, 25, 10,18,25, "Mini-Jefe"));
        availableEnemies.push_back(new Enemy("Alias Tiro Fijo", 75, 24, 11, 7,16, "Mini-Jefe"));
        availableEnemies.push_back(new Enemy("La Liendra", 90, 23, 9, 9,17, "Mini-Jefe"));

        // Final Boss (HP, ATK, DEF, SPD, LCK)
        availableEnemies.push_back(new Enemy("PETRO", 150, 35, 15, 10, 20, "Jefe Final"));
        availableEnemies.push_back(new Enemy("Gozo con Gonzo",90,45,25,18,16, "Jefe Final"));
    }
    
    void setupNewGame() {
        cout << "\n¡Bienvenido a SISAS!" << endl;
        cout << "¿Cuál es tu nombre, valiente aventurero? ";
        getline(cin, playerName);

        selectHeroes();
        initialMarket();
        initializeDungeon();
        currentRoomNumber = 0; // Reset room counter for new game
    }

    void selectHeroes() {
        playerTeam.clear(); // Clear previous team if any
        cout << "\n--- Selección de Héroes ---" << endl;
        cout << "Elige a 3 héroes para tu equipo." << endl;

        vector<Hero*> tempAvailableHeroes = availableHeroes; // Copy to remove selected ones
        
        for (int i = 0; i < 3; ++i) {
            while (true) {
                cout << "\nHéroe #" << (i + 1) << ":" << endl;
                for (size_t j = 0; j < tempAvailableHeroes.size(); ++j) {
                    cout << (j + 1) << ". ";
                    tempAvailableHeroes[j]->displayStats();
                }
                cout << "Elige un número: ";
                int choice = getValidatedInput(1, tempAvailableHeroes.size());

                // Create a deep copy of the hero to add to playerTeam
                Hero* chosenHero = new Hero(
                    tempAvailableHeroes[choice - 1]->getName(),
                    tempAvailableHeroes[choice - 1]->getMaxHp(), // Use maxHp for initial HP
                    tempAvailableHeroes[choice - 1]->getAtk(),
                    tempAvailableHeroes[choice - 1]->getDef(),
                    tempAvailableHeroes[choice - 1]->getSpd(),
                    tempAvailableHeroes[choice - 1]->getLck()
                );
                playerTeam.push_back(chosenHero);
                
                tempAvailableHeroes.erase(tempAvailableHeroes.begin() + choice - 1); // Remove selected hero
                cout << chosenHero->getName() << " se ha unido a tu equipo." << endl;
                break;
            }
        }
        cout << "\n¡Tu equipo está listo!" << endl;
        for (auto hero : playerTeam) {
            hero->displayStats();
        }
    }

    void initialMarket() {
        cout << "\n--- Mercado Inicial ---" << endl;
        cout << "¡Bienvenido al mercado! Puedes equipar a tus héroes con algunas armas y armaduras básicas." << endl;

        for (Hero* hero : playerTeam) {
            cout << "\nEquipando a " << hero->getName() << ":" << endl;
            
            // Offer a common weapon
            Weapon* weaponOffer = inventory->getRandomWeapon("Common");
            if (weaponOffer) {
                cout << "¿Quieres equipar " << weaponOffer->getName() << " (ATK +" << weaponOffer->getStatBoost1() << ") en " << hero->getName() << "? (1. Sí / 2. No): ";
                int choice = getValidatedInput(1, 2);
                if (choice == 1) {
                    hero->equipWeapon(weaponOffer);
                    cout << hero->getName() << " equipa " << weaponOffer->getName() << "." << endl;
                }
            } else {
                cout << "No hay armas comunes disponibles." << endl;
            }

            // Offer a common armor
            Armor* armorOffer = inventory->getRandomArmor("Common");
            if (armorOffer) {
                cout << "¿Quieres equipar " << armorOffer->getName() << " (DEF +" << armorOffer->getStatBoost1() << ") en " << hero->getName() << "? (1. Sí / 2. No): ";
                int choice = getValidatedInput(1, 2);
                if (choice == 1) {
                    hero->equipArmor(armorOffer);
                    cout << hero->getName() << " equipa " << armorOffer->getName() << "." << endl;
                }
            } else {
                cout << "No hay armaduras comunes disponibles." << endl;
            }

        }
        cout << "\nMercado inicial completado." << endl;
    }

    void initializeDungeon() {
        dungeon.clear(); // Clear previous dungeon if any
        for (int i = 1; i <= 10; ++i) {
            Room* room = new Room(i, "Normal"); // Default room type

            // Populate enemies based on room number and type
            if (i == 3) { // Special event room: Mini-boss
                room->clearRoom(); // Clear existing generic enemies if any
                room->addEnemy(createEnemyCopy("Pablo Escobar"));
                room->addEnemy(createEnemyCopy("La Liendra"));
                // Add more enemies to make it a mini-boss fight if desired
            } else if (i == 6) { // Special event room: Mini-boss
                room->clearRoom();
                room->addEnemy(createEnemyCopy("Alias Tiro Fijo"));
                room->addEnemy(createEnemyCopy("El Mindo"));
                room->addEnemy(createEnemyCopy("Betty la Fea"));
            } else if (i == 8) { // Special event room: Reward room
                room->clearRoom(); // No enemies in this specific reward room example
                room->addEnemy(createEnemyCopy("Carlos Vives")); // Example: still have enemies
                room->addEnemy(createEnemyCopy("Diva Jessurum"));
            } else if (i == 10) { // Final boss room
                room->clearRoom();
                room->addEnemy(createEnemyCopy("Gozo con Gonzo"));
                room->addEnemy(createEnemyCopy("PETRO")); // Final boss
            } else { // Regular rooms
                // Random number of enemies for regular rooms (1 to 3)
                uniform_int_distribution<> numEnemiesDis(2,3);
                int numEnemies = numEnemiesDis(gen);
                for (int e = 0; e < numEnemies; ++e) {
                    uniform_int_distribution<> enemyTypeDis(0, availableEnemies.size() - 4); // Exclude mini-bosses and final boss for regular rooms
                    room->addEnemy(createEnemyCopy(availableEnemies[enemyTypeDis(gen)]->getName()));
                }
            }
            dungeon.push_back(room);
        }
    }

    Enemy* createEnemyCopy(const string& name) {
        for (Enemy* enemy : availableEnemies) {
            if (enemy->getName() == name) {
                // Return a new instance, deeply copied
                return new Enemy(enemy->getName(), enemy->getMaxHp(), enemy->getAtk(), 
                                 enemy->getDef(), enemy->getSpd(), enemy->getLck(), enemy->getType());
            }
        }
        return nullptr; // Should not happen
    }

    void playGame() {
        cout << "\n--- ¡Comienza la Aventura en la Mazmorra! ---" << endl;
        for (currentRoomNumber = 0; currentRoomNumber < dungeon.size(); ++currentRoomNumber) {
            Room* currentRoom = dungeon[currentRoomNumber];
            currentRoom->displayRoomInfo();

            // Battle in the room if there are enemies
            if (!currentRoom->getEnemies().empty()) {
                Battle battle(playerTeam, currentRoom->getEnemies());
                bool heroesWon = battle.startBattle();

                if (!heroesWon) {
                    cout << "\nTu equipo ha sido derrotado. Fin de la partida." << endl;
                    endGame();
                    return;
                } else {
                    currentRoom->clearRoom();
                    cout << "¡Has limpiado la Sala " << (currentRoomNumber + 1) << "!" << endl;
                    handlePostBattleRewards();
                }
            } else {
                cout << "La sala " << (currentRoomNumber + 1) << " está vacía." << endl;
            }

            handleSpecialEvents(currentRoomNumber + 1); // Room numbers are 1-indexed for display

            // Check if game ends (e.g. after Room 10)
            if (currentRoomNumber == 9) { // Last room (index 9 is Room 10)
                cout << "\n¡Has completado todas las salas de la mazmorra!" << endl;
                endGame();
                return;
            }

            cout << "\n¿Listo para la siguiente sala? (Presiona Enter)";
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Consume pending newline
            cin.get(); // Wait for user to press enter
        }
    }

    void handlePostBattleRewards() {
        cout << "\n--- Recompensas ---" << endl;
        // Heal heroes by a percentage of max HP
        for (Hero* hero : playerTeam) {
            hero->boostStats(2.0f); // 2% ATK/DEF boost
        }

        cout << "Todos los héroes han recibido un pequeño aumento de estadísticas." << endl;
    }


    void handleSpecialEvents(int roomNum) {
        if (roomNum == 3) {
            cout << "\n--- EVENTO ESPECIAL: Sala 3 ---" << endl;
            cout << "¡Parece que hay un cofre especial por aquí!" << endl;
            Item* chestItem = inventory->getRandomWeapon("Rare"); // Guaranteed rare weapon
            if (!chestItem) chestItem = inventory->getRandomArmor("Rare");
            if (!chestItem) chestItem = inventory->getRandomPotion();

            if (chestItem) {
                cout << "Has encontrado en el cofre: ";
                chestItem->displayInfo();
                cout << "¿A quién quieres darle este tesoro? (0 para no dar a nadie)" << endl;
                for (size_t i = 0; i < playerTeam.size(); ++i) {
                    cout << (i + 1) << ". " << playerTeam[i]->getName() << endl;
                }
                int heroChoice = getValidatedInput(0, playerTeam.size());
                if (heroChoice > 0) {
                    Hero* chosenHero = playerTeam[heroChoice - 1];
                     if (Weapon* w = dynamic_cast<Weapon*>(chestItem)) {
                        chosenHero->equipWeapon(w);
                        cout << chosenHero->getName() << " equipa " << chestItem->getName() << "." << endl;
                    } else if (Armor* a = dynamic_cast<Armor*>(chestItem)) {
                        chosenHero->equipArmor(a);
                        cout << chosenHero->getName() << " equipa " << chestItem->getName() << "." << endl;
                    } else if (Potion* p = dynamic_cast<Potion*>(chestItem)) {
                        chosenHero->addPotion(p);
                        cout << chosenHero->getName() << " obtiene " << chestItem->getName() << "." << endl;
                    }
                } else {
                    cout << "Decides dejar el tesoro. Una pena." << endl;
                }
            } else {
                cout << "El cofre estaba vacío." << endl;
            }
        } else if (roomNum == 6) {
            cout << "\n--- EVENTO ESPECIAL: Sala 6 ---" << endl;
            cout << "¡Un tesoro ancestral te espera!" << endl;
            Item* treasureItem = inventory->getRandomWeapon("Rare"); // Guaranteed rare weapon
            if (!treasureItem) treasureItem = inventory->getRandomArmor("Rare");
            if (!treasureItem) treasureItem = inventory->getRandomPotion();

            if (treasureItem) {
                cout << "Has descubierto un Tesoro: ";
                treasureItem->displayInfo();
                 cout << "¿A quién quieres darle este tesoro? (0 para no dar a nadie)" << endl;
                for (size_t i = 0; i < playerTeam.size(); ++i) {
                    cout << (i + 1) << ". " << playerTeam[i]->getName() << endl;
                }
                int heroChoice = getValidatedInput(0, playerTeam.size());
                if (heroChoice > 0) {
                    Hero* chosenHero = playerTeam[heroChoice - 1];
                     if (Weapon* w = dynamic_cast<Weapon*>(treasureItem)) {
                        chosenHero->equipWeapon(w);
                        cout << chosenHero->getName() << " equipa " << treasureItem->getName() << "." << endl;
                    } else if (Armor* a = dynamic_cast<Armor*>(treasureItem)) {
                        chosenHero->equipArmor(a);
                        cout << chosenHero->getName() << " equipa " << treasureItem->getName() << "." << endl;
                    } else if (Potion* p = dynamic_cast<Potion*>(treasureItem)) {
                        chosenHero->addPotion(p);
                        cout << chosenHero->getName() << " obtiene " << treasureItem->getName() << "." << endl;
                    }
                } else {
                    cout << "Decides dejar el tesoro. Una pena." << endl;
                }
            } else {
                cout << "El tesoro estaba vacío." << endl;
            }
        } else if (roomNum == 8) {
            cout << "\n--- EVENTO ESPECIAL: Sala 8 ---" << endl;
            cout << "Un misterioso ermitaño te ofrece una bendición." << endl;
            cout << "Tus héroes recuperan su HP." << endl;
            for (Hero* hero : playerTeam) {
                int healAmount = static_cast<int>(hero->getMaxHp()); // Full heal
                hero->heal(healAmount);
                cout << hero->getName() << " recupera " << healAmount << " HP. (HP: " << hero->getHp() << "/" << hero->getMaxHp() << ")" << endl;
            }
        }
    }

    void endGame() {
        int totalHealthLost = 0;
        for (Hero* hero : playerTeam) {
            totalHealthLost += hero->getTotalHealthLost();
        }

        scoreManager->saveScore(playerName, currentRoomNumber + 1, totalHealthLost); // +1 because currentRoomNumber is 0-indexed
        scoreManager->displayLeaderboard();
        
        // Reset hero stats and potions for next game if starting again
        for (Hero* hero : playerTeam) {
            hero->resetPotionEffects(); // This should also reset stats if needed
            hero->heal(hero->getMaxHp()); // Full heal
        }
    }
};

int main() {
    // Seed the random number generator once for the whole program
    srand(time(0)); 

    Game game;
    game.startGame();

    return 0;
}

