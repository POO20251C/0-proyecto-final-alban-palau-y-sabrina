


// Utility function for user input
int getValidatedInput(int min, int max) {
    int choice;
    while (!(cin >> choice) || choice < min || choice > max) {
        cout << "Entrada inválida. Por favor, ingresa un número entre " << min << " y " << max << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear remaining input
    return choice;
}

















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


