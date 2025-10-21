#include <vector>
#include <iostream>
#include <list>

static void PrintVec(const std::vector<int> &vec)
{
    std::cout << *vec.begin() << "\n";
    std::cout << *(vec.end()-1) << "\n";

    for (auto x: vec)
    {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

static void RemoveOdds(std::list<int>& lst)
{
    auto it = lst.begin();
    while (it != lst.end())
    {
        if (*it % 2 == 1)
        {
            lst.erase(it++); // returns it to erase and increases it already
        }
        else ++it;
    }
}

int main()
{
    std::vector<int> v{1,2,3,4};
    std::vector<int> v2 = {1,2,3,4};
    std::vector<int> v3({1,2,3,4});
    std::vector<int> v4 = std::vector<int>({1,2,3,4});

    PrintVec(v4);
    
    std::list<int> l{1,2,3,4};

    RemoveOdds(l);

    for (int x : l)
        std::cout << x << " ";
}