#include <iostream>

template<class T>
class array
{
    T * data;
    //size_t capacity;
    size_t num_entries;
public:  //

    size_t capacity;
    
    array()
        :data(0), capacity(0), num_entries(0)
    {
    }

    array(size_t n)
        :data(new T[n]), capacity(n), num_entries(n)
    {
    }

    ~array()
    {
        delete [] data;
    }

    T& operator[](size_t n)
    {
        return data[n];
    }

    const T& operator[](size_t n) const
    {
        return data[n];
    }

    size_t size() const
    {
        return num_entries;
    }
    
    void resize(size_t n)
    {

        capacity = capacity + n;
        T * new_data = new T[capacity + n];
        for(size_t i = 0; i < num_entries; i++){
                new_data[i] = data[i];
        }
        delete [] data;
        data = new_data;
    }

    void append(const T& item)
    {   

        if(num_entries >= capacity){
            if(capacity == 0){
                resize(1);
            }

            else{
                resize(num_entries * 2);
            }
            num_entries++;
            data[num_entries - 1] = item;
        }
    }

    void clear()
    {
        delete [] data;
        num_entries = 0;
        capacity = 0;
    }
};

int main()
{
    array<int> a;
    a.append(10);
     std::cout << "capacity: " << a.capacity << std:: endl;
    std::cout << "Size: " << a.size() << std:: endl;
    a.append(20);
     std::cout << "capacity: " << a.capacity << std:: endl;
    std::cout << "Size: " << a.size() << std:: endl;

    for(size_t i = 0; i < 100; i++)
        a.append(a[i]);

    long sum = 0;

    std::cout << "Entering second for loop Size: " << a.size() << std:: endl;
    for(size_t i = 0; i < a.size(); i++){
        std::cout << "capacity: " << a.capacity << std:: endl;
        std::cout << "Size: " << a.size() << std:: endl;

        sum += a[i];
    }

    std::cout << "sum: " << sum << std::endl;

    return 0;
}
