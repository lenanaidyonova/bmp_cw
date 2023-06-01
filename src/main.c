#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#pragma pack (push, 1)
typedef struct
{
    unsigned short signature;
    unsigned int filesize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int pixelArrOffset;
} BitmapFileHeader;

typedef struct
{
    unsigned int headerSize;
    unsigned int width;
    unsigned int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    unsigned int xPixelsPerMeter;
    unsigned int yPixelsPerMeter;
    unsigned int colorsInColorTable;
    unsigned int importantColorCount;
} BitmapInfoHeader;

typedef struct
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Rgb;

#pragma pack(pop)

typedef struct{
    int x0, y0, x1, y1;
    char sign;
    unsigned int r, g, b;
    char* point;
    int thick;
    int help, inversion, black_and_white, resize, setLine, info, clean;
}Configs;

void printHelp(){
    printf("Руководство по использованию программы:\n");
    printf("-Программа обрабатывает BMP-файлы версии V3. С глубиной изобжения 24 бита на пиксель\n");
    printf("-Для запуска программы необходимо передать следующие аргументы:\n");
    printf("\t./a.out  -- имя исполняемого файла\n");
    printf("\t<filename>  -- имя BMP-файла, который необходимо обработать. Он должен находиться в текущей директории\n");
    printf("\t-f or --func  -- ключ для вызова функции (указаны в списке ключей)\n");
    printf("\t<arg1>,<arg2> ... -- аргументы к ключу, если требуются (указаны в списке ключей, аргументы разделяются запятой)\n");
    printf("-Список ключей и их аргументы:\n");
    printf("\t--help or -h -- вывод руководства по использованию программы\n");
    printf("\t--info or -i -- вывод информации о BMP-файле, значения полей его заголовков\n");
    printf("\t--inversion or -v -- инверсия цвета в заданной области\n");
    printf("\t--blackWhite or -w  -- преобразование в ЧБ заданной области\n");
    printf("\t--start or -u <x0>,<y0> -- задание начальных координат\n");
    printf("\t--end or -d <x1>,<y1> -- задание конечных координат\n");
    printf("\t\t<x0> - координата Х левого верхнего угла области\n");
    printf("\t\t<y0> - координата Y левого верхнего угла области\n");
    printf("\t\t<x1> - координата X правого нижнего угла области\n");
    printf("\t\t<y1> - координата Y правого нижнего угла области\n");
    printf("\t--color or -o <r>,<g>,<b> -- задание цвета\n");
    printf("\t--resize or -z -- изменение размера изображения с его обрезкой или расширением фона\n");
    printf("\t--point or -p <point> -- задание точки, относительно которой будет происходить обрезка\n");
    printf("\t--sign or -s -- увеличение или уменьшие изображения\n");
    printf("\t\t<sign> - символ '+', если нужно расширить фон, '-' если обрезать\n");
    printf("\t\t<r>,<g>,<b> - необходимо ввести число от 0 до 255, если <sign> = '-', выбор цвета не сыграет роли\n");
    printf("\t\t<point> - На выбор: leftDown, leftUp, rightDown, rightUp, centre\n");
    printf("\t--setLine or -l -- рисование отрезка\n");
    printf("\t--thick or -t <k> -- задание толщины <k> отрезка в пикселях\n");
    printf("-Результат успешной обработки изображения записывается в файл simp.bmp\n");
}

void printFileHeader(BitmapFileHeader header){
    printf("Информация о BMP-файле:\n");
    printf("signature:\t%x (%hu)\n", header.signature, header.signature);
    printf("filesize:\t%x (%u)\n", header.filesize, header.filesize);
    printf("reserved1:\t%x (%hu)\n", header.reserved1, header.reserved1);
    printf("reserved2:\t%x (%hu)\n", header.reserved2, header.reserved2);
    printf("pixelArrOffset:\t%x (%u)\n", header.pixelArrOffset, header.pixelArrOffset);
}

void printInfoHeader(BitmapInfoHeader header){
    printf("Поля структуры BitmapInfoHeader:\n");
    printf("headerSize:\t%x (%u)\n", header.headerSize, header.headerSize);
    printf("width:     \t%x (%u)\n", header.width, header.width);
    printf("height:    \t%x (%u)\n", header.height, header.height);
    printf("planes:    \t%x (%hu)\n", header.planes, header.planes);
    printf("bitsPerPixel:\t%x (%hu)\n", header.bitsPerPixel, header.bitsPerPixel);
    printf("compression:\t%x (%u)\n", header.compression, header.compression);
    printf("imageSize:\t%x (%u)\n", header.imageSize, header.imageSize);
    printf("xPixelsPerMeter:\t%x (%u)\n", header.xPixelsPerMeter, header.xPixelsPerMeter);
    printf("yPixelsPerMeter:\t%x (%u)\n", header.yPixelsPerMeter, header.yPixelsPerMeter);
    printf("colorsInColorTable:\t%x (%u)\n", header.colorsInColorTable, header.colorsInColorTable);
    printf("importantColorCount:\t%x (%u)\n", header.importantColorCount, header.importantColorCount);
}

void free_arr(Rgb** arr, unsigned int H){
    for(int i=0; i < H; i++){
        free(arr[i]);
    }free(arr);
}

//Перевернуть изображение
void reverse(Rgb** arr, unsigned int H){
    Rgb* t;
    for (int i= 0; i< H/2; i++){
        t = arr[i];
        arr[i] = arr[H-i-1];
        arr[H-i-1] = t;
    }
}

void put_to_file(BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, unsigned int W, unsigned int H, Rgb** arr, char* file){
    FILE *ff = fopen(file, "wb");

    unsigned int offset = (W * sizeof(Rgb)) % 4;
    offset = (offset ? 4-offset : 0);

    bmif->height = H;
    bmif->width = W;

    fwrite(bmfh, 1, sizeof(BitmapFileHeader),ff);
    fwrite(bmif, 1, sizeof(BitmapInfoHeader),ff);

    reverse(arr, H);

    for(int i=0; i<H; i++){
        fwrite(arr[i],1,W * sizeof(Rgb) + offset,ff);
    }

    free_arr(arr, H);
    fclose(ff);
}


void inversion(int x0, int y0, int x1, int y1, Rgb** arr, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, char* file){
    unsigned int H = bmif->height;
    unsigned int W = bmif->width;
    for(int i=y0; i < y1; i++){
        for (int j=x0; j<x1; j++){
            arr[i][j].r = 255 - arr[i][j].r;
            arr[i][j].b = 255 - arr[i][j].b;
            arr[i][j].g = 255 - arr[i][j].g;
        }
    }
    put_to_file(bmfh, bmif, W, H, arr, file);
}


void black_and_white(int x0, int y0, int x1, int y1, Rgb** arr, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, char* file){
    unsigned int H = bmif->height;
    unsigned int W = bmif->width;
    for(int i=y0; i < y1; i++){
        for (int j=x0; j<x1; j++){
            unsigned char new_color = (arr[i][j].r + arr[i][j].b + arr[i][j].g) /3;
            arr[i][j].r = new_color;
            arr[i][j].b = new_color;
            arr[i][j].g = new_color;
        }
    }
    put_to_file(bmfh, bmif, W, H, arr, file);
}


void add_frames(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int H_old, unsigned int H_new, unsigned int W_new,
                BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, unsigned char new_r, unsigned char new_g, unsigned char new_b, Rgb** arr, char* file){
    Rgb **arr_new = malloc(H_new*sizeof(Rgb*));
    unsigned int offset = (W_new * sizeof(Rgb)) % 4;
    offset = (offset ? 4-offset : 0);
    for(int i=0; i<H_new; i++){
        arr_new[i] = malloc(W_new * sizeof(Rgb) + offset);
    }
    int flag = 0;
    int y_old = 0;
    for (int i=0; i< H_new; i++){
        int x_old = 0;
        for (int j=0; j<W_new; j++){
            if (i >= y0 && j >= x0 && i <= y1 && j <= x1){
                arr_new[i][j] = arr[y_old][x_old];
                x_old++;
                flag = 1;
            }
            else{
                arr_new[i][j].r = new_r;
                arr_new[i][j].b = new_b;
                arr_new[i][j].g = new_g;
            }
        }
        if (flag == 1) y_old++;
    }

    free_arr(arr, H_old);
    put_to_file(bmfh, bmif, W_new, H_new, arr_new, file);
}

void cut_frames(unsigned int x0, unsigned int y0, unsigned int H_old, unsigned int H_new, unsigned int W_new,
                BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, Rgb** arr, char* file){
    Rgb **arr_new = malloc(H_new*sizeof(Rgb*));
    unsigned int offset = (W_new * sizeof(Rgb)) % 4;
    offset = (offset ? 4-offset : 0);
    for(int i=0; i<H_new; i++){
        arr_new[i] = malloc(W_new * sizeof(Rgb) + offset);
    }
    for (int i =0; i < H_new; i++){
        for (int j=0; j < W_new; j++){
            arr_new[i][j] = arr[y0+i][x0+j];
        }
    }
    free_arr(arr, H_old);
    put_to_file(bmfh, bmif, W_new, H_new, arr_new, file);
}


void resize(char sign, unsigned char new_r, unsigned char new_g, unsigned char new_b, char* move,
            BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, Rgb** arr, char* file){
    unsigned int thickness = 200;

    unsigned int H = bmif->height;
    unsigned int W = bmif->width;

    if (sign == '-'){
        if (!strcmp(move, "leftUp")){
            unsigned int y0 = 0;
            unsigned int x0 = 0;
            unsigned int H_new = H - thickness;
            unsigned int W_new = W - thickness;
            cut_frames(x0, y0,H, H_new, W_new, bmfh, bmif, arr, file);
        }
        if (!strcmp(move, "leftDown")){
            unsigned int y0 = thickness - 1;
            unsigned int x0 = 0;
            unsigned int H_new = H - thickness;
            unsigned int W_new = W - thickness;
            cut_frames(x0, y0,H, H_new, W_new, bmfh, bmif, arr, file);
        }
        if (!strcmp(move, "rightUp")){
            unsigned int y0 = 0;
            unsigned int x0 = thickness - 1;
            unsigned int H_new = H - thickness;
            unsigned int W_new = W - thickness;
            cut_frames(x0, y0,H, H_new, W_new, bmfh, bmif, arr, file);
        }
        if (!strcmp(move, "rightDown")){
            unsigned int y0 = thickness - 1;
            unsigned int x0 = thickness - 1;
            unsigned int H_new = H - thickness;
            unsigned int W_new = W - thickness;
            cut_frames(x0, y0,H, H_new, W_new, bmfh, bmif, arr, file);
        }
        if (!strcmp(move, "centre")){
            unsigned int y0 = thickness - 1;
            unsigned int x0 = thickness - 1;
            unsigned int H_new = H - 2 * thickness;
            unsigned int W_new = W - 2 * thickness;
            cut_frames(x0, y0,H, H_new, W_new, bmfh, bmif, arr, file);
        }
    }
    if (sign == '+'){
        if (!strcmp(move, "leftUp")){
            unsigned int y0 = 0;
            unsigned int x0 = 0;
            unsigned int y1 = H-1;
            unsigned int x1 = W-1;
            unsigned int H_new = H + thickness;
            unsigned int W_new = W + thickness;
            bmfh->filesize += 3 * thickness * (H_new + W_new - thickness);
            add_frames(x0, y0, x1, y1,H, H_new, W_new, bmfh, bmif, new_r, new_g, new_b, arr, file);
        }
        if (!strcmp(move, "leftDown")){
            unsigned int y0 = thickness;
            unsigned int x0 = 0;
            unsigned int y1 = H + thickness - 1;
            unsigned int x1 = W-1;
            unsigned int H_new = H + thickness;
            unsigned int W_new = W + thickness;
            bmfh->filesize += 3 * thickness * (H_new + W_new - thickness);
            add_frames(x0, y0, x1, y1,H, H_new, W_new, bmfh, bmif, new_r, new_g, new_b, arr, file);
        }
        if (!strcmp(move, "rightUp")){
            unsigned int y0 = 0;
            unsigned int x0 = thickness;
            unsigned int y1 = H-1;
            unsigned int x1 = W-1 + thickness;
            unsigned int H_new = H + thickness;
            unsigned int W_new = W + thickness;
            bmfh->filesize += 3 * thickness * (H_new + W_new - thickness);
            add_frames(x0, y0, x1, y1,H, H_new, W_new, bmfh, bmif, new_r, new_g, new_b, arr, file);
        }
        if (!strcmp(move, "rightDown")){
            unsigned int y0 = thickness;
            unsigned int x0 = thickness;
            unsigned int y1 = H-1 + thickness;
            unsigned int x1 = W-1 + thickness;
            unsigned int H_new = H + thickness;
            unsigned int W_new = W + thickness;
            bmfh->filesize += 3 * thickness * (H_new + W_new - thickness);
            add_frames(x0, y0, x1, y1,H, H_new, W_new, bmfh, bmif, new_r, new_g, new_b, arr, file);
        }
        if (!strcmp(move, "centre")){
            unsigned int y0 = thickness;
            unsigned int x0 = thickness;
            unsigned int y1 = H-1 + thickness;
            unsigned int x1 = W-1 + thickness;
            unsigned int H_new = H + 2 * thickness;
            unsigned int W_new = W + 2 * thickness;
            bmfh->filesize += 6 * thickness * (H_new + W_new);
            add_frames(x0, y0, x1, y1,H, H_new, W_new, bmfh, bmif, new_r, new_g, new_b, arr, file);
        }

    }
}

void setPoint(Rgb** arr, int x, int y, unsigned int r, unsigned int g, unsigned int b){
    arr[y][x].r = r;
    arr[y][x].g = g;
    arr[y][x].b = b;
}


void set_line(int x0, int y0, int x1, int y1, int thickness, unsigned char r, unsigned char g, unsigned char b,
              BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, Rgb** arr, char* file){
    unsigned int H = bmif->height;
    unsigned int W = bmif->width;

    int deltaX = x1 - x0;
    int deltaY = y1 - y0;
    int absDeltaX; int absDeltaY;
    if (deltaX > 0) absDeltaX = deltaX; else absDeltaX = deltaX * (-1);
    if (deltaY > 0) absDeltaY = deltaY; else absDeltaY = deltaY * (-1);

    int lambda = 0;

    if (absDeltaX >= absDeltaY) {
        int y = y0;
        int direction;
        if (deltaY != 0) {
            if (deltaY > 0) {
                direction = 1;
            } else direction = -1;
        } else direction = 0;

        if (deltaX > 0) {
            for (int x = x0; x <= x1; x++) {
                setPoint(arr, x, y, r, g, b);
                lambda += absDeltaY;
                if(thickness!=1){
                    if(y0 == y1){
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x,y+i,r,g,b);
                        }
                    }
                    else
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x,y+i,r,g,b);
                        }
                }
                if (lambda >= absDeltaX) {
                    lambda -= absDeltaX;
                    y += direction;
                }
            }
        } else {
            for (int x = x0; x >= x1; x--) {
                setPoint(arr, x, y, r, g, b);
                if(thickness!=1){
                    if(y0 == y1){
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x,y+i,r,g,b);
                        }
                    }
                    else
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x,y+i,r,g,b);
                        }
                }
                lambda += absDeltaY;

                if (lambda >= absDeltaX) {
                    lambda -= absDeltaX;
                    y += direction;
                }
            }
        }
    } else {
        int x = x0;
        int direction;
        if (deltaX != 0) {
            if (deltaX > 0) {
                direction = 1;
            } else direction = -1;
        } else direction = 0;

        if (deltaY > 0) {
            for (int y = y0; y <= y1; y++) {
                setPoint(arr, x, y, r, g, b);
                if(thickness!=1){
                    if(y0 == y1){
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x,y+i,r,g,b);
                        }
                    }
                    else
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x+i,y,r,g,b);
                        }
                }
                lambda += absDeltaX;

                if (lambda >= absDeltaY) {
                    lambda -= absDeltaY;
                    x += direction;
                }
            }
        } else {
            for (int y = y0; y >= y1; y--) {
                setPoint(arr, x, y, r, g, b);
                if(thickness!=1){
                    if(y0 == y1){
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x,y+i,r,g,b);
                        }
                    }
                    else
                        for(int i=1;i<thickness;i++){
                            setPoint(arr,x+i,y,r,g,b);
                        }
                }
                lambda += absDeltaX;

                if (lambda >= absDeltaY) {
                    lambda -= absDeltaY;
                    x += direction;
                }
            }
        }
    }
    put_to_file(bmfh, bmif, W, H, arr, file);
}


void free_picture(BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, char* file, Rgb** arr){
    FILE *f = fopen("simpsons.bmp", "rb");
    BitmapFileHeader bmfh2;
    BitmapInfoHeader bmif2;
    fread(&bmfh2,1,sizeof(BitmapFileHeader),f);
    fread(&bmif2,1,sizeof(BitmapInfoHeader),f);
    unsigned int H_2 = bmif2.height;
    unsigned int W_2 = bmif2.width;
    Rgb **arr_old = malloc(H_2*sizeof(Rgb*));
    unsigned int offset = (W_2 * sizeof(Rgb)) % 4;
    offset = (offset ? 4-offset : 0);
    for(int i=0; i<H_2; i++){
        arr_old[i] = malloc(W_2 * sizeof(Rgb) + offset);
        fread(arr_old[i],1,W_2 * sizeof(Rgb) + offset,f);
    }
    free_arr(arr, bmif->height);
    bmif->width = W_2;
    bmif->height = H_2;
    reverse(arr_old, H_2);
    put_to_file(bmfh, bmif, W_2, H_2, arr_old, file);
    fclose(f);
}

int choice(Configs* config, int opt){
    int count;
    switch (opt) {
        case 'u':
            count = sscanf(optarg, "%d,%d", &config->x0, &config->y0);
            if (count < 2) return 1;
            break;
        case 'd':
            count = sscanf(optarg, "%d,%d", &config->x1, &config->y1);
            if (count < 2) return 1;
            break;
        case 's':
            count = sscanf(optarg, "%c", &config->sign);
            if (count < 1) return 1;
            break;
        case 'o':
            count = sscanf(optarg, "%d,%d,%d", &config->r, &config->g,&config->b);
            if (count < 3) return 1;
            break;
        case 'p':
            config->point = calloc(20, sizeof(char));
            count = sscanf(optarg, "%s", config->point);
            if (count < 1) return 1;
            break;
        case 't':
            count = sscanf(optarg, "%d", &config->thick);
            if (count < 1) return 1;
            break;
        case 'h':
            config->help = 1;
            break;
        case 'v':
            config->inversion = 1;
            break;
        case 'w':
            config->black_and_white = 1;
            break;
        case 'z':
            config->resize = 1;
            break;
        case 'l':
            config->setLine = 1;
            break;
        case 'i':
            config->info = 1;
            break;
        case 'c':
            config->clean = 1;
            break;
        default:
            return 2;
    }
    return 0;
}


int main(int argc, char* argv[]){

    if (strcmp(argv[argc-1], "-h") == 0 || strcmp(argv[argc-1], "--help") == 0){
        printHelp();
        return 0;
    }

    char *file = malloc(100*sizeof(char));
    strcpy(file, argv[argc-1]);

    FILE *f = fopen(file, "rb");
    if (!f){
        printf("Такого файла не существует в текущей директории или нет к нему доступа.\n");
        return 0;
    }
    BitmapFileHeader bmfh;
    BitmapInfoHeader bmif;

    if (argc < 3){
        printHelp();
        return 0;
    }

    fread(&bmfh,1,sizeof(BitmapFileHeader),f);
    if(bmfh.signature != 0x4d42){
        printf("Файл не соответствует формату BMF.\n");
        return 0;
    }

    fread(&bmif,1,sizeof(BitmapInfoHeader),f);
    if(bmif.bitsPerPixel != 24){
        printf("Изображение не содержит 24 бита на цвет.\n");
        return 0;
    }

    if(bmif.headerSize != 40){
        printf("Данная версия BMP файла не обрабатывается.\n");
        return 0;
    }

    unsigned int H = bmif.height;
    unsigned int W = bmif.width;

    Rgb **arr = malloc(H*sizeof(Rgb*));
    unsigned int offset = (W * sizeof(Rgb)) % 4;
    offset = (offset ? 4-offset : 0);
    for(int i=0; i<H; i++){
        arr[i] = malloc(W * sizeof(Rgb) + offset);
        fread(arr[i],1,W * sizeof(Rgb) + offset,f);
    }

    reverse(arr, H);

    //getopt
    char *opts = "hivwzlcu:d:o:t:p:s:";
    struct option longOpts[]={
            {"help",no_argument,NULL,'h'},
            {"inversion", no_argument, NULL, 'v'},
            {"black_and_white", no_argument, NULL, 'w'},
            {"resize", no_argument, NULL, 'z'},
            {"setLine", no_argument, NULL, 'l'},
            {"info",no_argument,NULL,'i'},
            {"clean", no_argument, NULL, 'c'},
            {"start", required_argument, NULL, 'u'},
            {"end", required_argument, NULL, 'd'},
            {"color", required_argument, NULL, 'o'},
            {"thick", required_argument, NULL, 't'},
            {"point", required_argument, NULL, 'p'},
            {"sign", required_argument, NULL, 's'},
            { NULL, 0, NULL, 0}
    };

    int longIndex;
    int opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
    Configs config = {0,0,0,0,'\0', 0, 0, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0};

    if (opt == -1){
        puts("Неверный формат ввода.");
    }

    while(opt != -1){
        int ch = choice(&config, opt);
        if (ch == 1) {
            puts("Введено недостаточное количество аргументов.");
            free_arr(arr, H);
            return 0;
        } else if (ch == 2){
            printHelp();
        }
        opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
    }

    if (config.help + config.info + config.clean + config.inversion + config.black_and_white + config.resize + config.setLine > 1){
        puts("За один вызов возможно выполнить только одну ключ-функцию.");
        free_arr(arr, H);
        return 0;
    } else if (config.help + config.info + config.clean + config.inversion + config.black_and_white + config.resize + config.setLine == 0){
        puts("Необходимо ввести ключ-функцию.");
        free_arr(arr, H);
        return 0;
    }

    if (config.help == 1){
        printHelp();
    }

    if (config.info == 1){
        printFileHeader(bmfh);
        printInfoHeader(bmif);
    }

    if (config.clean == 1){
        free_picture(&bmfh, &bmif, file, arr);
    }

    if (config.inversion == 1){
        if (config.x0 > config.x1 || config.y0 > config.y1){
            puts("Проверьте, что (x0, y0) - координаты левого верхнего угла, (x1, y1) - правого нижнего.");
        }
        else if (config.x0 >= bmif.width || config.x1 >= bmif.width || config.y0 > bmif.height || config.y1 > bmif.height){
            printf("Введите координаты в размерах картинки.\n");
        }
        else{
            inversion(config.x0, config.y0, config.x1, config.y1, arr, &bmfh, &bmif, file);
        }
    }

    if (config.black_and_white == 1){
        if (config.x0 > config.x1 || config.y0 > config.y1){
            puts("Проверьте, что (x0, y0) - координаты левого верхнего угла, (x1, y1) - правого нижнего.");
        }
        else if (config.x0 >= bmif.width || config.x1 >= bmif.width || config.y0 >= bmif.height || config.y1 >= bmif.height){
            printf("Введите координаты в размерах картинки.\n");
        }
        else{
            black_and_white(config.x0, config.y0, config.x1, config.y1, arr, &bmfh, &bmif, file);
        }
    }

    if (config.resize == 1){
        if (config.sign != '+' && config.sign != '-'){
            printf("Аргументом <sign> может быть только '+' или '-'.\n");
        }
        else if(strcmp(config.point,"leftDown") !=0 && strcmp(config.point,"leftUp")!=0
                && strcmp(config.point,"rightDown")!=0 && strcmp(config.point,"rightUp")!=0 && strcmp(config.point,"centre") != 0){
            printf("Аргумент <point> может быть одним из 5 значений: leftDown,leftUp,rightDown,rightUp,centre.\n");
        }
        if (config.sign == '-'){
            if (bmif.height <= 200 || bmif.width <= 200){
                printf("Изображение не может быть уменьшено. Так как одна из компонент размера меньше, чем величина на которую небходимо уменьшить изображение.\n");
            }
            else{
                resize('-', config.r, config.g, config.b, config.point, &bmfh, &bmif, arr, file);
            }
        }
        if (config.sign == '+'){
            if (config.r >= 0 && config.r <= 255 && config.b >= 0 && config.b <= 255 && config.g >= 0 && config.g <= 255) {
                resize('+', config.r, config.g, config.b, config.point, &bmfh, &bmif, arr, file);
            } else {
                printf("Компоненты RGB должны находиться в диапазоне от 0 до 255.\n");
            }
        }
        free(config.point);
    }

    if (config.setLine == 1){
        if (config.x0 < 0 || config.x1 < 0 || config.y0 < 0 || config.y1 < 0 || config.x0 >= bmif.width ||
            config.x1 >= bmif.width || config.y0 >= bmif.height || config.y1 >= bmif.height){
            printf("Координаты отрезка должны быть в пределах размера изображения.\n");
        }
        else if (config.thick <= 0){
            printf("Толщина отрезка должна быть положительной.\n");
        }
        else if (config.r < 0 || config.r > 255 || config.b < 0 || config.b > 255 || config.g < 0 || config.g > 255){
            printf("Компоненты RGB должны находиться в диапазоне от 0 до 255.\n");
        }
        else if (config.x0 + config.thick >= bmif.width || config.x1 + config.thick >= bmif.width || config.y0 + config.thick >= bmif.height || config.y1 + config.thick >= bmif.width){
            puts("Прямая с такой толщиной выходит за рамки изображения.");
        }
        else{
            set_line(config.x0, config.y0, config.x1, config.y1, config.thick, config.r, config.g, config.b, &bmfh, &bmif, arr, file);
        }
    }

    if (config.info == 1 || config.help == 1){
        free_arr(arr, bmif.height);
    }

    free(file);
    fclose(f);
    return 0;
}
