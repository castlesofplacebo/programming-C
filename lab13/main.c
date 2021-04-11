#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//bog.pp.ru/dw/tags.html
//http://id3.org/id3v2.3.0#ID3v2_header
//� ������ ���� ��� ��������� (10 ����): "ID3", ��������� ID3v2 (0x0200), ����� (1 ����: ������������� ��������� ���������������, ������������� ������, ��������� - ������), ����� ���� ��� ��������� (4 �����, � ������ ����� ������������ ������ ������� 7 ���), �� ������� ��� ������������������ ������. ������ ���� ����� ���������: 4-� ���������� ���, 4-������� ����� ����� ��� ���������. �����, ������������ � 'X', 'Y' ��� 'Z', ��������� ������������������ (�����: ����� ����������� ���� �����). �� ���������, ������������ ��������� ISO-8859-1 (���������� ������ - 0x00). ��������� (�� ���, � ������ ����� ������� ���������� � 'T'?) ���� ������ ����� ������������ UCS-2 (Unicode), ���������� ������ - 0x0000, ��� ����������� ��� ����������� ������� ���� ������� BOM ������������ ������, ����� ����� ����� ������������ ���� � ������ ����� (0 - ISO-8859-1, 1 - UCS-2). ��������� ����� ����� ����� 3-� ���������� ����, ����������� ���� (ISO-639-2). ������������ � ������ URL ����� ���� ��������������. �������� ����� (����� �� ����� 80, � ������� ���� ����� ��� ID3v2.3 � ID3v2.4):

//��� -> ����� -> ��������� ������ -> ����� ... -> �����

char *getName(char *str)
{                                 //���������� ����� ����� �� ������� ������ --filepath=Song.mp3
    unsigned int n = strlen(str); //������� � ������ ������, ���������� ��� �����
    char *memory = (char *)malloc(sizeof(char) * n - 9);
    memory[n - 8] = '\0';
    for (unsigned int i = 0; i < n - 9; ++i)
    {
        memory[i] = str[i + 11];
    }
    return memory;
}

char *byte(FILE *current, int n)
{ //������ ������������� ���������� ������ �� ������
    char *memory = (char *)malloc(sizeof(char) * (n + 1));
    memory[n] = '\0';
    for (unsigned int i = 0; i < n; ++i)
    {
        memory[i] = getc(current);
    }
    return memory;
}

int size(FILE *current, int n)
{ //������ ����� ����
    char *memory = (char *)malloc(sizeof(char) * n);
    int curSize = 0;
    for (unsigned int i = 0; i < n; ++i)
    {
        memory[i] = getc(current);
        curSize = curSize << 7;
        curSize = curSize | memory[i]; //����� ���� ��� ��������� (4 �����, � ������ ����� ������������ ������ ������� 7 ���)
    }
    return curSize;
}

void currentFrame(FILE *current)
{
    //����� ������ ����� 4-� �����������, ����� ����� ����� ������� ������ 4-�� �������
    char *frameName = byte(current, 4);
    int frameLen = size(current, 4);
    // ��������� ����� ������� ���� ����� (2 �����) - �������� ������ �� 2 �����
    fseek(current, 2, SEEK_CUR);
    //Only text frame identifiers begin with "T", with the exception of the "TXXX" frame
    if (frameName[0] == 'T')
    {
        char *meta = byte(current, frameLen);
        printf("%s  %s\n", frameName, meta);
    }
    else
        fseek(current, frameLen, SEEK_CUR);
}

char *tag(char *str, int n)
{
    unsigned int m = strlen(str);
    char *memory = (char *)malloc(sizeof(char) * (m - n + 1));
    memory[m - n] = '\0';
    for (unsigned int i = 0; i < m - n; ++i)
        memory[i] = str[n + i];
    return memory;
}

char *isFrame(FILE *current, char *str)
{
    //����� ������ ����� 4-� �����������, ����� ����� ����� ������� ������ 4-�� �������
    char *frameName = byte(current, 4);
    int frameLen = size(current, 4);
    // ��������� ����� ������� ���� ����� (2 �����) - �������� ������ �� 2 �����
    fseek(current, 2, SEEK_CUR);
    //Only text frame identifiers begin with "T", with the exception of the "TXXX" frame
    if (!strcmp(frameName, str))
    {
        char *meta = byte(current, frameLen);
        printf("%s  %s\n", frameName, meta);
        return "YES";
    }
    else
    {
        fseek(current, frameLen, SEEK_CUR);
        return "NO";
    }
}

void intInTag(FILE *current, int value)
{ //���������� ����� � ���
    char *buf = (char *)malloc(sizeof(char) * 4);
    for (int i = 0; i < 4; ++i)
    {
        buf[i] = (value >> ((3 - i) * 7)) & 0xFF;
        buf[i] = buf[i] & 0x7F;
        putc(buf[i], current);
    }
}

void valueInTag(FILE *current, char *value, int n)
{ //���������� �������� � ���
    for (int i = 0; i < n; ++i)
        putc(value[i] & 0xFF, current);
}

int tagLenght(FILE *current, char *tag)
{ //����������� ����� ���� ��� ����� 0(� ������ ���������� ����)
    char *frameName = byte(current, 4);
    int frame_len = size(current, 4);
    fseek(current, 2, SEEK_CUR);
    if (!strcmp(frameName, tag))
    {
        return frame_len;
    }
    else
    {
        fseek(current, frame_len, SEEK_CUR);
        return 0;
    }
}

int main(int argc, char *argv[])
{
    char *name;
    char *idHeader;
    FILE *current;
    name = getName(argv[1]);
    current = fopen(name, "rb");
    // ID3v2 header
    //The first three bytes of the tag are always "ID3" to indicate that this is an ID3v2 tag
    idHeader = byte(current, 3);
    //��������, �������� �� ���� �������� ID3
    //ID3v2/file identifier   "ID3"
    //ID3v2 version           $03 00
    //ID3v2 flags             %abc00000
    //ID3v2 size              4 * %0xxxxxxx

    if (strcmp(idHeader, "ID3"))
    {
        printf("It is not ID3 format");
        return 0;
    }
    //��������� ����� ��������� 3 �����

    fseek(current, 3, SEEK_CUR);

    // � ������ ���� ��� ��������� (10 ����): "ID3", ��������� ID3v2 (0x0200), ����� (1 ����: ������������� ��������� ���������������, ������������� ������, ��������� - ������), ����� ���� ��� ��������� (4 �����, � ������ ����� ������������ ������ ������� 7 ���), �� ������� ��� ������������������ ������.

    int sizeFrames = size(current, 4); //������ ���� �����

    if (!strcmp(argv[2], "--show"))
    {
        while (ftell(current) < sizeFrames - 10)
        { //������ �����
            currentFrame(current);
        }
        return 0;
    }

    //���� ������ 6 �������� ��������� � --get= :
    if (!strncmp(argv[2], "--get=", 6))
    {
        char *str = tag(argv[2], 6);
        while (ftell(current) < sizeFrames - 10)
        { //������ �����
            char *curFrame = isFrame(current, str);
            if (!strcmp(curFrame, "YES"))
                return 0;
        }
        printf("NO FRAME");
        return 0;
    }

    //���� ������ 6 �������� ��������� � --set= :
    if (!strncmp(argv[2], "--set=", 6))
    {
        char *tag1 = tag(argv[2], 6);  //������ ����
        char *value = tag(argv[3], 8); //������ �������� ����

        int len = 0;
        while (ftell(current) < sizeFrames - 10)
        {
            len = tagLenght(current, tag1);
            if (len)
                break;
        }

        //����������� ����� ����
        int position = ftell(current) - 6; //����������� ������� ������� (������� ��� 6 ����)

        fseek(current, 0, SEEK_SET);

        //������� ����� ����
        FILE *edit = fopen("at.mp3", "wb");

        //��������� ������ 6 ���� � ����� ����
        for (int i = 0; i < 6; i++)
        {
            char c = getc(current);
            putc(c, edit);
        }

        //���������� ����� ��������� ����� � ������
        sizeFrames = sizeFrames - len + strlen(value); //����������� ����� ����� ���� ������� : �������� ����� ���� ����� - ����� ��������� �������� ���� + ����� ����������� �������� ����

        //���������� ����� ����� ���� �������
        intInTag(edit, sizeFrames);
        fseek(current, ftell(edit), SEEK_SET);

        while (ftell(edit) != position)
        {
            char c = getc(current);
            putc(c, edit);
        }
        //���������� ����� ����
        intInTag(edit, strlen(value) + 1);
        //������� �����
        putc(0x00, edit);
        putc(0x00, edit);
        putc(0x00, edit);
        //��������� ��������� �������� ����
        valueInTag(edit, value, strlen(value));

        fseek(current, 4 + 2 + len, SEEK_CUR);
        //���������� ���������� ����� �����
        while (1)
        {
            char c = getc(current);
            if (c == EOF)
                if (feof(current) != 0)
                    break;
            putc(c, edit);
        }
        fclose(current);
        fclose(edit);
        //������� �������� ����
        remove(name);
        //��������������� ��� ������ ����
        rename("at.mp3", name);
    }
    return 0;
}