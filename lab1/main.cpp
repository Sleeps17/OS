#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
using namespace std;

int main(){
    string fileName;
    cout << "Введите название файла: ";
    cin >> fileName; 

    int fd1[2], fd2[2];
    int err = pipe(fd1);
    if (err == -1) {
        cerr << "Ошибка при создании пайпа" << '\n';
        return 1;
    }

    err = pipe(fd2);
    if (err == -1) {
        cerr << "Ошибка при создани пайпа" << '\n';
        return 1;
    }

    int write1 = fd1[1], read1 = fd1[0];
    int write2 = fd2[1], read2 = fd2[0];

    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Ошибка при создании дочернего процесса" << '\n';
        return 1;
    }

    if (pid == 0) {
        close(write1);
        close(read2);

        err = dup2(read1, fileno(stdin));
        if (err == -1) { 
            cerr << "Ошибка при перенаправлении ввода" << '\n';
            return 11;
        }

        err = dup2(write2, fileno(stdout));
        if (err == -1) {
            cerr << "Ошибка при перенаправлении вывода" << '\n';
            return 1;
        }

        err = execl("child_process", "child_process", fileName.c_str(), NULL);
        if (err == -1) {
            cerr << "Ошибка при запуске программы из дочернего процесса" << '\n';
            return 1;
        }
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {

        close(read1);
        close(write2);

        double x;
        char c;
        vector<double> numbers;
        while (scanf("%lf%c", &x, &c) > 0) {
            numbers.push_back(x);
            if (c == '\n') {
                size_t n = numbers.size();
                dprintf(write1, "%ld ", n);
                for (size_t i = 0; i < n; ++i) {
                    dprintf(write1, "%lf ", numbers[i]);
                }
                numbers.clear();
            }
        }

        //Разобраться, что нужно получать по второму пайпу

        close(write1);
        close(read2);

        wait(nullptr);
    }
    return 0;
}