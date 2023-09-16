#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace std; 

int main(int argc, char *argv[]){

    int fd = open(argv[1], O_CREAT | O_WRONLY, S_IRWXU);
    if (fd == -1) {
        cerr << "Ошибка при попытке открыть файл" << '\n';
        return 1;
    }

    int err = ftruncate(fd, 0);
    if (err == -1) {
        cerr << "Ошибка при попытке очистить файл" << '\n';
        return 1;
    }

    size_t n;
    double x, res;
    while (cin >> n) {
        for (size_t i = 0; i < n; ++i) {
            cin >> x;
            if (i == 0) {
                res = x;
            } else if (x == 0){
                cerr << "ERROR: Попытка деления на 0" << '\n';
                exit(EXIT_FAILURE);
            } else {
                res /= x;
            }
        }
        dprintf(fd, "%lf\n", res);
    }
    return 0;
}