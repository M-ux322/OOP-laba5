#include "dynamic_array.h"
#include <iostream>
#include <string>

struct Person {
    std::string name;
    int age;
    double salary;
    
    Person() : name(""), age(0), salary(0.0) {}
    Person(const std::string& n, int a, double s) : name(n), age(a), salary(s) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        return os << "Person{name: " << p.name << ", age: " << p.age << ", salary: " << p.salary << "}";
    }
};

int main() {
    dynamic_list_memory_resource mr;
    
    std::cout << "=== Demonstration with int ===" << std::endl;
    dynamic_array<int> int_arr(&mr);
    
    for (int i = 0; i < 10; ++i) {
        int_arr.push_back(i * i);
    }
    
    std::cout << "Int array: ";
    for (auto it = int_arr.begin(); it != int_arr.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\n=== Demonstration with Person ===" << std::endl;
    dynamic_array<Person> person_arr(&mr);
    
    person_arr.emplace_back("Alice", 25, 50000.0);
    person_arr.emplace_back("Bob", 30, 60000.0);
    person_arr.emplace_back("Charlie", 35, 70000.0);
    
    std::cout << "Person array: " << std::endl;
    for (const auto& person : person_arr) {
        std::cout << "  " << person << std::endl;
    }
    
    std::cout << "\n=== Iterator demonstration ===" << std::endl;
    auto it = person_arr.begin();
    std::cout << "First person: " << *it << std::endl;
    ++it;
    std::cout << "Second person: " << *it << std::endl;
    
    return 0;
}
