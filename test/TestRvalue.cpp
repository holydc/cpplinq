#include <iostream>
#include <string>

#include "Enumerable.h"

using cpplinq::Enumerable;

namespace rvalue {
void TestAggregate() {
    {
        // Determine whether any string in the array is longer than "banana".
        auto longestName = Enumerable<std::string>{"apple", "mango", "orange", "passionfruit", "grape"}
            .Aggregate(
                std::string{"banana"},
                [] (std::string longest, const std::string& next) {
                    return (next.size() > longest.size()) ? next : longest;
                },
                // Return the final result as an upper case string.
                [] (const std::string& fruit) {
                    return Enumerable(fruit).Select([] (char x) -> char { return toupper(x); }).ToContainer<std::basic_string>();
                });

        std::cout << "The fruit with the longest name is " << longestName << std::endl;
        // output:
        //     The fruit with the longest name is PASSIONFRUIT
    }
    {
        // Count the even numbers in the array, using a seed value of 0.
        auto numEven = Enumerable{4, 8, 8, 3, 9, 0, 7, 8, 2}
            .Aggregate(0, [] (int total, int next) {
                return ((next % 2) == 0) ? (total + 1) : total;
            });

        std::cout << "The number of even integers is: " << numEven << std::endl;
        // output:
        //     The number of even integers is: 6
    }
}

void TestAll() {
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        // Determine whether all pet names in the array start with 'B'.
        auto allStartWithB = Enumerable<Pet>{{"Barley", 10}, {"Boots", 4}, {"Whiskers", 4}}
            .All([] (const Pet& pet) {
                auto itr = std::begin(pet.Name);
                return (itr != std::end(pet.Name)) && (*itr == 'B');
            });

        std::cout << (allStartWithB ? "All" : "Not all") << " pet names start with 'B'" << std::endl;
        // output:
        //     Not all pet names start with 'B'
    }
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        struct Person {
            std::string LastName;
            std::vector<Pet> Pets;
        };

        // Determine which people have pets that are all older than 5.
        Person persons[] = {
            {"Haas", {{"Barley", 10}, {"Boots", 14}, {"Whiskers", 6}}},
            {"Fakhouri", {{"Snowball", 1}}},
            {"Antebi", {{"Belle", 8}}},
            {"Philips", {{"Sweetie", 3}, {"Rover", 13}}},
        };
        auto names = Enumerable<Person>{persons}
            .Where([] (const Person& person) {
                return Enumerable(person.Pets).All([] (const Pet& pet) { return pet.Age > 5; });
            })
            .Select([] (const Person& person) { return person.LastName; });

        for (auto&& name : names) {
            std::cout << name << std::endl;
        }
        // output:
        //     Haas
        //     Antebi
    }
}

void TestAny() {
    {
        auto hasElements = Enumerable{1, 2}.Any();

        std::cout << "The list " << (hasElements ? "is not" : "is") << " empty" << std::endl;
        // output:
        //     The list is not empty
    }
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        struct Person {
            std::string LastName;
            std::vector<Pet> Pets;
        };

        // Determine which people have a non-empty Pet array.
        Person persons[] = {
            {"Haas", {{"Barley", 10}, {"Boots", 14}, {"Whiskers", 6}}},
            {"Fakhouri", {{"Snowball", 1}}},
            {"Antebi", {}},
            {"Philips", {{"Sweetie", 3}, {"Rover", 13}}},
        };
        auto names = Enumerable<Person>{persons}
            .Where([] (const Person& person) { return Enumerable(person.Pets).Any(); })
            .Select([] (const Person& person) { return person.LastName; });

        for (auto&& name : names) {
            std::cout << name << std::endl;
        }
        // output:
        //     Haas
        //     Fakhouri
        //     Philips
    }
}

void TestAppend() {
    {
        // Creating a list of numbers.
        Enumerable numbers{1, 2, 3, 4};

        // Trying to append any value of the same type.
        numbers.Append(5);

        // It doesn't work because the original list has not been changed.
        std::copy(std::begin(numbers), std::end(numbers), std::ostream_iterator<decltype(numbers)::value_type>{std::cout, ","});
        std::cout << std::endl;
        // output:
        //     1,2,3,4,

        // It works now because we are using a changed copy of the original list.
        // TODO Console.WriteLine(string.Join(", ", numbers.Append(5)));

        // If you prefer, you can create a new list explicitly.
        auto newNumbers = numbers.Append(5);

        // And then write to the console output.
        std::copy(std::begin(newNumbers), std::end(newNumbers), std::ostream_iterator<decltype(newNumbers)::value_type>{std::cout, ","});
        std::cout << std::endl;
        // output:
        //     1,2,3,4,5,
    }
}

void TestConcat() {
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        auto query = Enumerable<Pet>{{"Barley", 8}, {"Boots", 4}, {"Whiskers", 1}}
            .Select([] (const Pet& cat) { return cat.Name; })
            .Concat(Enumerable<Pet>{{"Bounder", 3}, {"Snoopy", 14}, {"Fido", 9}}.Select([] (const Pet& dog) { return dog.Name; }));

        for (auto&& name : query) {
            std::cout << name << std::endl;
        }
        // output:
        //     Barley
        //     Boots
        //     Whiskers
        //     Bounder
        //     Snoopy
        //     Fido
    }
}

void TestContains() {
    {
        std::string fruit{"mango"};
        auto hasMango = Enumerable<std::string>{"apple", "banana", "mango", "orange", "passionfruit", "grape"}
            .Contains(fruit);

        std::cout << "The array " << (hasMango ? "does" : "does not") << " contain '" << fruit << "'" << std::endl;
        // output:
        //     The array does contain 'mango'
    }
    {
        struct Product {
            std::string Name;
            int Code;

            struct EqualityComparer {
                bool operator()(const Product& lhs, const Product& rhs) const {
                    return (&lhs == &rhs) || (lhs.Code == rhs.Code);
                }
            };
        };

        Product fruits[] = {
            {"apple", 9}, {"orange", 4}, {"lemon", 12}
        };

        Product apple{"apple", 9};
        Product kiwi{"kiwi", 8};

        auto hasApple = Enumerable(fruits).Contains(apple, Product::EqualityComparer{});
        auto hasKiwi = Enumerable(fruits).Contains(kiwi, Product::EqualityComparer{});

        std::cout << std::boolalpha;
        std::cout << "Apple? " << hasApple << std::endl;
        std::cout << "Kiwi? " << hasKiwi << std::endl;
        // output:
        //     Apple? true
        //     Kiwi? false
    }
}

void TestCount() {
    {
        auto numberOfFruits = Enumerable<std::string>{"apple", "banana", "mango", "orange", "passionfruit", "grape"}.Count();

        std::cout << "There are " << numberOfFruits << " fruits in the collection" << std::endl;
        // output:
        //     There are 6 fruits in the collection
    }
    {
        struct Pet {
            std::string Name;
            bool Vaccinated;
        };

        auto numberUnvaccinated = Enumerable<Pet>{{"Barley", true}, {"Boots", false}, {"Whiskers", false}}
            .Count([] (const Pet& pet) { return !pet.Vaccinated; });

        std::cout << "There are " << numberUnvaccinated << " unvaccinated animals" << std::endl;
        // output:
        //     There are 2 unvaccinated animals
    }
}

void TestDefaultIfEmpty() {
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        Pet defaultPet{"Default Pet", 0};

        for (auto&& pet : Enumerable<Pet>{{"Barley", 8}, {"Boots", 4}, {"Whiskers", 1}}.DefaultIfEmpty(defaultPet)) {
            std::cout << "Name: " << pet.Name << std::endl;
        }
        // output:
        //     Name: Barley
        //     Name: Boots
        //     Name: Whiskers

        for (auto&& pet : Enumerable<Pet>{}.DefaultIfEmpty(defaultPet)) {
            std::cout << "Name: " << pet.Name << std::endl;
        }
        // output:
        //     Name: Default Pet
    }
}

void TestDistinct() {
    {
        auto distinctAges = Enumerable{21, 46, 46, 55, 17, 21, 55, 55}.Distinct();

        std::cout << "Distinct ages:" << std::endl;
        for (auto&& age : distinctAges) {
            std::cout << age << std::endl;
        }
        // output:
        //     Distinct ages:
        //     21
        //     46
        //     55
        //     17
    }
    {
        struct Product {
            std::string Name;
            int Code;

            // Test DistinctEqual.
            bool operator==(const Product& rhs) const {
                return (this == &rhs) || ((Name == rhs.Name) && (Code == rhs.Code));
            }
        };

        // Exclude duplicates.
        auto noduplicates = Enumerable<Product>{{"apple", 9}, {"orange", 4}, {"apple", 9}, {"lemon", 12}}.Distinct();

        for (auto&& product : noduplicates) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     apple 9
        //     orange 4
        //     lemon 12
    }
    {
        struct Age {
            int value;

            // Test DistinctLess.
            bool operator<(const Age& rhs) const {
                return (this != &rhs) && (value < rhs.value);
            }
        };

        auto distinctAges = Enumerable<Age>{{21}, {46}, {46}, {55}, {17}, {21}, {55}, {55}}.Distinct();

        std::cout << "Distinct ages:" << std::endl;
        for (auto&& age : distinctAges) {
            std::cout << age.value << std::endl;
        }
        // output:
        //     Distinct ages:
        //     21
        //     46
        //     55
        //     17
    }
}

void TestElementAt() {
    {
        constexpr int index = 2;

        auto name = Enumerable<std::string>{"Hartono, Tommy", "Adams, Terry", "Andersen, Henriette Thaulow", "Hedlund, Magnus", "Ito, Shu"}
            .ElementAt(index, "");

        std::cout << "The name chosen at index " << index << " is '" << name << "'" << std::endl;
        // output:
        //     The name chosen at index 2 is 'Andersen, Henriette Thaulow'
    }
    {
        constexpr int index = 5566;

        auto name = Enumerable<std::string>{"Hartono, Tommy", "Adams, Terry", "Andersen, Henriette Thaulow", "Hedlund, Magnus", "Ito, Shu"}
            .ElementAt(index, "<no name at this index>");

        std::cout << "The name chosen at index " << index << " is '" << name << "'" << std::endl;
        // output:
        //     The name chosen at index 1000 is '<no name at this index>'
    }
}

void TestEmpty() {
    {
        std::vector<std::string> names1{"Hartono, Tommy"};
        std::vector<std::string> names2{"Adams, Terry", "Andersen, Henriette Thaulow", "Hedlund, Magnus", "Ito, Shu"};
        std::vector<std::string> names3{"Solanki, Ajay", "Hoeing, Helge", "Andersen, Henriette Thaulow", "Potra, Cristina", "Iallo, Lucio"};

        auto allNames = Enumerable{names1, names2, names3}
            .Aggregate(Enumerable<std::string>::Empty(), [] (Enumerable<std::string> current, const std::vector<std::string>& next) {
                return (std::size(next) > 3) ? std::move(current).Union(next) : current;
            });

        for (auto&& name : allNames) {
            std::cout << name << std::endl;
        }
        // output:
        //     Adams, Terry
        //     Andersen, Henriette Thaulow
        //     Hedlund, Magnus
        //     Ito, Shu
        //     Solanki, Ajay
        //     Hoeing, Helge
        //     Potra, Cristina
        //     Iallo, Lucio
    }
}

void TestExcept() {
    {
        auto onlyInFirstSet = Enumerable{2.0, 2.0, 2.1, 2.2, 2.3, 2.3, 2.4, 2.5}.Except({2.2});

        for (auto&& number : onlyInFirstSet) {
            std::cout << number << std::endl;
        }
        // output:
        //     2
        //     2.1
        //     2.3
        //     2.4
        //     2.5
    }
    {
        struct Product {
            std::string Name;
            int Code;

            // Test ExceptLess.
            bool operator<(const Product& rhs) const {
                return (Name < rhs.Name) || ((Name == rhs.Name) && (Code < rhs.Code));
            }
        };

        // Get all the elements from the first array except for the elements from the second array.
        auto except = Enumerable<Product>{{"apple", 9}, {"orange", 4}, {"lemon", 12}}.Except({{"apple", 9}});

        for (auto&& product : except) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     orange 4
        //     lemon 12
    }
    {
        struct Product {
            std::string Name;
            int Code;

            // Test ExceptLinear.
            bool operator==(const Product& rhs) const {
                return (Name == rhs.Name) && (Code == rhs.Code);
            }
        };

        // Get all the elements from the first array except for the elements from the second array.
        auto except = Enumerable<Product>{{"apple", 9}, {"orange", 4}, {"lemon", 12}}.Except({{"apple", 9}});

        for (auto&& product : except) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     orange 4
        //     lemon 12
    }
}

void TestFirst() {
    {
        auto first = Enumerable{9, 34, 65, 92, 87, 435, 3, 54, 83, 23, 87, 435, 67, 12, 19}
            .First(5566);

        std::cout << first << std::endl;
        // output:
        //     9
    }
    {
        auto first = Enumerable{9, 34, 65, 92, 87, 435, 3, 54, 83, 23, 87, 435, 67, 12, 19}
            .First([] (int number) { return number > 80; }, 5566);

        std::cout << first << std::endl;
        // output:
        //     92
    }
    {
        auto first = Enumerable<int>{}.First(5566);

        std::cout << first << std::endl;
        // output:
        //     5566
    }
    {
        std::string names[] = {"Hartono, Tommy", "Adams, Terry", "Andersen, Henriette Thaulow", "Hedlund, Magnus", "Ito, Shu"};

        auto firstLongName = Enumerable(names).First([] (const std::string& name) { return name.size() > 20; }, "");
        std::cout << "The first long name is '" << firstLongName << "'" << std::endl;
        // output:
        //     The first long name is 'Andersen, Henriette Thaulow'

        auto firstVeryLongName = Enumerable(names).First([] (const std::string& name) { return name.size() > 30; }, "");
        std::cout << "There is " << (firstVeryLongName.empty() ? "not a" : "a") << " name longer than 30 characters" << std::endl;
        // output:
        //     There is not a name longer than 30 characters
    }
}

void TestGroupBy() {
    {
        struct Pet {
            std::string Name;
            double Age;
        };

        struct Record {
            int Key;
            size_t Count;
            double Min;
            double Max;
        };

        auto query =
            // Create a list of pets.
            Enumerable<Pet>{{"Barley", 8.3}, {"Boots", 4.9}, {"Whiskers", 1.5}, {"Daisy", 4.3}}
            // Group Pet.Age values by the Math.Floor of the age.
            // Then project an anonymous type from each group
            // that consists of the key, the count of the group's
            // elements, and the minimum and maximum age in the group.
            .GroupBy(
                [] (const Pet& pet) { return static_cast<int>(pet.Age); },
                [] (const Pet& pet) { return pet.Age; },
                [] (int baseAge, const Enumerable<double>& ages) {
                    std::multiset<double> orderedAges{std::begin(ages), std::end(ages)};
                    return Record{baseAge, orderedAges.size(), *orderedAges.begin(), *orderedAges.rbegin()};
                });

        // Iterate over each anonymous type.
        for (auto&& result : query) {
            std::cout << std::endl << "Age group: " << result.Key << std::endl;
            std::cout << "Number of pets in this age group: " << result.Count << std::endl;
            std::cout << "Minimum age: " << result.Min << std::endl;
            std::cout << "Maximum age: " << result.Max << std::endl;
        }
        // output:
        //
        //     Age group: 8
        //     Number of pets in this age group : 1
        //     Minimum age : 8.3
        //     Maximum age : 8.3
        //
        //     Age group : 4
        //     Number of pets in this age group : 2
        //     Minimum age : 4.3
        //     Maximum age : 4.9
        //
        //     Age group : 1
        //     Number of pets in this age group : 1
        //     Minimum age : 1.5
        //     Maximum age : 1.5
    }
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        auto query =
            // Create a list of pets.
            Enumerable<Pet>{{"Barley", 8}, {"Boots", 4}, {"Whiskers", 1}, {"Daisy", 4}}
            // Group the pets using Age as the key value
            // and selecting only the pet's Name for each value.
            .GroupBy(
                [] (const Pet& pet) { return pet.Age; },
                [] (const Pet& pet) { return pet.Name; });

        // Iterate over each IGrouping in the collection.
        for (auto&& petGroup : query) {
            // Print the key value of the IGrouping.
            std::cout << petGroup.Key() << std::endl;
            // Iterate over each value in the
            // IGrouping and print the value.
            for (auto&& name : petGroup) {
                std::cout << "  " << name << std::endl;
            }
        }
        // output:
        //     8
        //       Barley
        //     4
        //       Boots
        //       Daisy
        //     1
        //       Whiskers
    }
    {
        struct Integer {
            int val;

            Integer(int value) : val{value} {
            }

            operator std::string() const {
                return std::to_string(val);
            }

            // Test GroupByLess.
            bool operator<(const Integer& rhs) const {
                return val < rhs.val;
            }
        };

        auto query = Enumerable<Integer>{1, 2, 2, 3, 3, 3}
            .GroupBy(
                [] (const Integer& x) { return x; },
                [] (const Integer& x) { return x; });

        for (auto&& group : query) {
            std::cout << "Key:" << (std::string)group.Key() << " Count:" << group.Count() << std::endl;
        }
        // output:
        //     Key:1 Count:1
        //     Key:2 Count:2
        //     Key:3 Count:3
    }
    {
        struct Integer {
            int val;

            Integer(int value) : val{value} {
            }

            operator std::string() const {
                return std::to_string(val);
            }

            // Test GroupByEqual.
            bool operator==(const Integer& rhs) const {
                return val == rhs.val;
            }
        };

        auto query = Enumerable<Integer>{1, 2, 2, 3, 3, 3}
            .GroupBy(
                [] (const Integer& x) { return x; },
                [] (const Integer& x) { return x; });

        for (auto&& group : query) {
            std::cout << "Key:" << (std::string)group.Key() << " Count:" << group.Count() << std::endl;
        }
        // output:
        //     Key:1 Count:1
        //     Key:2 Count:2
        //     Key:3 Count:3
    }
}

void TestGroupJoin() {
    {
        struct Person {
            std::string Name;

            bool operator==(const Person& rhs) const {
                return Name == rhs.Name;
            }

            // Test GroupJoinHash.
            struct hash {
                size_t operator()(const Person& x) const {
                    return std::hash<std::string>{}(x.Name);
                }
            };
        };

        struct Pet {
            std::string Name;
            Person* Owner;
        };

        struct Result {
            std::string OwnerName;
            Enumerable<std::string> Pets;
        };

        Person magnus{"Hedlund, Magnus"};
        Person terry{"Adams, Terry"};
        Person charlotte{"Weiss, Charlotte"};
        Person john{"Doe, John"};

        Pet barley{"Barley", &terry};
        Pet boots{"Boots", &terry};
        Pet whiskers{"Whiskers", &charlotte};
        Pet daisy{"Daisy", &magnus};

        // Create a list where each element is an anonymous
        // type that contains a person's name and
        // a collection of names of the pets they own.
        auto query = Enumerable{magnus, terry, charlotte, john, terry}
            .GroupJoinHash<Person::hash>(
                {barley, boots, whiskers, daisy},
                [] (const Person& person) { return person; },
                [] (const Pet& pet) { return *pet.Owner; },
                [] (const Person& person, const Enumerable<Pet>& petCollection) {
                    return Result{person.Name, petCollection.Select([] (const Pet& pet) { return pet.Name; })};
                });

        for (auto&& obj : query) {
            // Output the owner's name.
            std::cout << obj.OwnerName << ':' << std::endl;
            // Output each of the owner's pet's names.
            for (auto&& pet : obj.Pets) {
                std::cout << "  " << pet << std::endl;
            }
        }
        // output:
        //     Hedlund, Magnus:
        //       Daisy
        //     Adams, Terry:
        //       Barley
        //       Boots
        //     Weiss, Charlotte :
        //       Whiskers
        //     Doe, John :
        //     Adams, Terry :
        //       Barley
        //       Boots
    }
    {
        struct Person {
            std::string Name;

            bool operator==(const Person& rhs) const {
                return Name == rhs.Name;
            }

            // Test GroupJoinLess.
            bool operator<(const Person& rhs) const {
                return Name < rhs.Name;
            }
        };

        struct Pet {
            std::string Name;
            Person* Owner;
        };

        struct Result {
            std::string OwnerName;
            Enumerable<std::string> Pets;
        };

        Person magnus{"Hedlund, Magnus"};
        Person terry{"Adams, Terry"};
        Person charlotte{"Weiss, Charlotte"};
        Person john{"Doe, John"};

        Pet barley{"Barley", &terry};
        Pet boots{"Boots", &terry};
        Pet whiskers{"Whiskers", &charlotte};
        Pet daisy{"Daisy", &magnus};

        // Create a list where each element is an anonymous
        // type that contains a person's name and
        // a collection of names of the pets they own.
        auto query = Enumerable{magnus, terry, charlotte, john, terry}
            .GroupJoin(
                {barley, boots, whiskers, daisy},
                [] (const Person& person) { return person; },
                [] (const Pet& pet) { return *pet.Owner; },
                [] (const Person& person, const Enumerable<Pet>& petCollection) {
                    return Result{person.Name, petCollection.Select([] (const Pet& pet) { return pet.Name; })};
                });

        for (auto&& obj : query) {
            // Output the owner's name.
            std::cout << obj.OwnerName << ':' << std::endl;
            // Output each of the owner's pet's names.
            for (auto&& pet : obj.Pets) {
                std::cout << "  " << pet << std::endl;
            }
        }
        // output:
        //     Hedlund, Magnus:
        //       Daisy
        //     Adams, Terry:
        //       Barley
        //       Boots
        //     Weiss, Charlotte :
        //       Whiskers
        //     Doe, John :
        //     Adams, Terry :
        //       Barley
        //       Boots
    }
    {
        struct Person {
            std::string Name;

            // Test GroupJoinEqual.
            bool operator==(const Person& rhs) const {
                return Name == rhs.Name;
            }
        };

        struct Pet {
            std::string Name;
            Person* Owner;
        };

        struct Result {
            std::string OwnerName;
            Enumerable<std::string> Pets;
        };

        Person magnus{"Hedlund, Magnus"};
        Person terry{"Adams, Terry"};
        Person charlotte{"Weiss, Charlotte"};
        Person john{"Doe, John"};

        Pet barley{"Barley", &terry};
        Pet boots{"Boots", &terry};
        Pet whiskers{"Whiskers", &charlotte};
        Pet daisy{"Daisy", &magnus};

        // Create a list where each element is an anonymous
        // type that contains a person's name and
        // a collection of names of the pets they own.
        auto query = Enumerable{magnus, terry, charlotte, john, terry}
            .GroupJoin(
                {barley, boots, whiskers, daisy},
                [] (const Person& person) { return person; },
                [] (const Pet& pet) { return *pet.Owner; },
                [] (const Person& person, const Enumerable<Pet>& petCollection) {
                    return Result{person.Name, petCollection.Select([] (const Pet& pet) { return pet.Name; })};
                });

        for (auto&& obj : query) {
            // Output the owner's name.
            std::cout << obj.OwnerName << ':' << std::endl;
            // Output each of the owner's pet's names.
            for (auto&& pet : obj.Pets) {
                std::cout << "  " << pet << std::endl;
            }
        }
        // output:
        //     Hedlund, Magnus:
        //       Daisy
        //     Adams, Terry:
        //       Barley
        //       Boots
        //     Weiss, Charlotte :
        //       Whiskers
        //     Doe, John :
        //     Adams, Terry :
        //       Barley
        //       Boots
    }
}

void TestIntersect() {
    {
        auto both = Enumerable{44, 26, 92, 30, 71, 38}.Intersect({39, 59, 83, 47, 26, 4, 30});

        for (auto&& id : both) {
            std::cout << id << std::endl;
        }
        // output:
        //     26
        //     30
    }
    {
        struct Product {
            std::string Name;
            int Code;

            // Test IntersectLess.
            bool operator<(const Product& rhs) const {
                return (Name < rhs.Name) || ((Name == rhs.Name) && (Code < rhs.Code));
            }
        };

        // Get the products from the first array
        // that have duplicates in the second array.
        auto duplicates = Enumerable<Product>{{"apple", 9}, {"orange", 4}}.Intersect({{"apple", 9}, {"lemon", 12}});

        for (auto&& product : duplicates) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     apple 9
    }
    {
        struct Product {
            std::string Name;
            int Code;

            // Test IntersectEqual.
            bool operator==(const Product& rhs) const {
                return (Name == rhs.Name) && (Code == rhs.Code);
            }
        };

        // Get the products from the first array
        // that have duplicates in the second array.
        auto duplicates = Enumerable<Product>{{"apple", 9}, {"orange", 4}}.Intersect({{"apple", 9}, {"lemon", 12}});

        for (auto&& product : duplicates) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     apple 9
    }
}

void TestJoin() {
    {
        struct Person {
            std::string Name;

            bool operator==(const Person& rhs) const {
                return Name == rhs.Name;
            }

            // Test JoinHash.
            struct hash {
                size_t operator()(const Person& x) const {
                    return std::hash<std::string>{}(x.Name);
                }
            };
        };

        struct Pet {
            std::string Name;
            Person* Owner;
        };

        struct Result {
            std::string OwnerName;
            std::string Pet;
        };

        Person magnus{"Hedlund, Magnus"};
        Person terry{"Adams, Terry"};
        Person charlotte{"Weiss, Charlotte"};
        Person john{"Doe, John"};

        Pet barley{"Barley", &terry};
        Pet boots{"Boots", &terry};
        Pet whiskers{"Whiskers", &charlotte};
        Pet daisy{"Daisy", &magnus};

        // Create a list of Person-Pet pairs where
        // each element is an anonymous type that contains a
        // Pet's name and the name of the Person that owns the Pet.
        auto query = Enumerable{magnus, terry, charlotte, john, terry}
            .JoinHash<Person::hash>(
                {barley, boots, whiskers, daisy},
                [] (const Person& person) { return person; },
                [] (const Pet& pet) { return *pet.Owner; },
                [] (const Person& person, const Pet& pet) { return Result{person.Name, pet.Name}; });

        for (auto&& obj : query) {
            std::cout << obj.OwnerName << " - " << obj.Pet << std::endl;
        }
        // output:
        //     Hedlund, Magnus - Daisy
        //     Adams, Terry - Barley
        //     Adams, Terry - Boots
        //     Weiss, Charlotte - Whiskers
        //     Adams, Terry - Barley
        //     Adams, Terry - Boots
    }
    {
        struct Person {
            std::string Name;

            bool operator==(const Person& rhs) const {
                return Name == rhs.Name;
            }

            // Test JoinLess.
            bool operator<(const Person& rhs) const {
                return Name < rhs.Name;
            }
        };

        struct Pet {
            std::string Name;
            Person* Owner;
        };

        struct Result {
            std::string OwnerName;
            std::string Pet;
        };

        Person magnus{"Hedlund, Magnus"};
        Person terry{"Adams, Terry"};
        Person charlotte{"Weiss, Charlotte"};
        Person john{"Doe, John"};

        Pet barley{"Barley", &terry};
        Pet boots{"Boots", &terry};
        Pet whiskers{"Whiskers", &charlotte};
        Pet daisy{"Daisy", &magnus};

        // Create a list of Person-Pet pairs where
        // each element is an anonymous type that contains a
        // Pet's name and the name of the Person that owns the Pet.
        auto query = Enumerable{magnus, terry, charlotte, john, terry}
            .Join(
                {barley, boots, whiskers, daisy},
                [] (const Person& person) { return person; },
                [] (const Pet& pet) { return *pet.Owner; },
                [] (const Person& person, const Pet& pet) { return Result{person.Name, pet.Name}; });

        for (auto&& obj : query) {
            std::cout << obj.OwnerName << " - " << obj.Pet << std::endl;
        }
        // output:
        //     Hedlund, Magnus - Daisy
        //     Adams, Terry - Barley
        //     Adams, Terry - Boots
        //     Weiss, Charlotte - Whiskers
        //     Adams, Terry - Barley
        //     Adams, Terry - Boots
    }
    {
        struct Person {
            std::string Name;

            // Test JoinEqual.
            bool operator==(const Person& rhs) const {
                return Name == rhs.Name;
            }
        };

        struct Pet {
            std::string Name;
            Person* Owner;
        };

        struct Result {
            std::string OwnerName;
            std::string Pet;
        };

        Person magnus{"Hedlund, Magnus"};
        Person terry{"Adams, Terry"};
        Person charlotte{"Weiss, Charlotte"};
        Person john{"Doe, John"};

        Pet barley{"Barley", &terry};
        Pet boots{"Boots", &terry};
        Pet whiskers{"Whiskers", &charlotte};
        Pet daisy{"Daisy", &magnus};

        // Create a list of Person-Pet pairs where
        // each element is an anonymous type that contains a
        // Pet's name and the name of the Person that owns the Pet.
        auto query = Enumerable{magnus, terry, charlotte, john, terry}
            .Join(
                {barley, boots, whiskers, daisy},
                [] (const Person& person) { return person; },
                [] (const Pet& pet) { return *pet.Owner; },
                [] (const Person& person, const Pet& pet) { return Result{person.Name, pet.Name}; });

        for (auto&& obj : query) {
            std::cout << obj.OwnerName << " - " << obj.Pet << std::endl;
        }
        // output:
        //     Hedlund, Magnus - Daisy
        //     Adams, Terry - Barley
        //     Adams, Terry - Boots
        //     Weiss, Charlotte - Whiskers
        //     Adams, Terry - Barley
        //     Adams, Terry - Boots
    }
}

void TestLast() {
    {
        auto last = Enumerable{9, 34, 65, 92, 87, 435, 3, 54, 83, 23, 87, 67, 12, 19}.Last(5566);

        std::cout << last << std::endl;
        // output:
        //     19
    }
    {
        auto last = Enumerable{9, 34, 65, 92, 87, 435, 3, 54, 83, 23, 87, 67, 12, 19}.Last([] (int x) { return x > 80; }, 5566);

        std::cout << last << std::endl;
        // output:
        //     87
    }
    {
        auto last = Enumerable<std::string>{}.Last("");

        std::cout << (last.empty() ? "<string is empty>" : last) << std::endl;
        // output:
        //     <string is empty>
    }
    {
        double numbers[] = {49.6, 52.3, 51.0, 49.4, 50.2, 48.3};

        auto last50 = Enumerable(numbers).Last([] (double x) { return round(x) == 50.0; }, 0.0);

        std::cout << "The last number that rounds to 50 is " << last50 << std::endl;
        // output:
        //     The last number that rounds to 50 is 50.2

        auto last40 = Enumerable(numbers).Last([] (double x) { return round(x) == 40.0; }, 0.0);

        std::cout << "The last number that rounds to 40 is " << ((last40 == 0.0) ? "<DOES NOT EXIST>" : std::to_string(last40)) << std::endl;
        // output:
        //     The last number that rounds to 40 is <DOES NOT EXIST>
    }
}

void TestOrderBy() {
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        auto query = Enumerable<Pet>{{"Barley", 8}, {"Boots", 4}, {"Whiskers", 1}}.OrderBy([] (const Pet& pet) { return pet.Age; });

        for (auto&& pet : query) {
            std::cout << pet.Name << " - " << pet.Age << std::endl;
        }
        // output:
        //     Whiskers - 1
        //     Boots - 4
        //     Barley - 8
    }
    {
        struct Pet {
            std::string Name;
            int Age;
        };

        auto query = Enumerable<Pet>{{"Barley", 8}, {"Boots", 4}, {"Whiskers", 1}}.OrderByDescending([] (const Pet& pet) { return pet.Age; });

        for (auto&& pet : query) {
            std::cout << pet.Name << " - " << pet.Age << std::endl;
        }
        // output:
        //     Barley - 8
        //     Boots - 4
        //     Whiskers - 1
    }
}

void TestPrepend() {
    {
        // Creating a list of numbers
        Enumerable numbers{1, 2, 3, 4};

        // Trying to prepend any value of the same type
        numbers.Prepend(0);

        // It doesn't work because the original list has not been changed
        std::copy(std::begin(numbers), std::end(numbers), std::ostream_iterator<decltype(numbers)::value_type>{std::cout, ","});
        std::cout << std::endl;
        // output:
        //     1,2,3,4,

        // It works now because we are using a changed copy of the original list
        // TODO Console.WriteLine(string.Join(", ", numbers.Prepend(0)));

        // If you prefer, you can create a new list explicitly
        auto newNumbers = numbers.Prepend(0);

        // And then write to the console output
        std::copy(std::begin(newNumbers), std::end(newNumbers), std::ostream_iterator<decltype(newNumbers)::value_type>{std::cout, ","});
        std::cout << std::endl;
        // output:
        //     0,1,2,3,4,
    }
}

void TestRange() {
    {
        auto squares = Enumerable<int>::Range(1, 10).Select([] (int x) { return x * x; });

        for (auto&& num : squares) {
            std::cout << num << std::endl;
        }
        // output:
        //     1
        //     4
        //     9
        //     16
        //     25
        //     36
        //     49
        //     64
        //     81
        //     100
    }
}

void TestRepeat() {
    {
        auto strings = Enumerable<std::string>::Repeat("I like programming.", 5);

        for (auto&& str : strings) {
            std::cout << str << std::endl;
        }
        // output:
        //     I like programming.
        //     I like programming.
        //     I like programming.
        //     I like programming.
        //     I like programming.
    }
}

void TestReverse() {
    {
        auto reversed = Enumerable{'a', 'p', 'p', 'l', 'e'}.Reverse();

        std::copy(std::begin(reversed), std::end(reversed), std::ostream_iterator<decltype(reversed)::value_type>{std::cout, " "});
        std::cout << std::endl;
        // output:
        //     e l p p a
    }
}

void TestSelect() {
    {
        struct Result {
            int index;
            std::string str;
        };

        auto query = Enumerable<std::string>{"apple", "banana", "mango", "orange", "passionfruit", "grape"}
            .SelectWithIndex([] (const std::string& fruit, int index) { return Result{index, fruit.substr(0, index)}; });

        for (auto&& obj : query) {
            std::cout << "{index=" << obj.index << ", str=" << obj.str << '}' << std::endl;
        }
        // output:
        //     {index=0, str=}
        //     {index=1, str=b}
        //     {index=2, str=ma}
        //     {index=3, str=ora}
        //     {index=4, str=pass}
        //     {index=5, str=grape}
    }
}

void TestSelectMany() {
    {
        struct PetOwner {
            std::string Name;
            std::vector<std::string> Pets;
        };

        struct Result1 {
            PetOwner petOwner;
            std::string petName;
        };

        struct Result2 {
            std::string Owner;
            std::string Pet;
        };

        PetOwner petOwners[] = {
            {"Higa", {"Scruffy", "Sam"}},
            {"Ashkenazi", {"Walker", "Sugar"}},
            {"Price", {"Scratches", "Diesel"}},
            {"Hines", {"Dusty"}},
        };

        // Project the pet owner's name and the pet's name.
        auto query = Enumerable<PetOwner>{petOwners}
            .SelectMany(
                [] (const PetOwner& petOwner) { return petOwner.Pets; },
                [] (const PetOwner& petOwner, const std::string& petName) { return Result1{petOwner, petName}; })
            .Where([] (const Result1& ownerAndPet) { return ownerAndPet.petName[0] == 'S'; })
            .Select([] (const Result1& ownerAndPet) { return Result2{ownerAndPet.petOwner.Name, ownerAndPet.petName}; });

        // Print the results.
        for (auto&& obj : query) {
            std::cout << "{Owner=" << obj.Owner << ", Pet=" << obj.Pet << '}' << std::endl;
        }
        // output:
        //     {Owner=Higa, Pet=Scruffy}
        //     {Owner=Higa, Pet=Sam}
        //     {Owner=Ashkenazi, Pet=Sugar}
        //     {Owner=Price, Pet=Scratches}
    }
    {
        struct PetOwner {
            std::string Name;
            std::vector<std::string> Pets;
        };

        PetOwner petOwners[] = {
            {"Higa, Sidney", {"Scruffy", "Sam"}},
            {"Ashkenazi, Ronen", {"Walker", "Sugar"}},
            {"Price, Vernette", {"Scratches", "Diesel"}},
        };

        // Query using SelectMany().
        auto query1 = Enumerable<PetOwner>{petOwners}.SelectMany([] (const PetOwner& petOwner) { return petOwner.Pets; });

        // Only one foreach loop is required to iterate
        // through the results since it is a
        // one-dimensional collection.
        std::cout << "Using SelectMany():" << std::endl;
        for (auto&& pet : query1) {
            std::cout << pet << std::endl;
        }
        // output:
        //     Using SelectMany():
        //     Scruffy
        //     Sam
        //     Walker
        //     Sugar
        //     Scratches
        //     Diesel

        // This code shows how to use Select()
        // instead of SelectMany().
        auto query2 = Enumerable<PetOwner>{petOwners}.Select([] (const PetOwner& petOwner) { return petOwner.Pets; });

        // Notice that two foreach loops are required to
        // iterate through the results
        // because the query returns a collection of arrays.
        std::cout << "Using Select():" << std::endl;
        for (auto&& petList : query2) {
            for (auto&& pet : petList) {
                std::cout << pet << std::endl;
            }
            std::cout << std::endl;
        }
        // output:
        //     Using Select():
        //     Scruffy
        //     Sam
        //
        //     Walker
        //     Sugar
        //
        //     Scratches
        //     Diesel
    }
    {
        struct PetOwner {
            std::string Name;
            std::vector<std::string> Pets;
        };

        PetOwner petOwners[] = {
            {"Higa, Sidney", {"Scruffy", "Sam"}},
            {"Ashkenazi, Ronen", {"Walker", "Sugar"}},
            {"Price, Vernette", {"Scratches", "Diesel"}},
            {"Hines, Patrick", {"Dusty"}},
        };

        // Project the items in the array by appending the index
        // of each PetOwner to each pet's name in that petOwner's
        // array of pets.
        auto query = Enumerable<PetOwner>{petOwners}
            .SelectManyWithIndex([] (const PetOwner& petOwner, int index) {
                return Enumerable<std::string>{petOwner.Pets}.Select([&] (const std::string& pet) { return std::to_string(index) + pet; });
            });

        for (auto&& pet : query) {
            std::cout << pet << std::endl;
        }
        // output:
        //     0Scruffy
        //     0Sam
        //     1Walker
        //     1Sugar
        //     2Scratches
        //     2Diesel
        //     3Dusty
    }
}

void TestSequenceEqual() {
    {
        struct Pet {
            std::string Name;
            int Age;

            bool operator==(const Pet& rhs) const {
                return (Name == rhs.Name) && (Age == rhs.Age);
            }
        };

        //Pet pets[] = {
        //    {"Turbo", 2},
        //    {"Peanut", 8},
        //};

        Pet pet1{"Turbo", 2};
        Pet pet2{"Peanut", 8};

        // Create two lists of pets.
        Pet pets1[] = {pet1, pet2};
        Pet pets2[] = {pet1, pet2};

        auto equal = Enumerable<Pet>{pets1}.SequenceEqual(pets2);

        std::cout << "This lists " << (equal ? "are" : "are not") << " equal" << std::endl;
        // output:
        //     This lists are equal
    }
    {
        struct Product {
            std::string Name;
            int Code;

            struct equal_to {
                bool operator()(const Product& lhs, const Product& rhs) const {
                    return (&lhs == &rhs) || ((lhs.Name == rhs.Name) && (lhs.Code == rhs.Code));
                }
            };
        };

        Product storeA[] = {
            {"apple", 9},
            {"orange", 4},
        };

        Product storeB[] = {
            {"apple", 9},
            {"orange", 4},
        };

        auto equalAB = Enumerable<Product>{storeA}.SequenceEqual(storeB, Product::equal_to{});

        std::cout << "Equal? " << (equalAB ? "True" : "False") << std::endl;
        // output:
        //     Equal? True
    }
}

void TestSingle() {
    {
        auto single1 = Enumerable{1}.Single(5566);

        std::cout << single1 << std::endl;
        // output:
        //     1
    }
    {
        auto single2 = Enumerable{1, 2}.Single(5566);

        std::cout << single2 << std::endl;
        // output:
        //     5566
    }
    {
        auto single3 = Enumerable{1, 2}.Single(5566, [] (int x) { return x < 2; });

        std::cout << single3 << std::endl;
        // output:
        //     1
    }
    {
        auto single4 = Enumerable{1, 2}.Single(5566, [] (int x) { return x < 3; });

        std::cout << single4 << std::endl;
        // output:
        //     5566
    }
}

void TestSkip() {
    {
        auto lowerGrades = Enumerable{59, 82, 70, 56, 92, 98, 85}
            .OrderByDescending()
            .Skip(3);

        std::cout << "All grades except the top three are:" << std::endl;
        for (auto&& grade : lowerGrades) {
            std::cout << grade << std::endl;
        }
        // output:
        //     All grades except the top three are:
        //     82
        //     70
        //     59
        //     56
    }
    {
        auto count = Enumerable{1, 2, 3}.Skip(5).Count();

        std::cout << count << std::endl;
        // output:
        //     0
    }
}

void TestSkipLast() {
    {
        auto lowerGrades = Enumerable{59, 82, 70, 56, 92, 98, 85}
            .OrderBy()
            .SkipLast(3);

        std::cout << "All grades except the top three are:" << std::endl;
        for (auto&& grade : lowerGrades) {
            std::cout << grade << std::endl;
        }
        // output:
        //     All grades except the top three are:
        //     56
        //     59
        //     70
        //     82
    }
    {
        auto count = Enumerable{1, 2, 3}.SkipLast(5).Count();

        std::cout << count << std::endl;
        // output:
        //     0
    }
}

void TestSkipWhile() {
    {
        auto lowerGrades = Enumerable{59, 82, 70, 56, 92, 98, 85}
            .OrderByDescending()
            .SkipWhile([] (int grade) { return grade >= 80; });

        std::cout << "All grades below 80:" << std::endl;
        for (auto&& grade : lowerGrades) {
            std::cout << grade << std::endl;
        }
        // output:
        //     All grades below 80:
        //     70
        //     59
        //     56
    }
    {
        auto query = Enumerable{5000, 2500, 9000, 8000, 6500, 4000, 1500, 5500}
            .SkipWhileWithIndex([] (int amount, int index) { return amount > index * 1000; });

        for (auto&& amount : query) {
            std::cout << amount << std::endl;
        }
        // output:
        //     4000
        //     1500
        //     5500
    }
}

void TestTake() {
    {
        auto topThreeGrades = Enumerable{59, 82, 70, 56, 92, 98, 85}
            .OrderByDescending()
            .Take(3);

        std::cout << "The top three grades are:" << std::endl;
        for (auto&& grade : topThreeGrades) {
            std::cout << grade << std::endl;
        }
        // output:
        //     The top three grades are:
        //     98
        //     92
        //     85
    }
    {
        auto count = Enumerable{1, 2, 3}.Take(5).Count();

        std::cout << count << std::endl;
        // output:
        //     3
    }
}

void TestTakeLast() {
    {
        auto topThreeGrades = Enumerable{59, 82, 70, 56, 92, 98, 85}
            .OrderBy()
            .TakeLast(3);

        std::cout << "The top three grades are:" << std::endl;
        for (auto&& grade : topThreeGrades) {
            std::cout << grade << std::endl;
        }
        // output:
        //     The top three grades are:
        //     85
        //     92
        //     98
    }
    {
        auto count = Enumerable{1, 2, 3}.TakeLast(5).Count();

        std::cout << count << std::endl;
        // output:
        //     3
    }
}

void TestTakeWhile() {
    {
        auto query = Enumerable<std::string>{"apple", "banana", "mango", "orange", "passionfruit", "grape"}
            .TakeWhile([] (const std::string& fruit) { return fruit != "orange"; });

        for (auto&& fruit : query) {
            std::cout << fruit << std::endl;
        }
        // output:
        //     apple
        //     banana
        //     mango
    }
    {
        auto query = Enumerable<std::string>{"apple", "passionfruit", "banana", "mango", "orange", "blueberry", "grape", "strawberry"}
            .TakeWhileWithIndex([] (const std::string& fruit, int index) { return fruit.size() >= index; });

        for (auto&& fruit : query) {
            std::cout << fruit << std::endl;
        }
        // output:
        //     apple
        //     passionfruit
        //     banana
        //     mango
        //     orange
        //     blueberry
    }
}

void TestUnion() {
    {
        auto u = Enumerable{5, 3, 9, 7, 5, 9, 3, 7}.Union({8, 3, 6, 4, 4, 9, 1, 0});

        std::copy(std::begin(u), std::end(u), std::ostream_iterator<decltype(u)::value_type>(std::cout, " "));
        std::cout << std::endl;
        // output:
        //     5 3 9 7 8 6 4 1 0
    }
    {
        struct ProductA {
            std::string Name;
            int Code;

            // Test UnionEqual.
            bool operator==(const ProductA& rhs) const {
                return (Name == rhs.Name) && (Code == rhs.Code);
            }
        };

        // Get the products from the both arrays
        // excluding duplicates.
        auto u = Enumerable<ProductA>{{"apple", 9}, {"orange", 4}, {"orange", 4}}.Union({{"apple", 9}, {"lemon", 12}, {"apple", 9}});

        for (auto&& product : u) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     apple 9
        //     orange 4
        //     lemon 12
    }
    {
        struct ProductA {
            std::string Name;
            int Code;

            bool operator==(const ProductA& rhs) const {
                return (Name == rhs.Name) && (Code == rhs.Code);
            }

            // Test UnionLess.
            bool operator<(const ProductA& rhs) const {
                return (Name < rhs.Name) || ((Name == rhs.Name) && (Code < rhs.Code));
            }
        };

        // Get the products from the both arrays
        // excluding duplicates.
        auto u = Enumerable<ProductA>{{"apple", 9}, {"orange", 4}, {"orange", 4}}.Union({{"apple", 9}, {"lemon", 12}, {"apple", 9}});

        for (auto&& product : u) {
            std::cout << product.Name << ' ' << product.Code << std::endl;
        }
        // output:
        //     apple 9
        //     orange 4
        //     lemon 12
    }
}

void TestWhere() {
    {
        auto query = Enumerable<std::string>{"apple", "passionfruit", "banana", "mango", "orange", "blueberry", "grape", "strawberry"}
            .Where([] (const std::string& fruit) { return fruit.size() < 6; });

        for (auto&& fruit : query) {
            std::cout << fruit << std::endl;
        }
        // output:
        //     apple
        //     mango
        //     grape
    }
    {
        auto query = Enumerable{0, 30, 20, 15, 90, 85, 40, 75}.WhereWithIndex([] (int number, int index) { return number <= index * 10; });

        for (auto&& number : query) {
            std::cout << number << std::endl;
        }
        // output:
        //     0
        //     20
        //     15
        //     40
    }
}

void TestZip() {
    {
        auto numbersAndWords = Enumerable{1, 2, 3, 4}
            .Zip(Enumerable<std::string>{"one", "two", "three"}, [] (int first, const std::string& second) { return std::to_string(first) + ' ' + second; });

        for (auto&& item : numbersAndWords) {
            std::cout << item << std::endl;
        }
        // output:
        //     1 one
        //     2 two
        //     3 three
    }
    {
        auto numbersAndWords = Enumerable{1, 2, 3, 4}
            .Zip(Enumerable<std::string>{"one", "two", "three"});

        for (auto&& item : numbersAndWords) {
            std::cout << item.first << ' ' << item.second << std::endl;
        }
        // output:
        //     1 one
        //     2 two
        //     3 three
    }
}
} // namespace rvalue

using namespace rvalue;

void TestRvalue() {
    TestAggregate();
    TestAll();
    TestAny();
    TestAppend();
    TestConcat();
    TestContains();
    TestCount();
    TestDefaultIfEmpty();
    TestDistinct();
    TestElementAt();
    TestEmpty();
    TestExcept();
    TestFirst();
    TestGroupBy();
    TestGroupJoin();
    TestIntersect();
    TestJoin();
    TestLast();
    TestOrderBy();
    TestPrepend();
    TestRange();
    TestRepeat();
    TestReverse();
    TestSelect();
    TestSelectMany();
    TestSequenceEqual();
    TestSingle();
    TestSkip();
    TestSkipLast();
    TestSkipWhile();
    TestTake();
    TestTakeLast();
    TestTakeWhile();
    TestUnion();
    TestWhere();
    TestZip();
}