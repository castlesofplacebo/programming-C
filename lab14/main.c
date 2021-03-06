#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

//https://github.com/lanit-tercom-school/grouplock/wiki/C%D1%82%D1%80%D1%83%D0%BA%D1%82%D1%83%D1%80%D0%B0-%D1%85%D1%80%D0%B0%D0%BD%D0%B5%D0%BD%D0%B8%D1%8F-bmp-%D1%84%D0%B0%D0%B9%D0%BB%D0%BE%D0%B2
//https://jenyay.net/Programming/Bmp
struct BMP
{
    int wid;
    int hei;
    int size;
};

//ПРАВИЛА ИГРЫ
//Каждая клетка на этой поверхности может находиться в двух состояниях: быть «живой» (заполненной) или быть «мёртвой» (пустой). Клетка имеет восемь соседей, окружающих её.
//Распределение живых клеток в начале игры называется первым поколением. Каждое следующее поколение рассчитывается на основе предыдущего по таким правилам:
//в пустой (мёртвой) клетке, рядом с которой ровно три живые клетки, зарождается жизнь;
//если у живой клетки есть две или три живые соседки, то эта клетка продолжает жить; в противном случае, если соседей меньше двух или больше трёх, клетка умирает («от одиночества» или «от перенаселённости»)
//Игра прекращается, если на поле не останется ни одной «живой» клетки
//конфигурация на очередном шаге в точности (без сдвигов и поворотов) повторит себя же на одном из более ранних шагов (складывается периодическая конфигурация)
//при очередном шаге ни одна из клеток не меняет своего состояния (складывается стабильная конфигурация; предыдущее правило, вырожденное до одного шага назад)

void gameRules(int **image, int hei, int wid)
{
    int array[hei][wid];
    int x, y;
    for (y = 0; y < hei; y++)
        for (x = 0; x < wid; x++)
            array[y][x] = image[y][x];
    for (y = 1; y < hei - 1; y++)
    {
        for (x = 1; x < wid - 1; x++)
        {
            int N = image[y + 1][x - 1] + image[y + 1][x] + image[y + 1][x + 1] + image[y][x - 1] + image[y][x + 1] + image[y - 1][x - 1] + image[y - 1][x] + image[y - 1][x + 1];
            if (array[y][x] == 1)
            {
                if (N == 2)
                    array[y][x] = image[y][x];
                if (N == 3)
                    array[y][x] = image[y][x];
                if (N > 3)
                    array[y][x] = 0;
                if (N < 2)
                    array[y][x] = 0;
            }
            else if (N == 3)
                array[y][x] = 1;
        }
    }
    int i = 0;
    for (y = 0; y < hei; y++)
        for (x = 0; x < wid; x++)
        {
            if (image[y][x] == array[y][x])
                ++i;
            image[y][x] = array[y][x];
        }
    if (i == hei * wid)
        exit(0);
}

int main(int argc, char *argv[])
{
    struct BMP current;
    int j, k, l, m;
    char *folderName;
    FILE *file;
    //обработка значений, полученных с консоли
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp("--input", argv[i]))
        {
            file = fopen(argv[1 + 1], "rb");
        }
        if (!strcmp("--output", argv[i]))
        {
            folderName = argv[i + 1];
            mkdir(folderName); //создание директории, хранящей в себе последующие поколения игры
        }
    }

    //обработка bmp-файла с первым поколением игры

    //Заголовок
    //Смещение	Длина поля	Описание поля
    //0	2	Код 4D42h — Буквы 'BM'
    //2	5	Размер файла в байтах
    //6	2	0 (Резервное поле)
    //8	2	0 (Резервное поле)
    //10	4	Начинается изображение
    //Заголовок BITMAP (Информация об изображении)
    //Смещение	Длина поля	Описание поля
    //14	4	Размер заголовка BITMAP в байтах (40)
    //18	4	Ширина изображения в пикселях
    //22	4	Высота изображения в пикселях
    //26	2	Число плоскостей (не используется) = 1
    //28	2	Бит/пиксель*
    //30	4	Тип сжатия (обычно не используется)
    //34	4	Размер сжатого изображения в байтах
    //38	4	Горизонтальное расширение, пиксель/м
    //42	4	Вертикальное расширение, пиксель/м
    //46	4	Кол-во используемых цветов (очень редко)
    //50	4	Кол-во "важных"цветов (не используется)
    //Палитра (Карта цветов для N цветов)**
    //Смещение	Длина поля	Описание поля
    //54	4*N	Палитра

    unsigned char header[54];
    //считываем размер файла в байтах, его ширину и высота
    fread(header, 1, 54, file);
    current.size = header[2] + header[3] * 256 + header[4] * 256 * 256 + header[5] * 256 * 256 * 256;
    current.wid = header[18] + header[19] * 256 + header[20] * 256 * 256 + header[21] * 256 * 256 * 256;
    current.hei = header[22] + header[23] * 256 + header[24] * 256 * 256 + header[25] * 256 * 256 * 256;

    //запись в array of char битов информации о содержании pixel-array
    unsigned char imagebyte[current.size - 54];
    fread(imagebyte, 1, current.size, file);

    //выделение памяти под pixel-array
    int **img = (int **)malloc(current.hei * sizeof(int *));
    for (int i = 0; i < current.hei; i++)
        img[i] = (int *)malloc(current.wid * sizeof(int));

    //обработка и получение массива (монохромного)
    k = -(current.wid % 4);
    for (int i = current.hei - 1; i > -1; --i)
    {
        //biBitCount	Формула на С
        //8	(3 * Width) % 4
        //16	(2 * Width) % 4
        //24	Width % 4
        // пиксели тут описываются так, как написано в таблице выше в зависимости от формата. И могут сами содержать значение компонентов цвета (для беспалитровых), а могут быть индексами массива-палитры. Сама картинка записывается построчно.
        // Во-вторых, картинка идет как бы перевернутая вверх ногами. То есть сначала записана нижняя строка, потом предпоследняя и так далее до самого верха.
        // если размер строки растра не кратен 4, то она дополняется от 1 до 3 пустыми (нулевыми) байтами, чтобы длина строки оказалась кратна параграфу
        k += (current.wid % 4);
        for (j = 0; j < current.wid; j++)
        {
            if (imagebyte[k] == 255)
                img[i][j] = 0;
            else
                img[i][j] = 1;
            k += 3;
        }
    }
    //--max_iter N
    //			Максимальное число поколений которое может эмулировать программа. Необязательный параметр, по-умолчанию бесконечность
    //--dump_freq N
    //Частота с которой программа должно сохранять поколения виде картинки. Необязательный параметр, по-умолчанию равен 1

    int maxiter = 1000000;
    int dumpfreq = 1;
    //проход по поколениям программы
    for (l = 0; l <= maxiter; ++l)
    {
        //если такт сохранения картинки - создавай новый bmp файл
        if (l % dumpfreq == 0)
        {
            char filename[l];
            //create the new folder
            char directFile[20];
            strcpy(directFile, folderName);
            strcat(strcat(directFile, "\\"), strcat(itoa(l, filename, 10), ".bmp"));
            FILE *photo = fopen(directFile, "w");
            //запись заголовка исходного поколения в новый bmp файл
            fwrite(header, 1, 54, photo);
            //создание самого bmp файла (pixel array)
            m = 0;
            for (int i = current.hei - 1; i > -1; --i)
            {
                for (j = 0; j < current.wid; ++j)
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (img[i][j] == 1)
                            imagebyte[m] = 0;
                        else
                            imagebyte[m] = 255;
                        m++;
                    }
                }
                //дополнение нулевыми байтами размера строки растра
                while (m % 4 != 0)
                {
                    imagebyte[m] = 0;
                    m++;
                }
            }
            fwrite(imagebyte, 1, current.size, photo);
            fclose(photo);
        }
        gameRules(img, current.hei, current.wid);
    }
    free(img);
    return 0;
}