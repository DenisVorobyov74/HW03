#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#define FileName_Size  256
#define WordArray_Size 1024
#define HashPairArray_AdditionStep 10
#define Coefficient_K 1

struct HashPair {
    char* InitialWord;
    int64_t Hash;
    int Count;
};

struct HashPairArray{

    struct HashPair** Array;
    size_t Count;
    size_t FreeItems;

};

//////////////////////////////////////////////////////////////////
/////////// Вспомогательные функции //////////////////////////////
//////////////////////////////////////////////////////////////////
// Выводит хранимые слова в консоль
void PrintWords(struct HashPairArray* HashPairArray){

    printf("\n\n        Word's list:\n");
    for(size_t i = 0; i < HashPairArray->Count; i++)
        if(HashPairArray->Array[i] != NULL)
            printf("%s -- count - %d\n", HashPairArray->Array[i]->InitialWord, HashPairArray->Array[i]->Count);

}

// Освобождает взятую в процессе выполнения программы память
void FreeMemory(struct HashPairArray* HashPairArray){

    for(size_t i = 0; i < HashPairArray->Count; i++){
        if(HashPairArray->Array[i] != 0)
            free(HashPairArray->Array[i]->InitialWord);
        free(HashPairArray->Array[i]);
    }

    free(HashPairArray);
    HashPairArray = 0;

}

// Кастумная функция копирования строк
size_t strlcpy(char *dst, const char *src, size_t dsize) {

	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1);	/* count does not include NUL */
}

// Вычисляет значение хэша
int64_t GetHash(char WordArray[WordArray_Size], size_t* WordLen){

    int64_t NewHash = 0, p_pow = 1;
    const int p = 52;
    size_t MaxLen = *WordLen-1;

    for(size_t i = 0; i < MaxLen; i++){
        NewHash += (int)WordArray[i] * p_pow;
        p_pow *= p;
    }

    return NewHash;

}

// Получаем адреса файлов.
void GetFullPathToFile(int argc, char* argv[], char PathToSrcFile[]){

    // Обрабатываем аргументы командной строки (если они есть в "достаточном" количестве)
    if(argc == 2){
        strlcpy(PathToSrcFile, argv[1], FileName_Size - 1);
    }
    else{

        // Если аргументы отсутсвует, работаем через интерфейс
        printf("        Welcome to the word counting program.\n");

        printf("Enter full path to the source file (max len - 250ch): ");
        scanf("%250s", PathToSrcFile);
    }
}

// Просто держит окно терминала открытым.
void KeepOpenWindow(){

    char EmptyEnter[1];
    scanf("%1s", EmptyEnter);
}

// Метод выполняет открытие файлов
FILE* OpenFile(char* mPathToFile, const char Mode[]) {

    struct stat buff;
    FILE* mStreamPointer = NULL;

    if(Mode[0] == 'w')
        mStreamPointer = fopen(mPathToFile, Mode);
    else{
        // Проверяем, что введенный путь указывает на обычный файл. Если это файл - попытаемся открыть.
        if(stat(mPathToFile, &buff) == 0 && (buff.st_mode & __S_IFMT) == __S_IFREG)
            mStreamPointer = fopen(mPathToFile, Mode);
    }

    return mStreamPointer;

}

// мметод закрывает файлы
int CloseFile(FILE* mStreamPointer) {

    int CloseResult;

    if(mStreamPointer == NULL){
        // Возвращаем 0, поскольку проверка корректности указателя должна быть выполнена заранее
        CloseResult = 0;
    }
    else
        CloseResult = fclose(mStreamPointer);

    return CloseResult;

}

//////////////////////////////////////////////////////////////////
/////////// Основные функции //////////////////////////////
//////////////////////////////////////////////////////////////////

void AddNewWord(struct HashPairArray* HashPairArray, char WordArray[WordArray_Size], size_t* WordLen, const int64_t Hash, const int Ind){

    const int MaxInd = HashPairArray->Count-1;
    int NewInd = Ind;
    int Attempts = HashPairArray->Count;
    _Bool IsInsertDone = false;

    // С начала пробуем разместить слово в уже имеющиеся элементы
    do{
        NewInd += Coefficient_K;
        if(NewInd > MaxInd)
            NewInd = 0;

        if(HashPairArray->Array[NewInd] != NULL &&
           HashPairArray->Array[NewInd]->Hash == Hash &&
           strncmp(HashPairArray->Array[NewInd]->InitialWord, WordArray, (*WordLen)-1) == 0){

           HashPairArray->Array[NewInd]->Count++;
           Attempts = 1;
           IsInsertDone = true;
        }
        Attempts--;
    }while(Attempts>0);

    // Если это действительно новое слово - создаем новый элемент массива
    if(!IsInsertDone){
        NewInd = Ind;

        struct HashPair* NewHashPair = malloc(sizeof(struct HashPair));

        NewHashPair->InitialWord = malloc((*WordLen) * sizeof(int));
        strncpy(NewHashPair->InitialWord, WordArray, *WordLen);
        NewHashPair->Hash  = Hash;
        NewHashPair->Count = 1;

        do{
            if(HashPairArray->Array[NewInd] == NULL){
                HashPairArray->Array[NewInd] = NewHashPair;
                IsInsertDone = true;
                HashPairArray->FreeItems--;
            }
            else {
                NewInd += Coefficient_K;
                if(NewInd > MaxInd)
                    NewInd = 0;
            }
        }while(!IsInsertDone);
    }
}

// Метод разрешает коллизиции. Вычисляет свободное место в массиве по Coefficient_K при перестроении таблицы
void InsertHashPairItem(struct HashPairArray* TempArray, struct HashPair* HashPair, const int Ind){

    const int MaxInd = TempArray->Count-1;
    int NewInd = Ind;
    _Bool IsInsertDone = false;

    do{
        NewInd += Coefficient_K;
        if(NewInd > MaxInd)
            NewInd = 0;

        if(TempArray->Array[NewInd] == NULL){
            TempArray->Array[NewInd] = HashPair;
            IsInsertDone = true;
            TempArray->FreeItems--;
        }

    }while(!IsInsertDone);

}

void IncreaseArraySize(struct HashPairArray** HashPairArray){

    int Ind, LastCount = 0;
    struct HashPairArray* TempArray  = malloc(sizeof(struct HashPairArray));
    struct HashPairArray* LocPointer = *HashPairArray;

    if(LocPointer != NULL)
        LastCount = LocPointer->Count;

    TempArray->FreeItems = LastCount + HashPairArray_AdditionStep;
    TempArray->Count     = LastCount + HashPairArray_AdditionStep;
    TempArray->Array     = calloc(TempArray->Count, sizeof(struct HashPair*));

    if(LocPointer != NULL) {

        for(size_t i = 0; i < LocPointer->Count; i++){
            Ind = LocPointer->Array[i]->Hash % TempArray->Count;
            if(LocPointer->Array[i]->Hash == 278596)
                LocPointer->Array[i]->Hash = 278596;
            if(TempArray->Array[Ind] == 0){
                TempArray->Array[Ind] = LocPointer->Array[i];
                TempArray->FreeItems--;
            }
            else
                InsertHashPairItem(TempArray, LocPointer->Array[i], Ind);
        }
        free(*HashPairArray);
    }

    *HashPairArray = TempArray;

}

void AddToHashPairArray(struct HashPairArray** LocPointer, char WordArray[WordArray_Size], size_t* WordLen){

    int Ind;
    int64_t Hash;
    struct HashPairArray* HashPairArray = *LocPointer;

    if(HashPairArray == 0 || HashPairArray->FreeItems == 0)
        IncreaseArraySize(&HashPairArray);

    Hash = GetHash(WordArray, WordLen);
    Ind  = Hash % HashPairArray->Count;
    if(Hash == 278596)
       Hash = 278596;
    // Порядок условий расположен от простого к сложному; последнее условие - самое "тяжелое" - сравнение строк
    // Поиск места хранения идет через индекс. Сравниваем значение хэша и воизбежении коллизий сравниванием строки
    if(HashPairArray->Array[Ind] != NULL &&                                             // Если данный содержит какое-то значение
       HashPairArray->Array[Ind]->Hash == Hash &&                                       // Если хэши значений совпадают
       strncmp(HashPairArray->Array[Ind]->InitialWord, WordArray, (*WordLen)-1) == 0){  // Если строки равны

        // Если очередное слово уже содержится в массиве слов, то добавляем 1
        HashPairArray->Array[Ind]->Count++;
    }else{

        // Если это новое слово, то заносим его в массив.
        AddNewWord(HashPairArray, WordArray, WordLen, Hash, Ind);
    }

    *LocPointer = HashPairArray;
}

int GetNextWord(FILE* StreamPointer, char WordArray[WordArray_Size]){

    int WordLen = 0;
    int NewChar;
    int MaxLen = WordArray_Size - 1;

    while((NewChar = fgetc(StreamPointer)) != EOF){

        // Символы-разделители слов.
        if(NewChar == ' ' || NewChar == ',' || NewChar == '.' || NewChar == '(' || NewChar == ')' ||
           NewChar == '\n' || NewChar == '\t' || NewChar == '[' || NewChar == ']'){

            if(WordLen == 0)
                continue;
            else
                break;
        }

        WordArray[WordLen] = (char)NewChar;
        WordLen++;

        if(WordLen == MaxLen)
            break;
    }

    if(WordLen > 0) {
        WordArray[WordLen] = '\0';
        WordLen++;
    }

    return WordLen;

}

void Calculate(FILE* StreamPointer){

    size_t WordLen;
    char WordArray[WordArray_Size];

    struct HashPairArray* HashPairArray = 0;

    while((WordLen = GetNextWord(StreamPointer, WordArray)) > 1)
        AddToHashPairArray(&HashPairArray, WordArray, &WordLen);

    PrintWords(HashPairArray);
    FreeMemory(HashPairArray);

}

int main(int argc, char* argv[]){
    char PathToSrcFile[FileName_Size];
    FILE* StreamPointer;
    int Result = 0, CloseResult;
    _Bool OperationDone = true;

    GetFullPathToFile(argc, argv, PathToSrcFile);
    StreamPointer = OpenFile(PathToSrcFile, "rb");
    if(StreamPointer == NULL){
        perror("Error open source file. Unknown file.\n");
        OperationDone = false;
    }

    // Выполняем подсчет
    if(OperationDone == true){
        Calculate(StreamPointer);
    }
    else
        Result = 1;

    // Если все прошло хорошо, закрываем файлы.
    CloseResult = CloseFile(StreamPointer);
    if(CloseResult == EOF){
        perror("Error close source file.\n");
        OperationDone = false;
    };

    if(OperationDone == true)
        printf("\nCounting done!");

    KeepOpenWindow();

    return Result;
}
