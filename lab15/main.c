#include <stdio.h>
#include <string.h>

//https://ru.wikipedia.org/wiki/%D0%9F%D0%BE%D1%80%D1%8F%D0%B4%D0%BE%D0%BA_%D0%B1%D0%B0%D0%B9%D1%82%D0%BE%D0%B2

void create(char *name, int count, char *fileNames[])
{
    //создаем двоичный файл arcfile
    FILE *arcfile = fopen(name, "wb");
    unsigned long long int size[128];
    for (int i = 5; i < count; ++i)
    {
        //открываем для чтения текущий файл, который передаем в командной строке
        FILE *file = fopen(fileNames[i], "rb");
        if (file == NULL)
            continue;
        fseek(file, 0, SEEK_END);
        //записываем размер текущего файла
        size[i - 5] = ftell(file);
        fseek(file, 0, SEEK_SET);
        fclose(file);
    }
    //в первых строках файла arcfile записываем информацию : название архивируемого файла - его размер
    for (int i = 0; i < count - 5; ++i)
        fprintf(arcfile, "< %s : %llu >", fileNames[i + 5], size[i]);
    fprintf(arcfile, "\n");
    int temp;
    //переносим информацию о каждом файле в arcfile
    for (int i = 5; i < count; ++i)
    {
        FILE *file = fopen(fileNames[i], "rb");
        while ((temp = getc(file)) != EOF)
        {
            putc(temp, arcfile);
        }
        fclose(file);
    }
    printf("\nCreating done\n");
}

void extract(char *name)
{
    //открываем двоичный файл дл просмотра и записи
    FILE *current = fopen(name, "rb+wb");
    unsigned long long int now = 0;
    unsigned long long int start = 0;
    int c;
    while ((c = getc(current)) != EOF)
    {
        ++start;
        if (c == '\n')
            break;
    }
    fseek(current, 0, SEEK_SET);
    char fileNames[128] = {0};
    unsigned long long int size;
    while (fscanf(current, "< %s : %llu >", fileNames, &size) != 0)
    {
        FILE *file = fopen(fileNames, "wb");
        if (file == NULL)
            break;
        now = ftell(current);
        fseek(current, start, SEEK_SET);
        start += size;
        while (size-- > 0)
            putc(getc(current), file);
        fseek(current, now, SEEK_SET);
        fclose(file);
    }
    printf("\nFiles are in the folder\n");
}

void list(char *name)
{
    FILE *file = fopen(name, "rb");
    char fileNames[10] = {0};
    unsigned long long size;
    printf("There are files : \n");
    while (fscanf(file, "< %s : %llu >", fileNames, &size) != 0)
    {
        printf("%s\n", fileNames);
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    char *name;
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp("--file", argv[i]))
            name = argv[i + 1];
        if (!strcmp("--create", argv[i]))
        {
            create(name, argc, argv);
            return 0;
        }
        if (!strcmp("--extract", argv[i]))
            extract(name);
        if (!strcmp("--list", argv[i]))
            list(name);
    }
    return 0;
}