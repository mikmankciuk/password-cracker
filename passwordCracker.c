#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <ctype.h>
#include <signal.h>

#define PASSWORD_COUNT 894

char package[2][100];
char crackedWordList[PASSWORD_COUNT][100];
char crackedPasswordList[PASSWORD_COUNT][33];

int passwordFound = 0;
int passwordsCracked = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct data {
    char passwords[PASSWORD_COUNT][33];
    char** words;
    int* wordSizes;
    int wordCount;
};

int linesInFile(FILE *file)
{
    int numberOfLines = 0;
    int character;
    while(!feof(file))
    {
        character = fgetc(file);
        if(character == '\n')
        {
            numberOfLines++;
        }
    }
    fseek(file,0,SEEK_SET);
    return numberOfLines;
}

void checkPassword(char* word, char* hash, char* password)
{
    if(strcmp(hash,password)==0)
    {
        pthread_mutex_lock(&lock);
        for(int k = 0; k < strlen(word); k++)
        {
            package[0][k] = word[k];
        }
        for(int k = 0; k < strlen(password); k++)
        {
           package[1][k] = password[k];
        }
        passwordFound = 1;
    }
}

void progress(int signal)
{
    printf("Current passwords cracked: %i / %i \n", passwordsCracked, PASSWORD_COUNT);
}

void* single_word_lowercase(void* arg)
{
    struct data *Data = arg;

    char result1[100]="";
    char result2[100]="";
    char result3[100]="";
    char result3temp[100]="";

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password1[33],password2[33],password3[33];

    char prefix[52]="";
    char word[100] = "";
    int counter[52];
    memset(counter, 0, 52);

    for(int i = 0; i < Data->wordCount; i++)
        {
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word[index] = Data->words[i][index];
            }
            MD5((unsigned char*)&word,strlen(word),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }

            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(word,password1,Data->passwords[i]);
            }
            memset(word, '\0', 100);
        }

    while(1)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }
        printf("=> %s\n",prefix);
        //pobiera slowo
        for(int i = 0; i < Data->wordCount; i++)
        {    
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word[index] = Data->words[i][index];
            }
            //zbuduj potencjalne hasla
            strcpy(result1,word);
            strcat(result1,prefix);

            strcpy(result2,prefix);
            strcat(result2,word);

            strcpy(result3temp,prefix);
            strcat(result3temp,word);

            strcpy(result3,result3temp);
            strcat(result3,prefix);

            //hash
            MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }
            MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
            }
            MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
            }

            //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(result1,password1,Data->passwords[i]);
                checkPassword(result2,password2,Data->passwords[i]);
                checkPassword(result3,password3,Data->passwords[i]);
            }

            memset(result1, '\0', 100);
            memset(result2, '\0', 100);
            memset(result3, '\0', 100);
            memset(result3temp, '\0', 100);
            memset(word, '\0', 100);
        }
        memset(prefix, '\0', 100);
    }
}

void* single_word_capital(void* arg)
{
    struct data *Data = arg;

    char result1[100]="";
    char result2[100]="";
    char result3[100]="";
    char result3temp[100]="";

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password1[33],password2[33],password3[33];

    char prefix[52]="";
    char word[100] = "";
    int counter[52];
    memset(counter, 0, 52);

    for(int i = 0; i < Data->wordCount; i++)
        {
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word[index] = Data->words[i][index];
            }
            word[0] = toupper(word[0]);
            MD5((unsigned char*)&word,strlen(word),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }

            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(word,password1,Data->passwords[i]);
            }
            memset(word, '\0', 100);
        }

    while(counter[51]<30)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }

        //pobiera slowo
        for(int i = 0; i < Data->wordCount; i++)
        {    
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word[index] = Data->words[i][index];
            }
            //zamien pierwsza litere na duza
            word[0] = toupper(word[0]);
            //zbuduj potencjalne hasla
            strcpy(result1,word);
            strcat(result1,prefix);

            strcpy(result2,prefix);
            strcat(result2,word);

            strcpy(result3temp,prefix);
            strcat(result3temp,word);

            strcpy(result3,result3temp);
            strcat(result3,prefix);

            //hash
            MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }
            MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
            }
            MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
            }

            //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(result1,password1,Data->passwords[i]);
                checkPassword(result2,password2,Data->passwords[i]);
                checkPassword(result3,password3,Data->passwords[i]);
            }

            memset(result1, '\0', 100);
            memset(result2, '\0', 100);
            memset(result3, '\0', 100);
            memset(result3temp, '\0', 100);
            memset(word, '\0', 100);
        }
        memset(prefix, '\0', 100);
    }
}

void* single_word_uppercase(void* arg)
{
    struct data *Data = arg;

    char result1[100]="";
    char result2[100]="";
    char result3[100]="";
    char result3temp[100]="";

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password1[33],password2[33],password3[33];

    char prefix[52]="";
    char word[100] = "";
    int counter[52];
    memset(counter, 0, 52);

    for(int i = 0; i < Data->wordCount; i++)
        {
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word[index] = toupper(Data->words[i][index]);
            }
            MD5((unsigned char*)&word,strlen(word),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }

            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(word,password1,Data->passwords[i]);
            }
            memset(word, '\0', 100);
        }

    while(1)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }

        //pobiera slowo
        for(int i = 0; i < Data->wordCount; i++)
        {
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word[index] = toupper(Data->words[i][index]);
            }
            //zbuduj potencjalne hasla
            strcpy(result1,word);
            strcat(result1,prefix);

            strcpy(result2,prefix);
            strcat(result2,word);

            strcpy(result3temp,prefix);
            strcat(result3temp,word);

            strcpy(result3,result3temp);
            strcat(result3,prefix);

            //hash
            MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }
            MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
            }
            MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
            }

            //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(result1,password1,Data->passwords[i]);
                checkPassword(result2,password2,Data->passwords[i]);
                checkPassword(result3,password3,Data->passwords[i]);
            }

            memset(result1, '\0', 100);
            memset(result2, '\0', 100);
            memset(result3, '\0', 100);
            memset(result3temp, '\0', 100);
            memset(word, '\0', 100);
        }
        memset(prefix, '\0', 100);
    }
}

void* two_words_lowercase(void* arg)
{
    struct data *Data = arg;

    char result1[100]="", result2[100]="", result3[100]="";
    char resultTemp[100]="";

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password1[33],password2[33],password3[33];

    char prefix[52]="";
    char word1[100] = "",word2[100] = "";
    int counter[52];
    memset(counter, 0, 52);

    for(int i = 0; i < Data->wordCount; i++)
    {
        for(int index = 0; index < Data->wordSizes[i]; index++)
        {
            word1[index] = Data->words[i][index];
        }

        for(int j = 0; j < Data->wordCount; j++)
        {
            for(int index = 0; index < Data->wordSizes[j]; index++)
            {
                word2[index] = Data->words[j][index];
            }
        
            strcpy(result1,word1);
            strcat(result1,word2);
            if(i!=j)
            {
                strcpy(result2,word2);
                strcat(result2,word1);
            }

            MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }
            if(i!=j)
            {
                MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                }
            }
            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(result1,password1,Data->passwords[i]);
                if(i!=j)
                {
                    checkPassword(result2,password2,Data->passwords[i]);
                }
            }
            memset(word2, '\0', 100);
        }
        memset(word1, '\0', 100);
    }

    while(1)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }

        //pobiera slowo
        for(int i = 0; i < Data->wordCount; i++)
        {    
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word1[index] = Data->words[i][index];
            }

            for(int j = 0; j < Data->wordCount; j++)
            {
                for(int index = 0; index < Data->wordSizes[j]; index++)
                {
                    word2[index] = Data->words[j][index];
                }
                //zbuduj potencjalne hasla
                strcpy(resultTemp, prefix);
                strcat(resultTemp, word1);
                strcpy(result1, resultTemp);
                strcat(result1, word2);
                memset(resultTemp, '\0', 100);

                strcpy(resultTemp, word1);
                strcat(resultTemp, prefix);
                strcpy(result2, resultTemp);
                strcat(result2, word2);
                memset(resultTemp, '\0', 100);

                strcpy(resultTemp, word1);
                strcat(resultTemp, word2);
                strcpy(result2, resultTemp);
                strcat(result2, prefix);
                memset(resultTemp, '\0', 100);

                //hash
                MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
                }
                MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                }
                MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
                }

                //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
                for(int i = 0; i < PASSWORD_COUNT; i++)
                {
                    checkPassword(result1,password1,Data->passwords[i]);
                    checkPassword(result2,password2,Data->passwords[i]);
                    checkPassword(result3,password3,Data->passwords[i]);
                }

                memset(result1, '\0', 100);
                memset(result2, '\0', 100);
                memset(result3, '\0', 100);

                if(i!=j)
                    {
                        strcpy(resultTemp, prefix);
                        strcat(resultTemp, word2);
                        strcpy(result1, resultTemp);
                        strcat(result1, word1);
                        memset(resultTemp, '\0', 100);

                        strcpy(resultTemp, word2);
                        strcat(resultTemp, prefix);
                        strcpy(result2, resultTemp);
                        strcat(result2, word1);
                        memset(resultTemp, '\0', 100);

                        strcpy(resultTemp, word2);
                        strcat(resultTemp, word1);
                        strcpy(result2, resultTemp);
                        strcat(result2, prefix);
                        memset(resultTemp, '\0', 100);

                        //hash
                        MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
                        }
                        MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                        }
                        MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
                        }

                        //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
                        for(int i = 0; i < PASSWORD_COUNT; i++)
                        {
                            checkPassword(result1,password1,Data->passwords[i]);
                            checkPassword(result2,password2,Data->passwords[i]);
                            checkPassword(result3,password3,Data->passwords[i]);
                        }

                        memset(result1, '\0', 100);
                        memset(result2, '\0', 100);
                        memset(result3, '\0', 100);
                    }
                memset(word2, '\0', 100);
            }
            memset(word1, '\0', 100);
        }
        memset(prefix, '\0', 100);
    }
}

void* two_words_capital(void* arg)
{
    struct data *Data = arg;

    char result1[100]="", result2[100]="", result3[100]="";
    char resultTemp[100]="";

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password1[33],password2[33],password3[33];

    char prefix[52]="";
    char word1[100] = "",word2[100] = "";
    int counter[52];
    memset(counter, 0, 52);

    for(int i = 0; i < Data->wordCount; i++)
    {
        for(int index = 0; index < Data->wordSizes[i]; index++)
        {
            word1[index] = Data->words[i][index];
        }
        word1[0]=toupper(word1[0]);
        for(int j = 0; j < Data->wordCount; j++)
        {
            for(int index = 0; index < Data->wordSizes[j]; index++)
            {
                word2[index] = Data->words[j][index];
            }
            word2[0]=toupper(word2[0]);
            strcpy(result1,word1);
            strcat(result1,word2);
            if(i!=j)
            {
                strcpy(result2,word2);
                strcat(result2,word1);
            }

            MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }
            if(i!=j)
            {
                MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                }
            }
            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(result1,password1,Data->passwords[i]);
                if(i!=j)
                {
                    checkPassword(result2,password2,Data->passwords[i]);
                }
            }
            memset(word2, '\0', 100);
        }
        memset(word1, '\0', 100);
    }

    while(1)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }

        //pobiera slowo
        for(int i = 0; i < Data->wordCount; i++)
        {    
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word1[index] = Data->words[i][index];
            }
            word1[0]=toupper(word1[0]);
            for(int j = 0; j < Data->wordCount; j++)
            {
                for(int index = 0; index < Data->wordSizes[j]; index++)
                {
                    word2[index] = Data->words[j][index];
                }
                word2[0]=toupper(word2[0]);
                //zbuduj potencjalne hasla
                strcpy(resultTemp, prefix);
                strcat(resultTemp, word1);
                strcpy(result1, resultTemp);
                strcat(result1, word2);
                memset(resultTemp, '\0', 100);

                strcpy(resultTemp, word1);
                strcat(resultTemp, prefix);
                strcpy(result2, resultTemp);
                strcat(result2, word2);
                memset(resultTemp, '\0', 100);

                strcpy(resultTemp, word1);
                strcat(resultTemp, word2);
                strcpy(result2, resultTemp);
                strcat(result2, prefix);
                memset(resultTemp, '\0', 100);

                //hash
                MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
                }
                MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                }
                MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
                }

                //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
                for(int i = 0; i < PASSWORD_COUNT; i++)
                {
                    checkPassword(result1,password1,Data->passwords[i]);
                    checkPassword(result2,password2,Data->passwords[i]);
                    checkPassword(result3,password3,Data->passwords[i]);
                }

                memset(result1, '\0', 100);
                memset(result2, '\0', 100);
                memset(result3, '\0', 100);

                if(i!=j)
                    {
                        strcpy(resultTemp, prefix);
                        strcat(resultTemp, word2);
                        strcpy(result1, resultTemp);
                        strcat(result1, word1);
                        memset(resultTemp, '\0', 100);

                        strcpy(resultTemp, word2);
                        strcat(resultTemp, prefix);
                        strcpy(result2, resultTemp);
                        strcat(result2, word1);
                        memset(resultTemp, '\0', 100);

                        strcpy(resultTemp, word2);
                        strcat(resultTemp, word1);
                        strcpy(result2, resultTemp);
                        strcat(result2, prefix);
                        memset(resultTemp, '\0', 100);

                        //hash
                        MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
                        }
                        MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                        }
                        MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
                        }

                        //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
                        for(int i = 0; i < PASSWORD_COUNT; i++)
                        {
                            checkPassword(result1,password1,Data->passwords[i]);
                            checkPassword(result2,password2,Data->passwords[i]);
                            checkPassword(result3,password3,Data->passwords[i]);
                        }

                        memset(result1, '\0', 100);
                        memset(result2, '\0', 100);
                        memset(result3, '\0', 100);
                    }
                memset(word2, '\0', 100);
            }
            memset(word1, '\0', 100);
        }
        memset(prefix, '\0', 100);
    }
}

void* two_words_uppercase(void* arg)
{
    struct data *Data = arg;

    char result1[100]="", result2[100]="", result3[100]="";
    char resultTemp[100]="";

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password1[33],password2[33],password3[33];

    char prefix[52]="";
    char word1[100] = "",word2[100] = "";
    int counter[52];
    memset(counter, 0, 52);

    for(int i = 0; i < Data->wordCount; i++)
    {
        for(int index = 0; index < Data->wordSizes[i]; index++)
        {
            word1[index] = toupper(Data->words[i][index]);
        }
        for(int j = 0; j < Data->wordCount; j++)
        {
            for(int index = 0; index < Data->wordSizes[j]; index++)
            {
                word2[index] = toupper(Data->words[j][index]);
            }
            strcpy(result1,word1);
            strcat(result1,word2);
            if(i!=j)
            {
                strcpy(result2,word2);
                strcat(result2,word1);
            }

            MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
            for(int k = 0; k < 16; k++)
            {
                sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
            }
            if(i!=j)
            {
                MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                }
            }
            for(int i = 0; i < PASSWORD_COUNT; i++)
            {
                checkPassword(result1,password1,Data->passwords[i]);
                if(i!=j)
                {
                    checkPassword(result2,password2,Data->passwords[i]);
                }
            }
            memset(word2, '\0', 100);
        }
        memset(word1, '\0', 100);
    }

    while(1)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }

        //pobiera slowo
        for(int i = 0; i < Data->wordCount; i++)
        {    
            for(int index = 0; index < Data->wordSizes[i]; index++)
            {
                word1[index] = toupper(Data->words[i][index]);
            }
            for(int j = 0; j < Data->wordCount; j++)
            {
                for(int index = 0; index < Data->wordSizes[j]; index++)
                {
                    word2[index] = toupper(Data->words[j][index]);
                }
                //zbuduj potencjalne hasla
                strcpy(resultTemp, prefix);
                strcat(resultTemp, word1);
                strcpy(result1, resultTemp);
                strcat(result1, word2);
                memset(resultTemp, '\0', 100);

                strcpy(resultTemp, word1);
                strcat(resultTemp, prefix);
                strcpy(result2, resultTemp);
                strcat(result2, word2);
                memset(resultTemp, '\0', 100);

                strcpy(resultTemp, word1);
                strcat(resultTemp, word2);
                strcpy(result2, resultTemp);
                strcat(result2, prefix);
                memset(resultTemp, '\0', 100);

                //hash
                MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
                }
                MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                }
                MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
                for(int k = 0; k < 16; k++)
                {
                    sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
                }

                //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
                for(int i = 0; i < PASSWORD_COUNT; i++)
                {
                    checkPassword(result1,password1,Data->passwords[i]);
                    checkPassword(result2,password2,Data->passwords[i]);
                    checkPassword(result3,password3,Data->passwords[i]);
                }

                memset(result1, '\0', 100);
                memset(result2, '\0', 100);
                memset(result3, '\0', 100);

                if(i!=j)
                    {
                        strcpy(resultTemp, prefix);
                        strcat(resultTemp, word2);
                        strcpy(result1, resultTemp);
                        strcat(result1, word1);
                        memset(resultTemp, '\0', 100);

                        strcpy(resultTemp, word2);
                        strcat(resultTemp, prefix);
                        strcpy(result2, resultTemp);
                        strcat(result2, word1);
                        memset(resultTemp, '\0', 100);

                        strcpy(resultTemp, word2);
                        strcat(resultTemp, word1);
                        strcpy(result2, resultTemp);
                        strcat(result2, prefix);
                        memset(resultTemp, '\0', 100);

                        //hash
                        MD5((unsigned char*)&result1,strlen(result1),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password1[k*2], "%02x",(unsigned int)digest[k]);
                        }
                        MD5((unsigned char*)&result2,strlen(result2),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password2[k*2], "%02x",(unsigned int)digest[k]);
                        }
                        MD5((unsigned char*)&result3,strlen(result3),(unsigned char*)&digest);
                        for(int k = 0; k < 16; k++)
                        {
                            sprintf(&password3[k*2], "%02x",(unsigned int)digest[k]);
                        }

                        //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
                        for(int i = 0; i < PASSWORD_COUNT; i++)
                        {
                            checkPassword(result1,password1,Data->passwords[i]);
                            checkPassword(result2,password2,Data->passwords[i]);
                            checkPassword(result3,password3,Data->passwords[i]);
                        }

                        memset(result1, '\0', 100);
                        memset(result2, '\0', 100);
                        memset(result3, '\0', 100);
                    }
                memset(word2, '\0', 100);
            }
            memset(word1, '\0', 100);
        }
        memset(prefix, '\0', 100);
    }
}

void* special_signs(void* arg)
{
    struct data *Data = arg;

    unsigned char digest[MD5_DIGEST_LENGTH];
    char password[33];

    char prefix[52]="";
    int counter[52];
    memset(counter, 0, 52);

    while(1)
    {
        //generuje prefix
        counter[0]++;
        for(int index = 0; index < 50; index++)
        {
            if(counter[index] > 32)
            {
                counter[index]=1;
                counter[index+1]++;
            }else if(counter[index]==0)
            {
                prefix[index]= '\0';
            }
            if(counter[index]!=0)
            {
                prefix[index] = '\0'+ (32 + counter[index]);
            }
        }

        //hash
        MD5((unsigned char*)&prefix,strlen(prefix),(unsigned char*)&digest);
        for(int k = 0; k < 16; k++)
        {
            sprintf(&password[k*2], "%02x",(unsigned int)digest[k]);
        }

        //sprawdz czy te hasla wystepuja i wyslij jezeli sie zgadza
        for(int i = 0; i < PASSWORD_COUNT; i++)
        {
            checkPassword(prefix,password,Data->passwords[i]);
        }
        memset(prefix, '\0', 100);
    }
}

void* receiver(void *arg)
{
    int control = 0;
    signal(SIGHUP,progress);

    while(1)
    {
        if(passwordFound==1)
        {
            for(int i = 0; i <= passwordsCracked; i++)
            {
                if(strcmp(package[0],crackedWordList[i])==0)
                {
                    control++;
                }
            }
            if(control==0)
            {
                for(int k = 0; k < strlen(package[0]); k++)
                {
                    crackedWordList[passwordsCracked][k] = package[0][k];
                }
                for(int k = 0; k < strlen(package[1]); k++)
                {
                    crackedPasswordList[passwordsCracked][k] = package[1][k];
                }
                printf("=> %s %s\n", package[0], package[1]);
                passwordsCracked++;
            }

            memset(package[0], '\0', 100);
            memset(package[1], '\0', 100);

            passwordFound = 0;
            control = 0;
            pthread_mutex_unlock(&lock);
        }
    }
}

int main()
{

memset(package[0], '\0', 100);
memset(package[1], '\0', 100);

struct data Data;
Data.words = NULL;
int firstLoop = 1;

FILE *dictionary, *passwords;
int sourceLines;

char sourcefile[100],wordTemp[100];
char* sourceFile;
char* word;

int numberOfThreads = 8;
pthread_attr_t attr;
pthread_t threads[numberOfThreads];

pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

// zaladowac hasla
passwords = fopen("passwords.txt","r");
for(int index = 0; index < 891; index++)
{
    fscanf(passwords,"%s",wordTemp);
    for(int i = 0; i < sizeof(wordTemp)-1; i++)
        {
            Data.passwords[index][i] = wordTemp[i];
        }
}

while(1)
    {
    // 0. podaj plik i sprawdz czy istnieje
    do{
    scanf("%s",sourcefile);
    dictionary = fopen(sourcefile,"r");
    }while(dictionary == NULL);
    // 1. zabic istniejace watki
    pthread_mutex_lock(&lock);
    for(int i = 0; i < numberOfThreads; i++)
    {
        pthread_cancel(threads[i]);
    }
    pthread_mutex_unlock(&lock);
    // 2. zczyscic pamiec dynamiczna
    if(firstLoop == 0)
    {
        for(int i = 0; i < sizeof(Data.words)-1; i++)
        {
            free(Data.words[i]);
        }
        free(Data.wordSizes);
        free(Data.words);
    }
    // 3. zaladowanie danych z pliku
    sourceLines = linesInFile(dictionary);
    Data.wordCount = sourceLines;
    Data.words = calloc(sourceLines,sizeof(char*));
    if(Data.words == NULL)
    {
        printf("blad przy alokacji slownika\n");
        return 1;
    }
    Data.wordSizes = calloc(sourceLines,sizeof(int));
    if(Data.wordSizes == NULL)
    {
        printf("blad przy alokacji tablicy rozmiaru slow\n");
        return 1;
    }
    for(int index = 0; index < sourceLines; index++)
    {
        fscanf(dictionary,"%s",wordTemp);
        word = malloc(sizeof(wordTemp));
        for(int i = 0; i < sizeof(wordTemp)-1; i++)
        {
            word[i] = wordTemp[i];
        }
        Data.words[index] = word;
        Data.wordSizes[index] = strlen(word);
    }
    fclose(dictionary);
    memset(sourcefile, '\0', 100);

    // 4. uruchomienie watkow
    pthread_create(&threads[0], &attr, single_word_lowercase, (void *) &Data);
    pthread_create(&threads[1], &attr, single_word_capital, (void *) &Data);
    pthread_create(&threads[2], &attr, single_word_uppercase, (void *) &Data);
    pthread_create(&threads[3], &attr, two_words_lowercase, (void *) &Data);
    pthread_create(&threads[4], &attr, two_words_capital, (void *) &Data);
    pthread_create(&threads[5], &attr, two_words_uppercase, (void *) &Data);
    pthread_create(&threads[6], &attr, special_signs, (void *) &Data);
    pthread_create(&threads[7], NULL, receiver, NULL);
    
    if(firstLoop == 1)
        {
            firstLoop = 0;
        }
    }
    
return 0;
}