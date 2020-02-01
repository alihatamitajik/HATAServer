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
#include <time.h>
#include "HATA_JSON.h"
#include "HATA_JSON.c"

#define INT_MAX 100000

int server_socket, client_socket;
struct sockaddr_in server, client;
/*
Updates :
01. forgot your password
02. Change password
03. Bio
04. Last seen
05. Private chat
06. group
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
    JSON *root;
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
            root = ParseJSON(line);
            chname = GetObjectItemJSON(root,"chname")->valuestring;
            lastchnl++;
            strcpy(chnlon[lastchnl].name,chname);
            chnlon[lastchnl].mmbronline=-1;
            chnlon[lastchnl].mmbrids[0]=-1;
        }
    }
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("[%4d/%02d/%02d][%02d:%02d:%02d] | All Channels Registered Successfully\n"
           ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
           ,tm.tm_hour,tm.tm_min,tm.tm_sec);
    free(chname);
    DeleteJSON(root);

}
//Register All Members---------------------------------------------------------------------------------------------------------------------------------------------
void RGMM(){
    DIR *FD;
    struct dirent* in_file;
    FILE *fp;
    JSON *root;
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
            root = ParseJSON(line);
            mmname = GetObjectItemJSON(root,"username")->valuestring;
            lastmem++;
            strcpy(memon[lastmem].name,mmname);
            strcpy(memon[lastmem].token,"\0");
            strcpy(memon[lastmem].chnlin,"\0");
            memon[lastmem].lastmsgrcvd= -1;
        }
    }
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("[%4d/%02d/%02d][%02d:%02d:%02d] | All Members Registered Successfully\n"
           ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
           ,tm.tm_hour,tm.tm_min,tm.tm_sec);
    DeleteJSON(root);
}
//AuthToken Generator-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
char *randstring() {
    int length = 30;
    static int mySeed = 25011984;
    char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-#'?!";
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
//Already Logged-in ??-------------------------------------------------------------------------------------------------------
int AlreadyLoggedIn(int mmid)
{
    return strcmp(memon[mmid].token,"\0");
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
int logout(char buff[]);
void searchMember(char buff[]);
void MessageSearch(char buff[]);


int readoprate()
{
    char buffer[1000],msg[150];
    memset(buffer,0,sizeof(buffer));
    struct tm tm;

    struct timeval timeout;
    timeout.tv_sec = 99;
    timeout.tv_usec = 0;
    int TM = setsockopt (client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout));

    recv(client_socket, buffer, sizeof(buffer), 0);
    //Server Log
    time_t t1 = time(NULL);
    tm = *localtime(&t1);
    printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Request Received\n"
            ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
            ,tm.tm_hour,tm.tm_min,tm.tm_sec
            ,client_socket);
         if(!strncmp(buffer,"login",5))login(buffer);
    else if(!strncmp(buffer,"register",8))createaccount(buffer);
    else if(!strncmp(buffer,"create channel",14))nwchnnl(buffer);
    else if(!strncmp(buffer,"join channel",12))JoinCh(buffer);
    else if(!strncmp(buffer,"send",4))SendMsg(buffer);
    else if(!strncmp(buffer,"refresh",7))refresh(buffer);
    else if(!strncmp(buffer,"channel members",15))chnlmmbr(buffer);
    else if(!strncmp(buffer,"leave",5))lvchnl(buffer);
    else if(!strncmp(buffer,"logout",6))logout(buffer);
    else if(!strncmp(buffer,"member search",13))searchMember(buffer);
    else if(!strncmp(buffer,"message search",14))MessageSearch(buffer);
    else if(TM==0){
        time_t t2 = time(NULL);
        tm = *localtime(&t2);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Read Time Out\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Read Time Out...\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
    }

    else   {
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Conflicting Command...\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
    }
    return 0;
}
//Create Account------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int createaccount(char buff[]) {
    char username[50],password[65];
    char route[500];
    char msg[500];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",username,password);
    sprintf(route,"./resources/members/%s.user.hata",username);
    if(access(route,F_OK)!=-1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"This UserName is Taken already!\"}");
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[CreateAccount] : Taken Already\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
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
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Member \"%s\" Successfully Created\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,username);
        send(client_socket, msg, sizeof(msg)+1, 0);
    }
    return 0;

}
//Login---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int login(char buff[]){
    char username[50],password[65],*content,line[150],msg[500];
    char route[500];
    int mmid;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",username,password);
    sprintf(route,"./resources/members/%s.user.hata",username);
    mmid = FindMemberIDByUsername(username);
    //If Member Exist---------------------------------------------------------
    if(mmid != -1){
        if(AlreadyLoggedIn(mmid)){
            //Making message and send--------------------------------------------------
            sprintf(msg,"{\"type\":\"Error\",\"content\":\"Already Logged In\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
            //Server Log
            printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[LogIn] : Already Logged In\n"
                    ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                    ,tm.tm_hour,tm.tm_min,tm.tm_sec
                    ,client_socket);
        }
        else{
            fp = fopen(route,"r");
            fgets(line,sizeof line,fp);
            fclose(fp);
            JSON *root = ParseJSON(line);
            content = GetObjectItemJSON(root,"password")->valuestring;
            if(!strcmp(content,password)){
                content = randstring();
                strcat(memon[mmid].token,content);
                sprintf(msg,"{\"type\":\"Successful\",\"content\":\"%s\"}",content);
                send(client_socket, msg, sizeof(msg)+1, 0);
                //Server Log
                printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Logged In | Token : %s \n"
                        ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                        ,tm.tm_hour,tm.tm_min,tm.tm_sec
                        ,client_socket
                        ,username
                        ,content);
            }
            else {
                //Making message and send--------------------------------------------------
                sprintf(msg,"{\"type\":\"Error\",\"content\":\"Wrong Password\"}");
                send(client_socket, msg, sizeof(msg)+1, 0);
                //Server Log
                printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[LogIn] : Wrong Pass\n"
                    ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                    ,tm.tm_hour,tm.tm_min,tm.tm_sec
                    ,client_socket);
            }
        }
    }
    //If Member Doesn't Exist-----------------------------------------------------
    else if(mmid == -1) {
        //Making message and send--------------------------------------------------
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"This UserName isn't Created yet!\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[LogIn] : No Such a UserName\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
    }

    return 0;
}
//Create New Channel--------------------------------------------------------------------------------------------------------
int nwchnnl(char buff[]){
    char chnlname[64],tkn[31],msg[500],route[500],temp[300],*out;
    int makerid;
    FILE *fp;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sscanf(buff,"%*s %*s %[^','], %s",chnlname,tkn);
    sprintf(route,"./resources/channels/%s.channel.hata",chnlname);
    if(IsValidAuthToken(tkn)){

        //check if channel already exists-----------------------------------------------
        if(access(route,F_OK) != -1){
            sprintf(msg,"{\"type\":\"Error\",\"content\":\"A channel is exist with this name already...\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
            //Server Log
            printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[CreateChannel] : Name Already Taken \n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
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
            JSON *root, *messages, *message;
            /* create root node and array */
            root = CreateNewObjectJSON();
            messages = CreateNewArrayJSON();

            /* add cars array to root */
            AddItemObjectJSON(root, "messages", messages);

            /* add 1st message to cars array */
            AddItemArrayJSON(messages, message = CreateNewObjectJSON());
            AddItemObjectJSON(message, "sender", CreateNewStringJSON("SERVER"));
            sprintf(temp,"%s Created this Channel",memon[makerid].name);
            AddItemObjectJSON(message, "message", CreateNewStringJSON(temp));
            AddItemObjectJSON(root,"chname",CreateNewStringJSON(chnlname));

            /* print everything */
            out = OutputJSON(root);
            fprintf(fp,"%s",out);
            free(out);
            /* free all objects under root and root itself */
            DeleteJSON(root);
            fclose(fp);
            sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
            //Server Log
            printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Channel \"%s\" Successfully Created\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,chnlname);
            send(client_socket, msg, sizeof(msg)+1, 0);
        }
    }
    //IF Contact enter a wrong authtoken---------------------------------------------
    else {
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[CreateChannel] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return 0;
    }
    return 0;

}
//Join a Channel----------------------------------------------------------------------------------------------------------------------
int JoinCh(char buff[])
{
    char chnm[64],tkn[31],msg[500],route[150],line[INT_MAX];
    int chid,mmid;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *fp;

    sscanf(buff,"%*s %*s %[^','], %s",chnm,tkn);
    chid = FindChannelByName(chnm);
    mmid = FindMemmberIDbyToken(tkn);
    if(chid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"There isn't any channel named %s.\"}",chnm);
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[JoinChannel] : No Such a \"%s\"\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,chnm);
        return 0;
    }
    if(mmid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[JoinChannel] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
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
    JSON *root = ParseJSON(line);
    JSON *msg_array,*item;
    msg_array = GetObjectItemJSON(root,"messages");
    item = CreateNewObjectJSON();
    AddItemObjectJSON(item, "sender", CreateNewStringJSON("SERVER"));
    sprintf(msg,"%s Joined",memon[mmid].name);
    AddItemObjectJSON(item, "message", CreateNewStringJSON(msg));
    AddItemArrayJSON(msg_array, item);
    //REWrite new JSON in the file--------------------------------------------------
    fp = fopen(route,"w");
    char *out = OutputJSON(root);
    fprintf(fp,"%s",out);
    fclose(fp);
    free(out);
    DeleteJSON(root);

    sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
    //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Joined \"%s\"\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name
                ,chnm);
    send(client_socket, msg, sizeof(msg)+1, 0);
    return 0;
}
//Send Messages-----------------------------------------------------------------------------------------------------------------------
int SendMsg(char buff[])
{
    char tkn[31],msg[500],route[300],line[INT_MAX];
    int chid,mmid;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *fp;
    sscanf(buff,"%*s %[^','], %s",msg,tkn);
    //Check if the AuthToken is a valid one----------------------------------------
    mmid = FindMemmberIDbyToken(tkn);
    if(mmid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[SendMessage] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return 0;
    }

    //check if member is in a channel or not---------------------------------------
    chid = FindChannelByName(memon[mmid].chnlin);
    if(chid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Invalid Command.\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[EnsMessage] : Invalid Command From \"%s\"\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name);
        return 0;
    }

    //Reading Latest channel file
    sprintf(route,"./resources/channels/%s.channel.hata",chnlon[chid].name);
    fp = fopen(route,"r");
    fgets(line,sizeof line,fp);
    fclose(fp);
    //Adding new message to the array in channel file------------------------------
    JSON *root = ParseJSON(line);
    JSON *msg_array,*item;
    msg_array = GetObjectItemJSON(root,"messages");
    item = CreateNewObjectJSON();
    AddItemObjectJSON(item, "sender", CreateNewStringJSON(memon[mmid].name));
    AddItemObjectJSON(item, "message", CreateNewStringJSON(msg));
    AddItemArrayJSON(msg_array, item);
    //REWrite new JSON in the file--------------------------------------------------
    fp = fopen(route,"w");
    char *out = OutputJSON(root);
    fprintf(fp,"%s",out);
    fclose(fp);
    free(out);
    free(msg);
    DeleteJSON(root);
    sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
    //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Send a Message in \"%s\"\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name
                ,chnlon[chid].name);
    send(client_socket, msg, sizeof(msg)+1, 0);
    return 0;
}
//Refresh-----------------------------------------------------------------------------------------------------------------------------
int refresh(char buff[])
{
    char tkn[31],msg[INT_MAX],route[300],line[INT_MAX];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
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
    JSON *sendroot,*sendmessages,*sendmessage;
    JSON *root = ParseJSON(line);
    JSON *messages,*message;
    messages = GetObjectItemJSON(root,"messages");
    msgcount = GetArraySizeJSON(messages);
    char *name = (char *)malloc(61);
    strcpy(name,memon[mmid].name);

    //Constructing a JSON to be sent------------------------------------------
    sendroot = CreateNewObjectJSON();
    sendmessages = CreateNewArrayJSON();
    AddItemObjectJSON(sendroot, "type", CreateNewStringJSON("List"));
    AddItemObjectJSON(sendroot, "content", sendmessages);

    for(int i=lmr+1;i<msgcount;i++){
        message = GetArrayItemJSON(messages,i);
        char *sender = GetObjectItemJSON(message,"sender")->valuestring;
        char *mssssg = GetObjectItemJSON(message,"message")->valuestring;
        AddItemArrayJSON(sendmessages, sendmessage = CreateNewObjectJSON());
        if(!strcmp(sender,name))AddItemObjectJSON(sendmessage, "sender", CreateNewStringJSON("\t\t\033[0;33m You\033[0m"));
        else AddItemObjectJSON(sendmessage, "sender", CreateNewStringJSON(sender));
        AddItemObjectJSON(sendmessage,"content", CreateNewStringJSON(mssssg));
    }
    free(name);
    memon[mmid].lastmsgrcvd = msgcount-1;
    char *out = OutputJSON(sendroot);
    sprintf(msg,"%s",out);
    send(client_socket, msg, sizeof(msg)+1, 0);
    free(out);
    DeleteJSON(sendroot);
    DeleteJSON(root);
    //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Refreshed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name);
    return 0;
}
//Channel Members---------------------------------------------------------------------------------------------------------------------
int chnlmmbr(char buff[])
{
     char tkn[31],msg[INT_MAX];
     sscanf(buff,"%*s %*s %s",tkn);
     int mmid,chid,mmcnt;
     time_t t = time(NULL);
    struct tm tm = *localtime(&t);
     mmid = FindMemmberIDbyToken(tkn);
     chid = FindChannelByName(memon[mmid].chnlin);
     mmcnt = chnlon[chid].mmbronline;

     JSON *sendroot,*sendmessages;
     //Constructing a JSON to be sent------------------------------------------
     sendroot = CreateNewObjectJSON();
     sendmessages = CreateNewArrayJSON();
     AddItemObjectJSON(sendroot, "type", CreateNewStringJSON("List"));
     AddItemObjectJSON(sendroot, "content", sendmessages);
     for(int i=0;i<=chnlon[chid].mmbronline;i++){
        AddItemArrayJSON(sendmessages, CreateNewStringJSON(memon[chnlon[chid].mmbrids[i]].name));
     }
     char *out = OutputJSON(sendroot);
     sprintf(msg,"%s",out);
     send(client_socket, msg, sizeof(msg)+1, 0);
     free(out);
     DeleteJSON(sendroot);
     //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Members Sent to \"%s\" \n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,chnlon[chid].name
                ,memon[mmid].name);
     return 0;
}
//Leave Channel ----------------------------------------------------------------------------------------------------------------------
int lvchnl(char buff[])
{
    char tkn[31],msg[50],route[300],line[INT_MAX];
    sscanf(buff,"%*s %s",tkn);
    int mmid,chid;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *fp;

    mmid = FindMemmberIDbyToken(tkn);
    chid = FindChannelByName(memon[mmid].chnlin);
    if(mmid != -1){
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
        JSON *root = ParseJSON(line);
        JSON *msg_array,*item;
        msg_array = GetObjectItemJSON(root,"messages");
        item = CreateNewObjectJSON();
        AddItemObjectJSON(item, "sender", CreateNewStringJSON("SERVER"));
        sprintf(msg,"%s Left",memon[mmid].name);
        AddItemObjectJSON(item, "message", CreateNewStringJSON(msg));
        AddItemArrayJSON(msg_array, item);

        //REWrite new JSON in the file--------------------------------------------------
        fp = fopen(route,"w");
        char* out = OutputJSON(root);
        fprintf(fp,"%s",out);
        fclose(fp);
        free(out);
        DeleteJSON(root);

        sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Left \"%s\"\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name
                ,chnlon[chid].name);
        return 0;
    }
    else {
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[LeaveChannel] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return 0;
    }
    return 0;
}
//Logout--------------------------------------------------------------------------------------------------------------------
int logout(char buff[])
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char tkn[31],msg[150];
    sscanf(buff,"%*s %s",tkn);
    int mmid;
    mmid = FindMemmberIDbyToken(tkn);
    if(mmid != -1){
        strcpy(memon[mmid].token,"\0");
        sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | \"%s\" Logged Out\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name);
        return 0;
    }
    else{
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[LogOut] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return 0;
    }
    return 0;
}

//Search Member -------------------------------------------------------------------------------------------
void searchMember(char buff[])
{
    char memnm[64],tkn[31],msg[500];
    int chid,mmid,mmcnt;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sscanf(buff,"%*s %*s %[^','], %s",memnm,tkn);
    mmid = FindMemmberIDbyToken(tkn);
    //if member request not found
    if(mmid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[SearchMember] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return;
    }
    chid = FindChannelByName(memon[mmid].chnlin);
    /* if member isn't in any channel */
    if(chid == -1){
        sprintf(msg,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg, sizeof(msg)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[SearchMember] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return;
    }
    mmcnt = chnlon[chid].mmbronline;
    for(int i=0;i<=mmcnt;i++){
        if(!strcmp(memnm,memon[chnlon[chid].mmbrids[i]].name)){
            sprintf(msg,"{\"type\":\"Successful\",\"content\":\"\"}");
            send(client_socket, msg, sizeof(msg)+1, 0);
            printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Search result sent to \"%s\" \n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name);
            return;
        }
    }
    sprintf(msg,"{\"type\":\"Error\",\"content\":\"Not Found\"}");
    send(client_socket, msg, sizeof(msg)+1, 0);
    //Server Log
    printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[SearchMember] : Search Ended With No Result\n"
            ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
            ,tm.tm_hour,tm.tm_min,tm.tm_sec
            ,client_socket);
    return;
}

//Search Messages-----------------------------------------------------------------------------------------
int searcher(char str[], char substr[])
{
    int n = strlen(str);
    int m = strlen(substr);
    int count = 0;
    //Go through the main str
    for (int i = 0; i <= n - m; i++)
    {
        //Check for same str in main str
        int j;
        for (j = 0; j < m; j++)
            if (str[i+j] != substr[j])
                break;

        if (j == m)
        {
           if(str[i+j]==' '||str[i+j]=='\0')return 1;
        }
    }
    return 0;
}//Source : HTTPS://WWW.GeeksForGeeks.ORG/frequency-substring-string/

void MessageSearch(char buff[])
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char tkn[31],searchkey[30],route[150],line[INT_MAX],msg1[300];
    int mmid,chid;
    sscanf(buff,"%*s %*s %[^','], %s",searchkey,tkn);
    mmid = FindMemmberIDbyToken(tkn);
    /* if member token is not valid */
    if(mmid == -1){
        sprintf(msg1,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg1, sizeof(msg1)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[SearchMember] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return;
    }

    chid = FindChannelByName(memon[mmid].chnlin);
    /* if member isn't in any channel */
    if(chid == -1){
        sprintf(msg1,"{\"type\":\"Error\",\"content\":\"Authentication Failed\"}");
        send(client_socket, msg1, sizeof(msg1)+1, 0);
        //Server Log
        printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Error[SearchMember] : Authentication Failed\n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket);
        return;
    }
    /* Creating the out to access the file */
    sprintf(route,"./resources/channels/%s.channel.hata",chnlon[chid].name);
    FILE *fp = fopen(route,"r");
    fgets(line,sizeof line,fp);
    fclose(fp);

    /* Parse the line into JSON */
    JSON *sendroot=CreateNewObjectJSON(),*sendmessages=CreateNewArrayJSON();
    JSON *sendmessage;
    JSON *root = ParseJSON(line);
    JSON *messages,*message;
    char *msg;
    char *tmp;
    messages = GetObjectItemJSON(root,"messages");
    int msgcount = GetArraySizeJSON(messages);
    /* Looking for search key */
    for (int i=0;i<msgcount;i++){
        message = GetArrayItemJSON(messages,i);
        msg = GetObjectItemJSON(message,"message")->valuestring;
        if(searcher(msg,searchkey)){
            sendmessage = CreateNewObjectJSON();
            tmp = GetObjectItemJSON(message,"sender")->valuestring;
            AddItemObjectJSON(sendmessage,"sender",CreateNewStringJSON(tmp));
            tmp = GetObjectItemJSON(message,"message")->valuestring;
            AddItemObjectJSON(sendmessage,"message",CreateNewStringJSON(tmp));
            AddItemArrayJSON(sendmessages,sendmessage);
        }
    }
    AddItemObjectJSON(sendroot,"content",sendmessages);

    /* sending the message */
    msg = OutputJSON(sendroot);
    send(client_socket, msg, sizeof(msg)+1, 0);

    free(msg);
    free(tmp);
    DeleteJSON(root);
    DeleteJSON(sendroot);
    /* Server Log */
    printf("[%4d/%02d/%02d][%02d:%02d:%02d] | Socket :: %3d | Search result sent to \"%s\" \n"
                ,tm.tm_year+1900,tm.tm_mon +1,tm.tm_mday
                ,tm.tm_hour,tm.tm_min,tm.tm_sec
                ,client_socket
                ,memon[mmid].name);
    return;
}
