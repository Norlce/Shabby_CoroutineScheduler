#include<iostream>
#include<set>

int main(){
    std::set<int> set;
    set.insert(100);
    set.insert(101);
    set.insert(102);
    set.insert(103);
    // auto it = set.begin();
    // auto temp = it;
    // temp++;
    // auto it2 = temp;
    // auto it3 = temp;
    // temp++;
    // set.erase(temp);
    // temp = it2;
    // temp++;
    // set.erase(temp);
    // set.erase(it);
    // std::cout<<' '<<*it2<<' '<<*it3<<std::endl;
    for(auto itt: set){
        set.erase(101);
        std::cout<<itt<<' ';
    }
    std::cout<<std::endl;
    return 0;
}