#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#define FileName_Size  256
#define WordArray_Size 1024
#define HashPairArray_AdditionStep 10

struct HashPair {
    char* InitialWord;
    int64_t Hash;
    int count;
};

struct HashPairArray{

    struct HashPair** Array;
    int Len;
    int Pointer;

};

void GetFullPathToFile(int argc, char* argv[], char PathToSrcFile[]);
void Calculate(FILE* StreamPointer);
void KeepOpenWindow();
FILE* OpenFile(char* mPathToFile, const char Mode[]);
int CloseFile(FILE* mStreamPointer);
int GetNextWord(FILE* StreamPointer, char WordArray[WordArray_Size]);
void AddToHashPairArray(struct HashPairArray* HashPairArray, char WordArray[WordArray_Size], int* WordLen);
void AddNewWord(struct HashPairArray* HashPairArray, char WordArray[WordArray_Size], int* WordLen);
int64_t GetHash(char WordArray[WordArray_Size], int* WordLen);
void FreeMemory(struct HashPairArray* HashPairArray);
void PrintWords(struct HashPairArray* HashPairArray);
size_t strlcpy(char *dst, const char *src, size_t dsize);

int main(int argc, char* argv[0])
{
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

void Calculate(FILE* StreamPointer){

    int WordLen;
    char WordArray[WordArray_Size];

    struct HashPairArray* HashPairArray = calloc(1, sizeof(struct HashPairArray));
    HashPairArray->Pointer = -1;

    while((WordLen = GetNextWord(StreamPointer, WordArray)) > 1)
        AddToHashPairArray(HashPairArray, WordArray, &WordLen);

    PrintWords(HashPairArray);
    FreeMemory(HashPairArray);

}

void PrintWords(struct HashPairArray* HashPairArray){

    printf("\n\n        Word's list:\n");
    for(int i = 0; i <= HashPairArray->Pointer; i++)
        printf("%s -- count - %d\n", HashPairArray->Array[i]->InitialWord, HashPairArray->Array[i]->count);

}

void FreeMemory(struct HashPairArray* HashPairArray){

    for(int i = 0; i <= HashPairArray->Pointer; i++){
        free(HashPairArray->Array[i]->InitialWord);
        free(HashPairArray->Array[i]);
    }

    free(HashPairArray);
    HashPairArray = 0;

}

void AddToHashPairArray(struct HashPairArray* HashPairArray, char WordArray[WordArray_Size], int* WordLen){

    _Bool IsFound = false;
    int i;

    // Выделяем место для новых элементов массива с шагом HashPairArray_AdditionStep
    if(HashPairArray->Len <= (HashPairArray->Pointer + 1)){

        if(HashPairArray->Len == 0)
            HashPairArray->Array = calloc(HashPairArray_AdditionStep, sizeof(struct HashPair*));
        else
            HashPairArray->Array = realloc(HashPairArray->Array, (HashPairArray->Len + HashPairArray_AdditionStep) * sizeof(struct HashPair*));

        HashPairArray->Len += HashPairArray_AdditionStep;
    }

    for(i = 0; i <= HashPairArray->Pointer; i++){
        if(strncmp(HashPairArray->Array[i]->InitialWord, WordArray, *WordLen) == 0){
            IsFound = true;
            break;
        }
    }

    // Если очередное слово уже содержится в массиве слов, то добавляем 1
    if(IsFound)
        HashPairArray->Array[i]->count++;
    else{
    // Если это новое слово, то заносим его в массив.
        AddNewWord(HashPairArray, WordArray, WordLen);
    }

}

void AddNewWord(struct HashPairArray* HashPairArray, char WordArray[WordArray_Size], int* WordLen){

    struct HashPair* Temp = calloc(1, sizeof(struct HashPair));

    HashPairArray->Pointer++;
    HashPairArray->Array[HashPairArray->Pointer] = Temp;

    Temp->count++;

    Temp->InitialWord = malloc((*WordLen) * sizeof(int));
    strncpy(Temp->InitialWord, WordArray, *WordLen);

    Temp->Hash = GetHash(WordArray, WordLen);

}

int64_t GetHash(char WordArray[WordArray_Size], int* WordLen){

    int64_t NewHash = 0, p_pow = 1;
    const int p = 52;
    int MaxLen = *WordLen-1;

    for(int i = 0; i < MaxLen; i++){
        NewHash += (int)WordArray[i] * p_pow;
        p_pow *= p;
    }

    return NewHash;

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
