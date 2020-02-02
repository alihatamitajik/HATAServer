#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "HATA_JSON.h"
/*HATA_JSON tries to parse and create JSON strings*/

/*Duplicating the string so that it will be accessible until it'll be freed*/
static unsigned char* StrDuplicate(const unsigned char* string)
{
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL)return NULL;

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*)malloc(length);
    if (copy == NULL)return NULL;
    memcpy(copy, string, length);

    return copy;
}

/*Creating New Object and return the start pointer*/
JSON* CreateNewObjectJSON(void)
{
    JSON *item = (JSON *)malloc(sizeof(JSON));
    if(item != NULL){
        memset(item,0,sizeof(JSON));
        item->type = JSON_Object;
    }
    return item;
}

/*Creating New Array and return the start pointer*/
JSON* CreateNewArrayJSON(void)
{
    JSON *item = (JSON *)malloc(sizeof(JSON));
    if(item != NULL){
        memset(item,0,sizeof(JSON));
        item->type = JSON_Array;
    }
    return item;
}

/*Creating New String and return the start pointer*/
JSON* CreateNewStringJSON(const char const * string)
{
    JSON *item = (JSON *)malloc(sizeof(JSON));
    if(item != NULL){
        memset(item,0,sizeof(JSON));
        item->type = JSON_String;
        item->valuestring = StrDuplicate((const unsigned char*)string);
    }
    return item;
}

/*
Add item(String/Object/Array) to an array.

*/
void AddItemArrayJSON(JSON *array, JSON *item)
{
    /*temporarily call the child of the array "tmp"*/
    JSON *tmp = array->child;
    /*If The Item was empty*/
    if (!item) return;
    /*If Any Chain wasn't created*/
    if (!tmp)array->child=item;
    /*If The Chain Already Was Created : */
    else {
        /*It Reaches us to the tail of the chain*/
        while (tmp && tmp->next) tmp=tmp->next;
        /*Making the chain*/
        tmp->next = item;
        item->prev = tmp;
    }
}

/*
 Add Item(String/Object/Array) to a Object
[Objects are Arrays too, But they are Different in output syntax]
*/
void AddItemObjectJSON(JSON *object,const char *string,JSON *item)
{
    if (!item) return;
    /* Put the title in "char* string" */
    if(item->string)free(item->string);
    item->string=StrDuplicate(string);
    /* Observe the Object as an array and put the next item in it */
    AddItemArrayJSON(object,item);
}

/* Prototypes Used in output */
void OutputObjectJSON(JSON *object,char *output);
void OutputArrayJSON(JSON *array,char *output);
void OutputStringJSON(JSON *string,char *output);

/* Create an output string of our JSON Object */
char *OutputJSON(JSON *rootobject)
{
    char *output = (char *)malloc(200000);
    memset(output,0,sizeof(output));
    JSON *tmp;
    /* opening the accolade */
    strcpy(output,"{");
    if(rootobject->child == NULL){
        /* closing the accolade if the root object doesn't have any child*/
        strcat(output,"}");
        return output;
    }
    tmp = rootobject->child;
    do{
        /* Adding title */
        strcat(output,"\"");
        strcat(output,tmp->string);
        strcat(output,"\":");
        switch(tmp->type)
        {
            /* Choose what kind is our JSON* */
            case 1: OutputStringJSON(tmp,output);break;
            case 2: OutputArrayJSON(tmp,output);break;
            case 3: OutputObjectJSON(tmp,output);break;
            default:break;
        }
        tmp = tmp->next;
        /* Adding a separator if there is another child */
        if(tmp != NULL)strcat(output,",");
    }while(tmp != NULL);/* Looping while there is child left */

    /* closing the accolade*/
    strcat(output,"}");
    return output;

}

void OutputArrayJSON(JSON *array,char *output)
{
    JSON *tmp;
    strcat(output,"[");
    /* opening the [ */
    if(array->child == NULL){
        strcat(output,"]");
        return;
    }
    tmp = array->child;
    do{
        switch(tmp->type)
        {
            /* Choose what kind is our JSON* */
            case 1: OutputStringJSON(tmp,output);break;
            case 2: OutputArrayJSON(tmp,output);break;
            case 3: OutputObjectJSON(tmp,output);break;
            default:break;
        }
        tmp = tmp->next;
        /* Adding a separator if there is another child */
        if(tmp != NULL)strcat(output,",");
    }while(tmp != NULL);/* Looping while there is child left */
    /* closing the ] */
    strcat(output,"]");
    return;
}

void OutputStringJSON(JSON *string,char *output)
{
    char tmp[2047];
    sprintf(tmp,"\"%s\"",string->valuestring);
    strcat(output,tmp);
}

void OutputObjectJSON(JSON *object,char *output)
{
    JSON *tmp;
    /* opening the accolade */
    strcat(output,"{");
    if(object->child == NULL){
        /* closing the accolade if the root object doesn't have any child*/
        strcat(output,"}");
        return;
    }
    tmp = object->child;
    do{
        /* Adding title */
        strcat(output,"\"");
        strcat(output,tmp->string);
        strcat(output,"\":");
        switch(tmp->type)
        {
            /* Choose what kind is our JSON* */
            case 1: OutputStringJSON(tmp,output);break;
            case 2: OutputArrayJSON(tmp,output);break;
            case 3: OutputObjectJSON(tmp,output);break;
            default:break;
        }
        tmp = tmp->next;
        /* Adding a separator if there is another child */
        if(tmp != NULL)strcat(output,",");
    }while(tmp != NULL);/* Looping while there is child left */
    /* closing the accolade*/
    strcat(output,"}");
    return;
}
/* Get information from Objects */

/* Search between all children and find the child that has same string if not found it returns NULL */
JSON *GetObjectItemJSON(JSON *root,const char *string)
{
    JSON *tmp = root->child;
    /* While there exist children*/
    while(tmp != NULL){
        /* if both strings are the-same it returns the item*/
        if(!strcmp(tmp->string,string))return tmp;
        /* if it wasn't go to next child*/
        tmp = tmp->next;
    }
    /* If nothing found it returns NULL*/
    return NULL;
}

/* Go through the array and returns the index's item */
JSON *GetArrayItemJSON(JSON *array,int index)
{
    /* Set a counter that alert us when we reach the wanted index*/
    int counter = 0;
    /* Going trough the array*/
    JSON *tmp = array->child;
    /* While there are other children */
    while( tmp != NULL ){
        /* It checks if it reaches the index or not if it was it returns the item */
        if(counter==index)return tmp;
        /* Else it go trough the next child */
        tmp = tmp->next;
        /* And add on counter*/
        counter++;
    }
    /* If there isn't any child with this index it returns NULL*/
    return NULL;
}

/* It Finds the array size */
int   GetArraySizeJSON(JSON *array)
{
    /* Set a counter that alert us when we reach the wanted index*/
    int counter = 0;
    /* Going trough the array*/
    JSON *tmp = array->child;
    /* While there are other children */
    while( tmp != NULL ){
        /*  it go trough the next child */
        tmp = tmp->next;
        /* And add on counter */
        counter++;
    }
    /* Returns size of array */
    return counter;
}
/* Parser *****************************************************/
/* Parser *****************************************************/
/* Prototypes needed */
void ParseStringJSON(JSON *toaddon,const char *jstr,const char *string,int *startpoint,int option);
void ParseObjectJSON(JSON *toaddon,const char *jstr,const char *string,int *startpoint,int option);
void  ParseArrayJSON(JSON *toaddon,const char *jstr,const char *string,int *startpoint,int option);
/*
options :
0. Add to Object
1. Add to Array
*/


JSON *ParseJSON(const char * string)
{
    JSON *root = CreateNewObjectJSON();
    int counter = 0,tmpcnt=0;
    char *tmpstr;

    while(1)
    {
        tmpstr = (char *)malloc(2000);
        /* go through the string and find the first " */
        while(string[counter]!='\"'&&string[counter])counter++;
        if(string[counter]=='\0')return root;
        counter++;
        tmpcnt = 0;
        /* copy characters until reaching next " */
        while(string[counter]!='\"'){
            tmpstr[tmpcnt] = string[counter];
            counter++;
            tmpcnt++;
        }
        tmpstr[tmpcnt]='\0';
        /* tmpstr is now our title*/
        tmpcnt = 0;
        while(string[counter]!=':')counter++;
        counter++;
        /* Find out what is the next subtitle */
        switch(string[counter])
        {
            case '\"' : ParseStringJSON(root,string,tmpstr,&counter,0);break;
            case  '{' : ParseObjectJSON(root,string,tmpstr,&counter,0);break;
            case  '[' : ParseArrayJSON(root,string,tmpstr,&counter,0);break;
            default : break;
        }
        free(tmpstr);
    }
    return root;
}

void ParseStringJSON(JSON *toaddon,const char *jstr,const char *string,int *startpoint,int option)
{
    char *tmpstr = (char *)malloc(2000);
    int tmpcnt = 0;
    (*startpoint)++;
    /* Finding the subtitle */
    while(jstr[*startpoint]!='\"'){
        tmpstr[tmpcnt] = jstr[*startpoint];
        (*startpoint)++;
        tmpcnt++;
    }
    tmpstr[tmpcnt]='\0';
    /* tmpstr is our subtitle */
    JSON *item = CreateNewStringJSON(tmpstr);
    /* Add new object to the base */
    if(option==0)AddItemObjectJSON(toaddon,string,item);
    else if(option==1)AddItemArrayJSON(toaddon,item);
    free(tmpstr);
    (*startpoint)++;
}

void ParseObjectJSON(JSON *toaddon,const char *jstr,const char *string,int *startpoint,int option)
{
    JSON *beadd;
    int tmpcnt=0;
    char *tmpstr;
    int flag = 0;
    /* Create a new object to be added */
        beadd = CreateNewObjectJSON();

    while(1)
    {

        tmpstr = (char *)malloc(2000);
        /* go through the string and find the first " (Break if it reaches })*/
        while(jstr[*startpoint]!='\"')
        {
            if(jstr[*startpoint]=='}'){flag=1;break;}
            (*startpoint)++;
            if(jstr[*startpoint]=='}'){flag=1;break;}

        }
        /* Breaking if it reaches } */
        if(flag)break;
        (*startpoint)++;
        tmpcnt = 0;
        /* copy characters until reaching next " */
        while(jstr[*startpoint]!='\"'){
            tmpstr[tmpcnt] = jstr[*startpoint];
            (*startpoint)++;
            tmpcnt++;
        }
        tmpstr[tmpcnt]='\0';
        /* tmpstr is now our title*/
        tmpcnt = 0;
        while(jstr[*startpoint]!=':')(*startpoint)++;
        (*startpoint)++;
        /* Find out what is the next subtitle */
        switch(jstr[*startpoint])
        {
            case '\"' : ParseStringJSON(beadd,jstr,tmpstr,startpoint,0);break;
            default : break;
        }

        free(tmpstr);
    }
    /* Add new object to the base */
    if(option==0)AddItemObjectJSON(toaddon,string,beadd);
    else if(option==1)AddItemArrayJSON(toaddon,beadd);
}

void ParseArrayJSON(JSON *toaddon,const char *jstr,const char *string,int *startpoint,int option)
{
    JSON *beadd;
    int tmpcnt=0;
    char *tmpstr;
    beadd = CreateNewArrayJSON();
    (*startpoint)++;
    while(1)
    {
        /* Find out what is each element */
             if(jstr[*startpoint]=='\"')ParseStringJSON(beadd,jstr,"",startpoint,1);
        else if(jstr[*startpoint]== '{')ParseObjectJSON(beadd,jstr,"",startpoint,1);
        else if(jstr[*startpoint]== '[')ParseArrayJSON(beadd,jstr,"",startpoint,1);
        else if(jstr[*startpoint]== ','){
            (*startpoint)++;
            continue;
        }
        else if(jstr[(*startpoint)--]== ']')break;
        else if(jstr[*startpoint]== ']')break;
        (*startpoint)++;
    }
    /* Put The Complete array in the root */
         if(option==0)AddItemObjectJSON(toaddon,string,beadd);
    else if(option==1)AddItemArrayJSON(toaddon,beadd);
    return;
}

/* Delete a JSON structure */
void DeleteJSON(JSON *input)
{
    JSON *next;
	while (input != NULL)
	{
		next = input->next;
		/* If the input has a child child should be removed first */
		if ((input->type==2||input->type==3) && (input->child!=NULL)) DeleteJSON(input->child);
		/* If input has the ValueString it should be freed */
		if (input->valuestring != NULL) free(input->valuestring);
		/* If input has the string it should be freed */
		if (input->string != NULL) free(input->string);
		/* at last free the memory of the input */
		free(input);
		/* go for the next child */
		input=next;
	}
	return;
}

