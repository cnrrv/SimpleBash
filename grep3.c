#include <stdio.h>  //argc - количество аргументов командной строки argv - список аргументов командной строки команда - нулевой аргумент
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <getopt.h>
#include "flags.h"

void flags(int argc, char** argv, char** string_list, char** file_list);
void grep_func(char** string_list, char* file_list, struct flags f);

int main(int argc, char** argv) {
    const int m = 500, n = 1000;
    int count = 0;
    //выделение динамической памяти для search_string
    char** search_string = (char **)malloc(m * sizeof(char*));
    for (int i = 0; i < m; i++) {
        search_string[i] = (char *)malloc(n * sizeof(char));
    }
    //выделение динамической памяти для file_list
    char** search_file = (char **)malloc(m * sizeof(char*));
    for (int i = 0; i < m; i++) {
        search_file[i] = (char *)malloc(n * sizeof(char));
    }
    //проверка флагов
    flags(argc, argv, search_string, search_file);
    //освобождение памяти для search_file
    for (int i = 0; i < m; i++) {
        free(search_file[i]);
    }
    //освобождение памяти для search_string
    for (int i = 0; i < m; i++) {
        free(search_string[i]);
    }
    free(search_string);
    free(search_file);
    return 0;
}

void flags(int argc, char** argv, char** string_list, char** file_list) {
    int res = 0, counter = 1, search_counter = 0;
    struct flags f = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // ? можно ли так
    while (counter < argc) { //пока не прошли все аргументы строки
        res = getopt(argc, argv, "e:ivcln"); //указываем, какие флаги мы хотим найти
        if (res == 'e') {
            f.opt_e = 1;
            int len = strlen(optarg); //сканируем длину следующего элемента(слова) после флага -е
            for (int i = 0; i < len; i++) {
                //посимвольно добавляем слово в массив (список в списке)
                string_list[search_counter][i] = optarg[i];
            }
            search_counter++; //инкрементируем, т.к. нужна следующая строка дл ввода нового слова в список
            f.search_num++; //инктементируем, т.к. у нас больше одного слова для поиска
        }
        else if (res == 'i') f.opt_i = 1;
        else if (res == 'v') f.opt_v = 1;
        else if (res == 'c') f.opt_c = 1;
        else if (res == 'l') f.opt_l = 1;
        else if (res == 'n') f.opt_n = 1;
        //else if (res == '?') f.error = 1; //если флаг не найден
        else if (res == -1 && optind < argc) { //если ни одного флага не найдено
            int len = strlen(argv[optind]); //находим количество элементов строки
                for (int i = 0; i < len; i++) { //и пока они не закончились
                    file_list[f.file_num][i] = argv[optind][i]; //добавляем в в список файлов эти элементы
                }
                file_list[f.file_num][len] = '\0'; //обозначаем конец файла
                f.file_num++; //инкрементируем, т.к. больше одного файла на поиск
                optind++; //переходим к следующему элементу
            }
            counter = optind; //изменение counter
        }
        if (f.opt_e != 1) {
            string_list[0] = file_list[0]; //мы присваиваем первый элемент списка файлов первому элементу списка поисковых строк
            //т.к. если нет флага e мы добавляем слово в список файлов первым элементом
            f.flag_start = 1; //если флага e не было, смотрим файлы с 1 элемента списка
            f.search_num = 1; //т.к. нужно будет найти одно слово
        }
    for (int i = f.flag_start; i < f.file_num; i++) {
        grep_func(string_list, file_list[i], f);
    }
}

void grep_func(char** string_list, char* file_list, struct flags f) {
    FILE* file;
    if ((file = fopen(file_list, "r")) != NULL) { //проверяем что файл существует
        int compare = 0, regflag = 0, str_num = 1, end_line = 0, count = 0;
        if (f.opt_v) compare = REG_NOMATCH; //реализация флага v
        if (f.opt_i) regflag = REG_ICASE; //реализация флага i
        char* TMP_LINE = NULL; //нужно указать пустую строку
        size_t len = 1000; //макс длина совпадающей строки
        ssize_t line = getline(&TMP_LINE, &len, file); //берем строчку из файла
        while (line != EOF) { //пока не достигли конца линии
            end_line = 0; //переменная для переноса строки
            if (TMP_LINE) { //проверяем что линия не 0
                int print_flag = 1; //чтобы не печтать несколько раз одну строчку (если несколько совпадений)
                for (int i = 0; i < f.file_num - f.flag_start; i++) { //если была поисковая строка в файле, вычитаем ее
                    regex_t regex; //создаем переменную формата regex
                    regcomp(&regex, string_list[i], regflag); //сравниваем строку файла со строкой для поиска
                    int success = regexec(&regex, TMP_LINE, 0, NULL, 0); //присваиваем результат переменной
                    if (((success == 0) && (f.opt_v == 0)) || ((success == compare) && print_flag)) { //если переменная равна 0 (т.е. сравнение нашлось) и флаг v равен 0, или переменная равна переменной compare и вхождение строки было найдено
                        if (f.opt_c == 0 && f.opt_l == 0) { //если флаги с и l не подключены
                            if (f.file_num - f.flag_start > 1) printf("%s:", file_list); //
                            if (f.opt_n) printf("%d:", str_num); //если флаг n подключен, выводим номер строки
                            printf("%s", TMP_LINE); //ЗДЕСЬ ВЫВОД СТРОКИ
                            print_flag = 0; //флаг совпадений равен 0
                            if (TMP_LINE[strlen(TMP_LINE) - 1] != '\n' && TMP_LINE[strlen(TMP_LINE) - 1] != '\0' && strlen(TMP_LINE) > 0) end_line = 1; //если последний символ не перенос и не конец строки и длина строки больше 0 присваиваем end_line 1 (понадобится для переноса последней строки)
                        }
                        count++; //увеличиваем счетчик
                    }
                    regfree(&regex); //освобождаем regex
                }
                str_num++; //увеличиваем номер строки
            }
            line = getline(&TMP_LINE, &len, file); //сканируем следующую строку
        }
        if (f.opt_c) { //если подключен флаг с
            if (f.opt_l && count > 0) { //если подключен флаг l и счетчик больше 0
                if (f.file_num - f.flag_start > 1) printf("%s:", file_list); //печатаем название файла
                printf("1\n");
            } else {
            if (f.file_num - f.flag_start > 1) printf("%s:", file_list);
            printf("%d\n", count); //печатаем номер строки
            }
        }
        if (f.opt_l && count) printf("%s\n", file_list); //если флаг l и 
        if (end_line == 1 && count) printf("\n"); //перенос строки в конце
        free(TMP_LINE); //очищаем
        fclose(file); //закрываем файл
    }
}