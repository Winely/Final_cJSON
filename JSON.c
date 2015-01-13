#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "JSON.h"
#include <string.h>

//extra
int DigitTest (const char* value) //test if the value is legal number
{
	int status = -1, length = 0, result = 0;
	//status:-1 => begin; 0 => int;	1 => float;	2 => exponential; 3 => end;
	int i;
	for (i = 0; value[i] != '\0'; i++)
	{
		if (value[i] != '.' && value[i] != '-' && value[i] != '+' &&value[i] != 'E' &&value[i] != 'e' && (value[i] <'0' || value[i] >'9'))
			result = -1;
		if (i == 1)				status = 0;
		if (i == length - 1)	status = 3;
		if (value[i] == '.')	//dot
		{
			if (status == -1 || status == 0)	status = 1;
			else result = -1;
		}
		else if (value[i] == 'E' || value[i] == 'e')	//E or e
		{
			if (status == 0 || status == 1)		status = 1;
			else result = -1;
		}
		else if (value[i] == '+' || value[i] == '-')	//+ or -
		{
			if (i != 0 && value[i - 1] != 'e' && value[i - 1] != 'E')
				result = -1;
		}
		else if (value[i] >= '0' && value[i] <= '9')	//number 0~9
		{
			if (value[i] == '0' && status == -1 && value[i + 1] != '.')
				result = -1;
			// no change
		}
		else	result = -1;
	}
	return result;
}

char *DigitToString(double value)
{//transform double value into standard string for printing purpose
	char *str = (char*)malloc(25*sizeof(char));
	if (value - (int)value == 0) // int
		sprintf(str, "%d", (int)value);
	else
	{//to transform numbers like "12.300000" to "12.3"
		sprintf(str, "%.10lf", value);
		int i = 0;
		while (str[i++] != '.') ;
		for (; str[i] != '\0'; i++)
		{
			while (str[i] != '0') i++;
			int ii, result = 0;
			for (ii = i; str[ii] != '\0'; ii++)
				if (str[ii] != '0') result = -1;
			if (result == 0)
			{
				str[i] = '\0'; return str;
			}
		}
	}
	return str;
}

int ArrayNumber (JSON *item)
{//return the ranking of the item in array ( or even object.)
	int num = 0;
	JSON *p;
	for (p = item->belongto->head; p != item; p = p->next) num++;
	return num;
}

char *CopyString (char *string)	//a new string copying the parameter value
{
	char *copy = (char*)malloc(strlen(string) + 1);
	int i;
	for (i = 0; string[i] != '\0'; i++)	copy[i] = string[i];
	copy[i] = '\0';
	return copy;
}

void PrePrintJSON (JSON *item)	//print JSON without tab
{
	if (item == NULL) 
	{
		printf("Error: JSON no found!\n"); return;
	}
	else
	{
		if (item->type == JSON_FALSE)		printf("false");
		else if (item->type == JSON_TRUE)	printf("true");
		else if (item->type == JSON_NULL)	printf("null");
		else if (item->type == JSON_NUMBER)
		{
			char *s = DigitToString(item->valuedouble);
			printf("%s", s);
			free(s);
		}
		else if (item->type == JSON_STRING)	printf("\"%s\"", item->valuestring);
		else if (item->type == JSON_ARRAY)
		{
			printf("[");
			JSON *p;
			for (p = item->head; p != NULL; p = p->next)
			{
				PrePrintJSON(p);
				if (p->next != NULL)
					printf(",");
			}
			printf("]");
		}
		else
		{// print object
			printf("{");
			JSON *p;
			for (p = item->head; p != NULL; p = p->next)
			{
				printf("\"%s\":", p->object_key);
				PrePrintJSON(p);
				if (p->next != NULL)
					printf(",");
			}
			printf("}");
		}
	}
}

char *GetValue (const char* value, const char end, int start, int direction)
{	//the function to copy a child string, in which the reading direction, start position and the end sign can be changed.
	//Come someone tell me how to use [sscanf] to realize it ('_>')
	int i, length = 0;
	i = start + direction;//direction == 1 => read backward; direction == -1 => read forward£»
	char a;
	while ((end != '"' && end != '/') && (a = value[i + direction*length]) != end && a != ']' && a != '}' && a != ' ' && a != ',' && a != '\n') length++;
	while ((end == '"' || end == '/') && (a = value[i + direction*length] != end))length++;
	char *getvalue;
	if (NULL == (getvalue = (char*)malloc((1+length)*sizeof(char)))) return NULL;
	if (direction == -1)
	{
		for (i = 0; i < length; i++) getvalue[i] = value[start - length + i];
	}
	else if (direction == 1)
	{
		for (i = 0; i < length; i++) getvalue[i] = value[start + 1 + i];
	}
	getvalue[i] = '\0';
	return getvalue;
}

void PrintTab (FILE *fp, int level)
{	//print '\t'.
	int i = 0;
	for (i = 0; i < level; i++)
	{
		if ((fputs("    ", fp)) == EOF) printf("Error: Failed to print\n");
	}
}

int PrintToFile(char *value, FILE *fp)
{
	if (EOF == (fputs(value, fp)))
	{
		printf("Error: Failed to print.\n");
		return 0;
	}
	else return 1;
}

void PrintFormatJSON (FILE *fp, JSON *item, int level)
{
	JSON *p1;
	if (item == NULL)
	{
		printf("Error: Item does not exist.\n");
		return;
	}
	if (item->belongto != NULL)
	{
		if (item->belongto->type == JSON_OBJECT);
		else PrintTab(fp, level);
	}
	else PrintTab(fp, level);
	if (item->type == JSON_ARRAY)
	{
		if (0 == PrintToFile("[\n", fp)) return;
		for (p1 = item->head; p1 != NULL; p1 = p1->next)
		{
			PrintFormatJSON(fp, p1, level + 1);
			if (p1->next != NULL)
			{
				if (0 == PrintToFile(",\n", fp)) return;
			}
			else
			{
				if (0 == PrintToFile("\n", fp)) return;
			}
		}
		PrintTab(fp, level);
		if (0 == PrintToFile("]", fp)) return;
	}
	else if (item->type == JSON_OBJECT)
	{
		if (0 == PrintToFile("{\n", fp)) return;
		for (p1 = item->head; p1 != NULL; p1 = p1->next)
		{
			PrintTab(fp, level + 1);
			if (0 == PrintToFile("\"", fp)) return;
			if (0 == PrintToFile(p1->object_key, fp)) return;
			if (0 == PrintToFile("\": ", fp)) return;
			PrintFormatJSON(fp, p1, level + 1);
			if (p1->next != NULL)
			{
				if (0 == PrintToFile(",\n", fp)) return;
			}
			else
			if (0 == PrintToFile("\n", fp)) return;
		}
		PrintTab(fp, level);
		if (0 == PrintToFile("}", fp)) return;
	}
	else if (item->type == JSON_TRUE)
	{
		if (0 == PrintToFile("true", fp)) return;
	}
	else if (item->type == JSON_FALSE)
	{
		if (0 == PrintToFile("false", fp)) return;
	}
	else if (item->type == JSON_NULL)
	{
		if (0 == PrintToFile("null", fp)) return;
	}
	else if (item->type == JSON_NUMBER)
	{
		if (0 == PrintToFile(DigitToString(item->valuedouble), fp)) return;
	}
	else if (item->type == JSON_STRING)
	{
		if (0 == PrintToFile("\"", fp)) return;
		if (0 == PrintToFile(item->valuestring, fp)) return;
		if (0 == PrintToFile("\"", fp)) return;
	}
}

void ReplaceItem (JSON *target, JSON *new_item)
{//replace target item with new item.
	new_item->belongto = target->belongto;
	new_item->next = target->next;
	new_item->previous = target->previous;
	new_item->object_key = target->object_key;
	if (target->previous == NULL)
		target->belongto->head = new_item;
	else
		target->previous->next = new_item;
	if (target->next == NULL)
		target->belongto->end = new_item;
	else
		target->next->previous = new_item;
}

int IsSpace(const char *value, int start)
{//to judge if value is space
	for (int i = start; value[i] != '\0'; i++)
	if (value[i] != ' ' && value[i] != '\n' && value[i] != '\t') return 0;
	return 1;
}

// Parse & Print
JSON *ParseJSON(const char*value)
{
	char *str;
	JSON *json;
	int i = 0;
	while (value[i] == ' ') i++;
	switch (value[i])
	{
	case '"':	return (CreateString(GetValue(value, '"', i, 1)));
	case 't':	if (value[i + 1] == 'r' && value[i + 2] == 'u' && value[i + 3] == 'e' && IsSpace(value, i + 4)) return(CreateTrue());
				else { printf("Error: Data illegal!\n"); return NULL; }
	case'f':	if (value[i + 1] == 'a' && value[i + 2] == 'l' && value[i + 3] == 's' && value[i + 4] == 'e' && IsSpace(value, i + 5)) return(CreateFalse());
				else{ printf("Error: Data illegal!\n"); return NULL; }
	case'[':	json = CreateArray(); break;
	case'{':	json = CreateObject(); break;
	case'+':case'-':case'0':case'1':case'2':case'3':case'4':case'5':case'6':case'7':case'8':case'9': 
		str = GetValue(value, ' ', i - 1, 1);
		if (0 == DigitTest(str)) 
		{
			json = CreateNumber(atof(str)); free(str); return json;
		}
		else{ printf("Error: Data illegal!\n"); free(str); return NULL; }
	default: printf("Error: Data illegal!\n");	return NULL;
	}
	JSON *new_item, *status = json;
	for (i++; value[i] != '\0'; i++)
	{
		if (status->type == JSON_OBJECT)		//skip to ':' and skip space, if the type is object
		{
			while (value[i] != ':'&& value[i] != '}') i++; 
			if(value[i] == ':') while (value[++i] == ' ');
		}
		if (value[i] == 't')		new_item = CreateTrue();//so in fact if you type "ture" or "tell", system will fix it into "true" XD
		else if (value[i] == 'n')	new_item = CreateNULL();//the same if you type "nuLL"
		else if (value[i] == 'f')	new_item = CreateFalse();
		else if (value[i] == '"')	new_item = CreateString(GetValue(value, '"', i, 1));
		else if (value[i] == '[')	new_item = CreateArray();
		else if (value[i] == '{')	new_item = CreateObject();
		else if (value[i] == ']' || value[i] == '}')
		{
			if (NULL == (status = status->belongto))
			{
				if (IsSpace(value, i + 1)) break;
				else { printf("Error: Data illegal!\n"); break; }
			}
			continue;
		}
		else if (value[i] == '\n' || value[i] == ' ' || value[i] == ',' || value[i] == '\n' || value[i] == '\t') 
			continue;
		else
		{
			char *a = GetValue(value, ' ', i-1, 1);
			if (0 == DigitTest(a))
			{
				new_item = CreateNumber(atof(a));
				new_item->valuestring = a;
			}
			else
			{
				free(a); 
				printf("Error: Data illegal!\n");	//illegal data
				continue;
			}
		}
		if (status->type == JSON_OBJECT)		//read object_key
		{
			while (value[i--] != ':'); 
			while (value[i] == ' ') i--;
			const char *key;
			if (value[i] == '"')				//standard key
			{
				key = GetValue(value, '"', i, -1);
			}
			else								//unstandard key
			{
				key = GetValue(value, '{', i +1, -1);
			}
			new_item->object_key = (char*)key;
			AddItemToObject(status, key, new_item);
			while (value[i] != ':') i++; 		//skip back to ':' to avoid read again
		}
		else
		{
			AddItemToArray(status, new_item);
			if (new_item->type == JSON_NUMBER)	//avoid read the same number twice.
			{
				while (value[++i] != ',' && value[i] != ']');
				i--;
			}
			else if (new_item->type == JSON_TRUE || new_item->type == JSON_FALSE)
				i = i + 4 - new_item->type;
			else if (new_item->type == JSON_NULL) i += 3;
			else if (new_item->type == JSON_STRING)
			while (value[++i] != '"'); 
		}
		if (new_item->type == JSON_ARRAY || new_item->type == JSON_OBJECT)
		{
			status = new_item;
			while (value[i] != '[' && value[i] != '{') i++;
		}
	}
	printf("JSON parsing done.\n");
	return json;
}

JSON *ParseJSONFromFile(const char *file_name)
{
	FILE *fp;
	if (NULL == (fp = fopen(file_name, "r")))
	{
		printf("Error: Cannot open file %s.\n", file_name);
		exit(0);
	}
	fseek(fp, 0, SEEK_END);
	long bytes = ftell(fp);
	char *buffer;
	if (NULL == (buffer = (char*)calloc(bytes + 1,sizeof(char))))
	{
		printf("Error: Failed to create buffer. A NULL pointer is returned.\n");
		fclose(fp);
		return NULL;
	}
	fseek(fp, 0, SEEK_SET);
	long i;
	for (i = 0; !feof(fp); i++) buffer[i] = getc(fp);
	buffer[i] = '\0';
	JSON *result = ParseJSON(buffer);
	free(buffer);
	fclose(fp);
	return result;
}

void PrintJSON(JSON *item)
{
	PrePrintJSON(item);
	printf("\n");
}

void PrintJSONToFile(JSON *item, const char *file_name)
{
	FILE *fp;
	if (NULL == (fp = fopen(file_name, "w")))
	{
		printf("Error: Cannot open file %s.\n", file_name);
		exit(0);
	}
	PrintFormatJSON(fp, item, 0);
	fputs("\n", fp);
	fclose(fp);
	printf("Success printing item to %s.\n", file_name);
}

// Create
void Initialise(JSON *item)			//Initialise pointers in JSON
{
	item->head = NULL;
	item->next = NULL;
	item->previous = NULL;
	item->end = NULL;
	item->object_key = NULL;
	item->valuestring = NULL;
	item->belongto = NULL;
}

JSON *CreateNULL()
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON)))) 
	{
		printf("Error: Failed to create.\n"); return NULL;
	}
	Initialise(p);
	p->type = JSON_NULL;
	return p;
}

JSON *CreateTrue()
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON)))) 
	{
		printf("Error: Failed to create.\n"); return NULL;
	}
	Initialise(p);
	p->type = JSON_TRUE;
	p->valueint = 1;
	return p;
}

JSON *CreateFalse()
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON))))
	{
		printf("Error: Failed to create.\n"); return NULL;
	}
	Initialise(p);
	p->type = JSON_FALSE;
	p->valueint = 0;
	return p;
}

JSON *CreateBool(int b)
{
	if (b == 0)
	{
		return CreateFalse();
	}
	else
	{
		return CreateTrue();
	}
}

JSON *CreateNumber(double num)
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON)))) 
		{ printf("Error: Failed to create.\n"); return NULL; }
	Initialise(p);
	p->type = JSON_NUMBER;
	p->valuedouble = num;
	return p;
}

JSON *CreateString(const char *string)
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON)))) 
		{ printf("Error: Failed to create.\n"); return NULL; }
	Initialise(p);
	p->type = JSON_STRING;
	p->valuestring = (char*)string;
	return p;
}

JSON *CreateArray(void)
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON)))) 
		{ printf("Error: Failed to create.\n"); return NULL; }
	Initialise(p);
	p->type = JSON_ARRAY;
	return p;
}

JSON *CreateObject(void)
{
	JSON *p;
	if (NULL == (p = (JSON*)malloc(sizeof(JSON)))) 
		{ printf("Error: Failed to create.\n"); return NULL; }
	Initialise(p);
	p->type = JSON_OBJECT;
	return p;
}

/* Append */
void AddItemToArray(JSON *array, JSON *item)
{
	if (array == NULL || item == NULL)
	{
		printf("Error: Item no found!");
		return;
	}
	item->belongto = array;
	if (array->head == NULL)
		array->head = item;
	else
	{
		item->previous = array->end;
		array->end->next = item;
	}
	array->end = item;
}

void AddItemToObject(JSON *object, const char *key, JSON *value)
{
	if (object == NULL || value == NULL)
	{
		printf("Error: Item no found!");
		return;
	}
	value->object_key = (char*)key;
	value->belongto = object;
	AddItemToArray(object, value);
}

/* Update */
void ReplaceItemInArray(JSON *array, int which, JSON *new_item)
{
	JSON *target;
	if ((target = GetItemInArray(array, which)) == NULL)return;
	ReplaceItem(target, new_item);
	target->belongto = NULL;
	DeleteJSON(target);
}

void ReplaceItemInObject(JSON *object, const char *key, JSON *new_value)
{
	JSON *target;
	if(NULL == (target = GetItemInObject(object, key))) return;
	ReplaceItem(target, new_value);
	target->belongto = NULL;
	DeleteJSON(target);
}

/* Remove/Delete */
JSON *DetachItemFromArray(JSON *array, int which)
{
	JSON *p = GetItemInArray(array, which);
	if (p == NULL) return NULL;
	if (which == 0)
	{
		if (array->end == array->head)
			Initialise(array);
		else
		{
			p->next->previous = NULL;
			array->head = p->next;
		}
	}
	else
	{
		p->previous->next = p->next;
		if (p == array->end)
			array->end = p->previous;
	}
	return p;
}

void DeleteItemFromArray(JSON *array, int which)
{
	DeleteJSON(DetachItemFromArray(array, which));
}

JSON *DetachItemFromObject(JSON *object, const char *key)
{
	JSON *p = GetItemInObject(object, key);
	if (p == NULL) return NULL;
	if (p->previous == NULL)
	{
		if (p == object->end) Initialise(object);
		else
		{
			object->head = p->next;
			p->next->previous = NULL;
		}
	}
	else
	{
		p->previous->next = p->next;
		if (p == object->end) object->end = p->previous;
	}
	return p;
}

void DeleteItemFromObject(JSON *object, const char *key)
{
	DeleteJSON(DetachItemFromObject(object, key));
}

void DeleteJSON(JSON *item)
{
	if (!item) return;
	free(item->object_key);
	switch (item->belongto->type)
	{
	case JSON_ARRAY: DetachItemFromArray(item->belongto, ArrayNumber(item)); break;
	case JSON_OBJECT:DetachItemFromObject(item->belongto, item->object_key); break;
	default:break;
	}
	switch (item->type)
	{
	case JSON_ARRAY: 
		while (!item->head) DeleteItemFromArray(item, 0); 
		free(item); break;
	case JSON_OBJECT:
		while (!item->head) DeleteItemFromObject(item, item->head->object_key);
		free(item); break;
	case JSON_STRING: 
		free(item->valuestring); free(item); break;
	default: free(item);
		break;
	}
}

/* Duplicate */
JSON *Duplicate(JSON *item, int recurse)
{
	JSON *p = NULL;
	//types that do not use pointers
	if (item->type == JSON_FALSE)			p = CreateFalse();
	else if (item->type == JSON_TRUE)		p = CreateTrue();
	else if (item->type == JSON_NULL)		p = CreateNULL();
	else if (item->type == JSON_NUMBER)		p = CreateNumber(item->valuedouble);
	else
	{//types that use pointers
		if (recurse == JSON_FALSE)
		{
			if (item->type == JSON_STRING)
			{
				p = CreateString(item->valuestring);
			}
			else if (item->type == JSON_ARRAY)
			{
				p = CreateArray();
				p->head = item->head;
				p->end = p->end;
			}
			else
			{
				p = CreateObject();
				p->head = item->head;
				p->end = item->end;
				p->object_key = item->object_key;
			}
			if ((p->belongto = item->belongto)->type == JSON_ARRAY || p->belongto->type == JSON_OBJECT)
			{//if the target is an element of array or object, copy the pointer.
				p->next = item->next;
				p->previous = item->previous;
			}
		}
		else		//recurse
		{
			if (item->type == JSON_STRING)
			{
				p = CreateString(CopyString(item->valuestring));
			}
			else if (item->type == JSON_ARRAY)
			{
				p = CreateArray();
				JSON *p1;
				for (p1 = item->head; p1 != NULL; p1 = p1->next)
				{
					AddItemToArray(p, Duplicate(p1, JSON_TRUE));
				}
			}
			else	// dupliccate object
			{
				p = CreateObject();
				JSON *p1;
				for (p1 = item->head; p1 != NULL; p1 = p1->next)
				{
					AddItemToObject(p, p1->object_key, Duplicate(p1, JSON_TRUE));
				}
			}
		}
	}
	return p;
}

/* Read */
JSON *GetItemInArray(JSON *array, int which)
{
	if (array == NULL || which < 0)
	{
		printf("Error: The destination does not exists. A NULL pointer is returned.\n");
		return NULL;
	}
	JSON *p = array->head;
	for (int i = 0; i < which; i++)
	{
		p = p->next;
		if (p == NULL)
		{
			printf("Error: The destination does not exists. A NULL pointer is returned.\n");
			return NULL;
		}
	}
	return p;
}

JSON *GetItemInObject(JSON *object, const char* key)
{
	if (object == NULL || key == NULL)
	{
		printf("Error: The destination does not exists.A NULL pointer is returned.\n");
		return NULL;
	}
	JSON *p = object->head;
	for (; p != NULL && *(p->object_key) != *(char*)key; p = p->next);
	if (p == NULL)
	{
		printf("Error: The destination does not exists. A NULL pointer is returned.\n");
		return NULL;
	}
	return p;
}

JSON *GetItemInJSON(JSON *json, const char *path)
{
	JSON *item = json;
	int i = 0;
	while (path[i] != '\0')
	{
		char *branch = GetValue(path, '/', i, 1);
		if (item->type == JSON_OBJECT)
			item = GetItemInObject(item, branch);
		else if (item->type == JSON_ARRAY)
			item = GetItemInArray(item, atoi(branch));
		if (item == NULL) return NULL;
		free(branch);
		while (path[++i] != '/' && path[i] != '\0');
	}
	return item;
}