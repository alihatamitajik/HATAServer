#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include "cJSON.h"
#include "cJSON.c"

#define INT_MAX 100000

int server_socket, client_socket;
struct sockaddr_in server, client;
/*
01. forgot your password
02. Change password
03. Bio
04. Last seen
05. Private chat
06. group
*/

/*
Bugs :
1. two login together : Login have to use FindMemberByName!
2. there is a big problem in refreshing
*/
struct member {
    char name[128];
    char token[31];
    char chnlin[50];
    int lastmsgrcvd;
}memon[1000];
int lastmem = -1;

struct channel{
    char name[50];
    int mmbrids[50];
    int mmbronline;
}chnlon[50];
int lastchnl = -1;
//Register All Channel----------------------------------------------------------------------------------------------------------------------------------------------
void RGCH(){
    DIR *FD;
    struct dirent* in_file;
    FILE *fp;
    cJSON *root;
    char *chname,line[INT_MAX],route[150];

    if (NULL == (FD = opendir("./resources/channels")))
    {
        printf("Error : Failed to open input directory - %s\n", strerror(errno));
        return 1;
    }
    while(in_file = readdir(FD)){
        //Ignore Parent and child directories---
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))
            continue;
        //--------------------------------------
        sprintf(route,"./resources/channels/%s",in_file->d_name);
        fp = fopen(route, "r");
        if(fp == NULL){
             printf("Error : Failed to open input directory - %s\n", in_file->d_name);
        }
        else{
            fgets(line,sizeof line,fp);
            root = cJSON_Parse(line);
            chname = cJSON_GetObjectItem(root,"chname")->valuestring;
            lastchnl++;
            strcpy(chnlon[lastchnl].name,chname);
            chnlon[lastchnl].mmbronline=-1;
            chnlon[lastchnl].mmbrids[0]=-1;
            printf("Channel \"%s\" Successfully registered.\n",chname);
        }
    }
    free(chname);
    cJSON_Delete(root);

}
//Register All Members---------------------------------------------------------------------------------------------------------------------------------------------
void RGMM(){
    DIR *FD;
    struct dirent* in_file;
    FILE *fp;
    cJSON *root;
    char *mmname,line[INT_MAX],route[150];
    if (NULL == (FD = opendir("./resources/members")))
    {
        printf("Error : Failed to open input directory - %s\n", strerror(errno));
        return 1;
    }
    while(in_file = readdir(FD)){
        //Ignore Parent and child directories---
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))
            continue;
        //--------------------------------------
        sprintf(route,"./resources/members/%s",in_file->d_name);
        fp = fopen(route, "r");
        if(fp == NULL){
             printf("Error : Failed to open input directory - %s\n", in_file->d_name);
        }
        else{
            fgets(line,sizeof line,fp);
            root = cJSON_Parse(line);
            mmname = cJSON_GetObjectItem(root,"username")->valuestring;
            lastmem++;
            strcpy(memon[lastmem].name,mmname);
            strcpy(memon[lastmem].token,"\0");
            strcpy(memon[lastmem].chnlin,"\0");
            memon[lastmem].lastmsgrcvd= -1;
            printf("User \"%s\" Successfully registered.\n",mmname);
        }
    }
    cJSON_Delete(root);
}
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
    for(int i=0;i<=lastmem;i++){
        if(!strcmp(memon[i].token,tkn))return 1;
    }
    return 0;
}
//Match Client And Authtoken----------------------------------------------------------------------------------------------------------
int FindMemmberIDbyToken(char *tkn)
{
    for(int i=0;i<=lastmem;i++){
        if(!strcmp(memon[i].token,tkn)){
            return i;
        }
    }
    return -1;
}
//Find Member by UserName---------------------------------------------------------------------------------------------------
int FindMemberIDByUsername(char username[])
{
    for(int i=0;i<=lastmem;i++){
        if(!strcmp(memon[i].name,username)){
            return i;
        }
    }
    return -1;
}
//Find Channel By ID------------------------------------------------------------------------------------------------------------------
int FindChannelByName(char chnm[])
{
    for(int i=0;i<=lastchnl;i++){
        if(!strcmp(chnlon[i].name,chnm))return i;
    }
    return -1;
}
//Read operation Client want----------------------------------------------------------------------------------------------------------------------------------------
int login(char buff[]);
int createaccount(char buff[]);
int nwchnnl(char buff[]);
int JoinCh(char buff[]);
int SendMsg(char buff[]);
int refresh(char buff[]);
int chnlmmbr(char buff[]);
int lvchnl(char buff[]);

int readoprate()
{
    char buffer[1000],msg[150];
    memset(buffer,0,sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
         if(!strncmp(buffer,"login",5))login(buffer);
    else if(!strncmp(buffer,"register",8))createaccount(buffer);
    else if(!strncmp(buffer,"create channel",14))nwchnnl(buffer);
    else if(!strncmp(buffer,"join channel",12))JoinCh(buffer);
    else if(!strncmp(buffer,"send",4))SendMsg(buffer);
    else if(!strncmp(buffer,"refresh",7))refresh(buffer);
    else if(!strncmp(buffer,"channel members",15))chnlmmbr(buffer);
    else if(!strncmp(buffer,"leave",5))lvchnl(buffer);
    else   {
        sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
    }
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
        lastmem++;
        strcpy(memon[lastmem].name,username);
        strcpy(memon[lastmem].token,"\0");
        strcpy(memon[lastmem].chnlin,"\0");
        memon[lastmem].lastmsgrcvd= -1;
        printf("User \"%s\" Successfully registered.\n",username);
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
    int mmid;
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",username,password);
    sprintf(route,"./resources/members/%s.user.hata",username);
    mmid = FindMemberIDByUsername(username);
    //If Member Exist---------------------------------------------------------
    if(mmid != -1){
        fp = fopen(route,"r");
        fgets(line,sizeof line,fp);
        fclose(fp);
        cJSON *root = cJSON_Parse(line);
        content = cJSON_GetObjectItem(root,"password")->valuestring;
        if(!strcmp(content,password)){
            content = randstring();
            strcat(memon[mmid].token,content);
            sprintf(msg,"{\"type\":\"Successful\",\"content\":\"%s\"}",content);
            send(client_socket, msg, sizeof(msg)+1, 0);
        }
        else {
            //Making message and send--------------------------------------------------
            sprintf(msg,"{\"type\":\"Error\",\"content\":\"Wrong Password\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
        }
    }
    //If Member Doesn't Exist-----------------------------------------------------
    else if(mmid == -1) {
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
            makerid = FindMemmberIDbyToken(tkn);
            strcpy(memon[makerid].chnlin,chnlname);
            fp = fopen(route,"w");
            lastchnl++;
            chnlon[lastchnl].mmbronline = 0;
            chnlon[lastchnl].mmbrids[chnlon[lastchnl].mmbronline]=makerid;
            strcpy(chnlon[lastchnl].name,chnlname);
            //Creating JSON in Favor of Saving it in a file--------------------------------
            cJSON *root, *messages, *message;
            /* create root node and array */
            root = cJSON_CreateObject();
            messages = cJSON_CreateArray();

            /* add cars array to root */
            cJSON_AddItemToObject(root, "messages", messages);

            /* add 1st message to cars array */
            cJSON_AddItemToArray(messages, message = cJSON_CreateObject());
            cJSON_AddItemToObject(message, "sender", cJSON_CreateString("SERVER"));
            sprintf(temp,"%s Created this Channel",memon[makerid].name);
            cJSON_AddItemToObject(message, "message", cJSON_CreateString(temp));

            cJSON_AddItemToObject(root,"chname",cJSON_CreateString(chnlname));
            /* print everything */
            out = cJSON_PrintUnformatted(root);
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
    return 0;

}
//Join a Channel----------------------------------------------------------------------------------------------------------------------
int JoinCh(char buff[])
{
    char chnm[64],tkn[31],msg[500],route[150],line[INT_MAX];
    int chid,mmid;
    FILE *fp;

    sscanf(buff,"%*s %*s %[^','], %s",chnm,tkn);
    chid = FindChannelByName(chnm);
    mmid = FindMemmberIDbyToken(tkn);
    if(chid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"There isn't any channel named %s.\"}",chnm);
        send(client_socket, msg, sizeof(msg)+1, 0);
        return 0;
    }
    if(mmid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        return 0;
    }
    chnlon[chid].mmbrids[++(chnlon[chid].mmbronline)]=mmid;
    strcpy(memon[mmid].chnlin,chnm);

    //Reading Latest channel file
    sprintf(route,"./resources/channels/%s.channel.hata",chnlon[chid].name);
    fp = fopen(route,"r");
    fgets(line,sizeof line,fp);
    fclose(fp);
    //Adding new message to the array in channel file------------------------------
    cJSON *root = cJSON_Parse(line);
    cJSON *msg_array,*item;
    msg_array = cJSON_GetObjectItem(root,"messages");
    item = cJSON_CreateObject();
    cJSON_AddItemToObject(item, "sender", cJSON_CreateString("SERVER"));
    sprintf(msg,"%s Joined",memon[mmid].name);
    cJSON_AddItemToObject(item, "message", cJSON_CreateString(msg));
    cJSON_AddItemToArray(msg_array, item);
    //REWrite new JSON in the file--------------------------------------------------
    fp = fopen(route,"w");
    fprintf(fp,"%s",cJSON_PrintUnformatted(root));
    fclose(fp);
    cJSON_Delete(root);

    sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
    send(client_socket, msg, sizeof(msg)+1, 0);
    return 0;
}
//Send Messages-----------------------------------------------------------------------------------------------------------------------
int SendMsg(char buff[])
{
    char tkn[31],msg[500],route[300],line[INT_MAX];
    int chid,mmid;
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",msg,tkn);
    //Check if the AuthToken is a valid one----------------------------------------
    mmid = FindMemmberIDbyToken(tkn);
    if(mmid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        return 0;
    }

    //check if member is in a channel or not---------------------------------------
    chid = FindChannelByName(memon[mmid].chnlin);
    if(chid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Invalid Command.\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        return 0;
    }

    //Reading Latest channel file
    sprintf(route,"./resources/channels/%s.channel.hata",chnlon[chid].name);
    fp = fopen(route,"r");
    fgets(line,sizeof line,fp);
    fclose(fp);
    //Adding new message to the array in channel file------------------------------
    cJSON *root = cJSON_Parse(line);
    cJSON *msg_array,*item;
    msg_array = cJSON_GetObjectItem(root,"messages");
    item = cJSON_CreateObject();
    cJSON_AddItemToObject(item, "sender", cJSON_CreateString(memon[mmid].name));
    cJSON_AddItemToObject(item, "message", cJSON_CreateString(msg));
    cJSON_AddItemToArray(msg_array, item);
    //REWrite new JSON in the file--------------------------------------------------
    fp = fopen(route,"w");
    fprintf(fp,"%s",cJSON_PrintUnformatted(root));
    fclose(fp);
    free(msg);
    cJSON_Delete(root);
    sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
    send(client_socket, msg, sizeof(msg)+1, 0);
    return 0;
}
//Refresh-----------------------------------------------------------------------------------------------------------------------------
int refresh(char buff[])
{
    char tkn[31],msg[INT_MAX],route[300],line[INT_MAX];
    int mmid,chid,lmr,msgcount; /*LMR : Last Message Received*/
    FILE *fp;
    sscanf(buff,"%*s %s",tkn);
    mmid = FindMemmberIDbyToken(tkn);
    chid = FindChannelByName(memon[mmid].chnlin);
    lmr = memon[mmid].lastmsgrcvd;
    //Read Previous Messages-------------------------------------------------
    sprintf(route,"./resources/channels/%s.channel.hata",chnlon[chid].name);
    fp = fopen(route,"r");
    fgets(line,sizeof line,fp);
    fclose(fp);
    cJSON *sendroot,*sendmessages,*sendmessage;
    cJSON *root = cJSON_Parse(line);
    cJSON *messages,*message;
    messages = cJSON_GetObjectItem(root,"messages");
    msgcount = cJSON_GetArraySize(messages);

    //Constructing a JSON to be sent------------------------------------------
    sendroot = cJSON_CreateObject();
    sendmessages = cJSON_CreateArray();
    cJSON_AddItemToObject(sendroot, "type", cJSON_CreateString("List"));
    cJSON_AddItemToObject(sendroot, "content", sendmessages);

    for(int i=lmr+1;i<msgcount;i++){
        message = cJSON_GetArrayItem(messages,i);
        char *sender = cJSON_GetObjectItem(message,"sender")->valuestring;
        char *mssssg = cJSON_GetObjectItem(message,"message")->valuestring;
        cJSON_AddItemToArray(sendmessages, sendmessage = cJSON_CreateObject());
        cJSON_AddItemToObject(sendmessage, "sender", cJSON_CreateString(sender));
        cJSON_AddItemToObject(sendmessage,"content", cJSON_CreateString(mssssg));
    }
    memon[mmid].lastmsgrcvd = msgcount-1;
    sprintf(msg,"%s",cJSON_PrintUnformatted(sendroot));
    printf("%s\n",cJSON_PrintUnformatted(sendroot));
    send(client_socket, msg, sizeof(msg)+1, 0);
    cJSON_Delete(sendroot);
    cJSON_Delete(root);
    return 0;
}
//Channel Members---------------------------------------------------------------------------------------------------------------------
int chnlmmbr(char buff[])
{
     char tkn[31],msg[INT_MAX],route[300];
     sscanf(buff,"%*s %*s %s",tkn);
     int mmid,chid,mmcnt;
     mmid = FindMemmberIDbyToken(tkn);
     chid = FindChannelByName(memon[mmid].chnlin);
     mmcnt = chnlon[chid].mmbronline;

     cJSON *sendroot,*sendmessages;
     //Constructing a JSON to be sent------------------------------------------
     sendroot = cJSON_CreateObject();
     sendmessages = cJSON_CreateArray();
     cJSON_AddItemToObject(sendroot, "type", cJSON_CreateString("List"));
     cJSON_AddItemToObject(sendroot, "content", sendmessages);
     for(int i=0;i<=chnlon[chid].mmbronline;i++){
        cJSON_AddItemToArray(sendmessages, cJSON_CreateString(memon[chnlon[chid].mmbrids[i]].name));
     }
     sprintf(msg,"%s",cJSON_PrintUnformatted(sendroot));
     send(client_socket, msg, sizeof(msg)+1, 0);
     cJSON_Delete(sendroot);
     return 0;
}
//Leave Channel ----------------------------------------------------------------------------------------------------------------------
int lvchnl(char buff[])
{
    char tkn[31],msg[50],route[300],line[INT_MAX];
    sscanf(buff,"%*s %s",tkn);
    int mmid,chid;
    FILE *fp;

    mmid = FindMemmberIDbyToken(tkn);
    chid = FindChannelByName(memon[mmid].chnlin);
    int i=0;
    while(chnlon[chid].mmbrids[i] != mmid)i++;
    for(int j=i;j<chnlon[chid].mmbronline;j++)
        chnlon[chid].mmbrids[j] = chnlon[chid].mmbrids[j+1];
    chnlon[chid].mmbronline--;
    strcpy(memon[mmid].chnlin,"\0");
    memon[mmid].lastmsgrcvd = -1;

    //Editing Channel file---------------------------------------------------------
    sprintf(route,"./resources/channels/%s.channel.hata",chnlon[chid].name);
    fp = fopen(route,"r");
    fgets(line,sizeof line,fp);
    fclose(fp);

    //Adding new message to the array in channel file------------------------------
    cJSON *root = cJSON_Parse(line);
    cJSON *msg_array,*item;
    msg_array = cJSON_GetObjectItem(root,"messages");
    item = cJSON_CreateObject();
    cJSON_AddItemToObject(item, "sender", cJSON_CreateString("SERVER"));
    sprintf(msg,"%s Left",memon[mmid].name);
    cJSON_AddItemToObject(item, "message", cJSON_CreateString(msg));
    cJSON_AddItemToArray(msg_array, item);

    //REWrite new JSON in the file--------------------------------------------------
    fp = fopen(route,"w");
    fprintf(fp,"%s",cJSON_PrintUnformatted(root));
    fclose(fp);
    cJSON_Delete(root);

    sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
    send(client_socket, msg, sizeof(msg)+1, 0);
    return 0;
}
