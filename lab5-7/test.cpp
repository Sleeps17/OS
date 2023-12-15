#include<iostream>
#include<map>

int cnt = 0;

void func() {
    cnt++;
}


int main() {
    for(int i = 0; i < 100; i++) {
        pthred_join(func);
    }

    std::cout << cnt << std::endl;
}