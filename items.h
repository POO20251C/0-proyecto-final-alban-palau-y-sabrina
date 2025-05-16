#items.h

class Item {
protected:
    std::string nombre;
public:
    Item(const std::string& nombre);
    virtual ~Item() {}
    virtual void usar() = 0;
    std::string getNombre() const;
}
