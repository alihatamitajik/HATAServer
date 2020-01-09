#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "cJSON.h"
#include "cJSON.c"

int server_socket, client_socket;
struct sockaddr_in server, client;
/*
01. forgot your password
02. Change password
03. Bio
04. Last seen
*/
struct member {
    char name[128];
    char token[31];
    int id;
    char chnlin[50];
}memon[1000];
int lastmem = 0;

struct channel{
    char name[50];
    int mmbrids[50];
    int mmbronline;
}chnlon[20];
int lastchnl = 0;

//AuthToken Generator-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
char *randstring() {
    int length = 30;
    static int mySeed = 25011984;
    char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    size_t stringLen = strlen(string);
    char *randomString = NULL;

    srand(time(NULL) * length + ++mySeed);

    if (length < 1) {
        length = 1;
    }

    randomString = malloc(sizeof(char) * (length +1));

    if (randomString) {
        short key = 0;

        for (int n = 0;n < length;n++) {
            key = rand() % stringLen;
            randomString[n] = string[key];
        }

        randomString[length] = '\0';

        return randomString;
    }
    else {
        printf("No memory");
        exit(1);
    }
}
//Is Valid Auth Token-----------------------------------------------------------------------------------------------------------------
int IsValidAuthToken(char *tkn){
    for(int i=1;i<=lastmem;i++){
        if(!strcmp(memon[i].token,tkn))return 1;
    }
    return 0;
}
//Match Client And Authtoken----------------------------------------------------------------------------------------------------------
int FindMemmberIDbyToken(char *tkn)
{

}
//Read operation Client want----------------------------------------------------------------------------------------------------------------------------------------
int login(char buff[]);
int createaccount(char buff[]);
int readoprate()
{
    char buffer[1000];
    memset(buffer,0,sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    if(!strncmp(buffer,"login",5))login(buffer);
    else if(!strncmp(buffer,"register",8))createaccount(buffer);
    else if(!strncmp(buffer,"create channel",14))nwchnnl(buffer);
    return 0;
}
//Create Account------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int createaccount(char buff[]) {
    char username[50],password[65];
    char route[500];
    char msg[500];
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",username,password);
    sprintf(route,"./resources/members/%s.user.hata",username);
    if(access(route,F_OK)!=-1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"This UserName is Taken already!\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
    }
    else{
        fp = fopen(route,"w");
        fprintf(fp,"{\"username\":\"%s\",\"password\":\"%s\"}\n",username,password);
        sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
        fclose(fp);
        send(client_socket, msg, sizeof(msg)+1, 0);
    }
    return 0;

}
//Login---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int login(char buff[]){
    char username[50],password[65],*content,line[150],msg[500];
    char route[500];
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",username,password);
    sprintf(route,"./resources/members/%s.user.hata",username);
    //If File Exist---------------------------------------------------------
    if(access(route,F_OK)!=-1){
        fp = fopen(route,"r");
        fgets(line,sizeof line,fp);
        fclose(fp);
        cJSON *root = cJSON_Parse(line);
        content = cJSON_GetObjectItem(root,"password")->valuestring;
        if(!strcmp(content,password)){
            content = randstring();
            lastmem++;
            strcat(memon[lastmem].name,username);
            strcat(memon[lastmem].token,content);
            sprintf(msg,"{\"type\":\"Successful\",\"content\":\"%s\"}",content);
            send(client_socket, msg, sizeof(msg)+1, 0);
        }
        else {
            //Making message and send--------------------------------------------------
            sprintf(msg,"{\"type\":\"Error\",\"content\":\"Wrong Password\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
        }
    }
    //If File Doesn't Exist-----------------------------------------------------
    else {
        //Making message and send--------------------------------------------------
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"This UserName isn't Created yet!\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
    }
    return 0;
}
//Create New Channel--------------------------------------------------------------------------------------------------------
int nwchnnl(char buff[]){
    char chnlname[64],tkn[31],msg[500],route[500],temp[300],*out;
    int makerid;
    FILE *fp;

    sscanf(buff,"%*s %*s %[^','], %s",chnlname,tkn);
    sprintf(route,"./resources/channels/%s.channel.hata",chnlname);
    if(IsValidAuthToken(tkn)){

        //check if channel already exists-----------------------------------------------
        if(access(route,F_OK) != -1){
            sprintf(msg,"{\"type\":\"Error\",\"content\":\"A channel is exist with this name already...\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
            return 0;
        }


        //Making a new channel----------------------------------------------------------
        else{
            //Setting Values for Channel and it's first member
            makerid = 1;
            strcat(memon[makerid].chnlin,chnlname);
            fp = fopen(route,"w");
            lastchnl++;
            chnlon[lastchnl].mmbronline = 1;
            chnlon[lastchnl].mmbrids[chnlon[lastchnl].mmbronline-1]=makerid;
            strcat(chnlon[lastchnl].name,chnlname);
            //Creating JSON in Favor of Saving it in a file--------------------------------
            cJSON *root, *cars, *car;
            /* create root node and array */
            root = cJSON_CreateObject();
            cars = cJSON_CreateArray();

            /* add cars array to root */
            cJSON_AddItemToObject(root, "messages", cars);

            /* add 1st car to cars array */
            cJSON_AddItemToArray(cars, car = cJSON_CreateObject());
            cJSON_AddItemToObject(car, "sender", cJSON_CreateString("SERVER"));
            sprintf(temp,"%s",chnlname);
            cJSON_AddItemToObject(car, "message", cJSON_CreateString(temp));

            /* print everything */
            out = cJSON_Print(root);
            fprintf(fp,"%s",out);
            free(out);

            /* free all objects under root and root itself */
            cJSON_Delete(root);
            fclose(fp);
            sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
        }
    }
    //IF Contact enter a wrong authtoken---------------------------------------------
    else {
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        return 0;
    }

}
