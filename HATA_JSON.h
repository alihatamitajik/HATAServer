#ifndef HATA_JSON_H_INCLUDED
#define HATA_JSON_H_INCLUDED

#include <stddef.h>

/* JSON Types: */
#define JSON_NULL      0
#define JSON_String    1
#define JSON_Array     2
#define JSON_Object    3


/*
Making a linked list of JSONs and it has a child that let us add add arrays
*/
typedef struct JSON
{
    struct JSON *next;
    struct JSON *prev;

    struct JSON *child;

    int type;
    /*
    NULL      0 //that is used for the root...
    String    1
    Array     2
    Object    3
    */

    /* The item's string, if type == JSON_String*/
    char *valuestring;
    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} JSON;

/* Creating JSON Items That might be Object/Array/String */
JSON* CreateNewObjectJSON(void);
JSON* CreateNewArrayJSON(void);
JSON* CreateNewStringJSON(const char const * string);

/* Adding Item(String/Object/Array) to another Item(Object/Array) */
void  AddItemObjectJSON(JSON *object,const char *string,JSON *item);
void  AddItemArrayJSON(JSON *array, JSON *item);

/* Output of our JSON */
char *OutputJSON(JSON *rootobject);

/* Parser */
JSON *ParseJSON(const char * string);

/* Get information from Objects */
JSON *GetObjectItemJSON(JSON *root,const char *string);
JSON *GetArrayItemJSON(JSON *array,int index);
int   GetArraySizeJSON(JSON *array);

/* Deleting a JSON structure */
void DeleteJSON(JSON *input);

#endif //HATA_JSON_H_INCLUDED
