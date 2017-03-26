#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
//#include <arpa/inet.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
#include "gm2_structs.h"

#if 0
void gm2s_test() {
    int size = 128;
    delim_str* ds1 = NULL;
    char* str = "I wish I were a jellyfish that cannot fall down stairs.";;
    char* name = "jellyfish_string";

#if 0
    //delim_str testing stuff
    ds_print(ds1);
    ds_resize(&ds1, size);
    ds_print(ds1);
    memcpy(ds1->str, str, strlen(str));
    ds_print(ds1);
    ds_free(&ds1);
    printf("ds1 has been free'd\n");
    ds_print(ds1);
#endif

#if 0
    //named_data testing stuff (print)
    named_data* nd1 = NULL;
    nd_print(nd1);
    nd_resize_name(&nd1, 128);
    nd_print(nd1);
    memcpy(nd1->name, str, strlen(str));
    nd_print(nd1);
    nd_resize_data(&nd1, 128);
    nd_print(nd1);
    memcpy(nd1->data, name, strlen(name));
    nd_print(nd1);
    nd_free(&nd1);
    printf("nd1 is free'd\n");
    nd_print(nd1);
#endif

#if 0
    //test named data list stuff
    int count = 1024;
    named_data* nda[count];
    named_data* list = NULL;
    for (int i = 0; i < count; i++) {
        nda[i] = NULL;
        nd_resize_data(&nda[i], strlen(str) + 1);
        memcpy(nda[i]->data, str, strlen(str));
        nd_resize_name(&nda[i], strlen(name) + 1);
        memcpy(nda[i]->name, name, strlen(name));
        nda[i]->order = (int)(((double)rand() / RAND_MAX)*100.0);
        nd_add_sorted(&list, &nda[i]);
        //nd_print_all(list);
    }

    nd_print_all(list);
    nd_free_all(&list);

#endif

#if 0
    //test subtitle stuff (printing, list op)
    int count = 110;
    subtitle* sa[count];
    subtitle* list = NULL;
    for (int i = 0; i < count; i++) {
        sa[i] = NULL;
        sub_resize(&sa[i], 128);
        memcpy(sa[i]->txt->str, str, strlen(str));
        sa[i]->txt_color = 0x11223344;
        sa[i]->bord_color = 0x66778899;
        sa[i]->start_frame = (int)(((double)rand() / RAND_MAX)*100.0);
        sa[i]->end_frame = 123;
        sa[i]->x = 40;
        sa[i]->y = 80;
        sub_add_sorted(&list, &sa[i]);
        sub_print_all(list);
    }

    sub_free_all(&list);
#endif
}

#endif

/*//taken from http://www.bsdforums.org/forums/showthread.php?p=94810
void print_hex(char *packet, unsigned short len)
{
int i, s_cnt;
unsigned short *p, w;
p = (unsigned short *)packet;
s_cnt = len / sizeof(unsigned short);
for (i = 0; --s_cnt >= 0; ++i, ++p)
{
if ((!(i % 2)) && i != 0)
printf(" ");
w = ntohs(*p);
printf("%02X ", (w>>8)&0xff);
printf("%02X ", w&0xff);
}
if (len & 1)
printf("%02X ", *(u_char *)p);
}*/

//data structure manipulation functions
//delim_str functions
int ds_resize(delim_str** ds, int len) {
    //printf("ds_resize called with len %d\n",len);
    if (*ds == NULL) {
        //printf("callocing null delim_str\n");
        *ds = (delim_str*)calloc(sizeof(delim_str), 1);
    }
    (*ds)->str = (char*)realloc((*ds)->str, len);
    (*ds)->len = len;
    if ((*ds)->str == NULL)
        (*ds)->len = 0;
    //printf("ds_resize resized a string to length %d\n",len);
    return (*ds)->len;
}

int ds_set(delim_str** ds, const char* str, int len) {
    if (ds_resize(ds, len) != len) {
        ds_free(ds);
        return 0;
    }
    memcpy((*ds)->str, str, len);
    return len;
}

void ds_print(delim_str* ds) {
    if (ds == NULL) {
        printf("delim_str is NULL\n");
    }
    else if (ds->len == 0 && ds->str == 0) {
        printf("delim_str is uninitialized\n");
    }
    else if (ds->len > 0 && ds->str == 0) {
        printf("delim_str's len is %d, string is null\n", ds->len);
    }
    else {
        char* tmp = (char*)malloc(ds->len + 1);
        memcpy(tmp, ds->str, ds->len);
        tmp[ds->len] = 0;
        printf("delim_str's len is %d, string is '%s'\n", ds->len, tmp);
        free(tmp);
    }
}

void ds_free(delim_str** ds) {
    if (*ds == NULL) {
        //printf("attempt to free null ds\n");
        return;
    }
    if ((*ds)->len > 0) {
        //printf("freeing ds->str string of len %d\n",(*ds)->len);
        free((*ds)->str);
    }
    //printf("freeing ds string of len %d\n",(*ds)->len);
    free(*ds);
}

/*//named_data functions
int nd_resize_name(named_data** nd, int len) {
if ((*nd) == NULL)
(*nd) = (named_data*)calloc(sizeof(named_data),1);
if ((*nd)->nlen > 0)
free((*nd)->name);
(*nd)->name = (char*)realloc((*nd)->name,len);
(*nd)->nlen = len;
if ((*nd)->name == NULL)
(*nd)->nlen = 0;
return (*nd)->nlen;
}

int nd_resize_data(named_data** nd, int len) {
if ((*nd) == NULL)
(*nd) = (named_data*)calloc(sizeof(named_data),1);
if ((*nd)->dlen > 0)
free((*nd)->data);
(*nd)->data = (char*)realloc((*nd)->data,len);
(*nd)->dlen = len;
if ((*nd)->data == NULL)
(*nd)->dlen = 0;
return (*nd)->dlen;
}

void nd_add_sorted(named_data** list, named_data** elem) {
if ((*list) == NULL) {
*list = *elem;
} else if ((*list) != (*elem)) {
//if element goes before at beginning of list
if ( (*list)->order > (*elem)->order) {
(*elem)->next = (*list);
*list = *elem;
} else {
named_data *prev = (*list), *curr = (*list);
while (curr->next != NULL && curr->order < (*elem)->order) {
prev = curr;
curr = curr->next;
}
//if element goes before curr
if (curr->order > (*elem)->order) {
(*elem)->next = curr;
prev->next = (*elem);
//if element goes after curr
} else {
(*elem)->next = curr->next;
curr->next = (*elem);
}
}
}
}

void nd_print(named_data* nd) {
if (nd == NULL) {
printf("named_data is NULL\n");
} else if (nd->dlen == 0 && nd->nlen == 0 && nd->data == NULL && nd->name == NULL) {
printf("named_data is uninitialized\n");
} else {
if (nd->nlen == 0 && nd->name == NULL ) {
printf("named_data->name is uninitialized\n");
} else {
char* tmp_n = (char*)malloc(nd->nlen+1);
memcpy(tmp_n,nd->name,nd->nlen);
tmp_n[nd->nlen] = 0;
printf("named_data's nlen is %d, name is '%s'\n",nd->nlen,nd->name);
free(tmp_n);
}

if (nd->dlen == 0 && nd->data == NULL ) {
printf("named_data->data is uninitialized\n");
} else {
char* tmp_d = (char*)malloc(nd->dlen+1);
memcpy(tmp_d,nd->data,nd->dlen);
tmp_d[nd->dlen] = 0;
printf("named_data's dlen is %d, data is '",nd->dlen);
print_hex(tmp_d,nd->dlen);
printf("'\n");
free(tmp_d);
}
printf("named_data's order number is %d\n",nd->order);
}
}

void nd_print_all(named_data* list) {
printf("printing named_data list: ");
if (list == NULL)
printf("(none)\n");
while(list != NULL) {
//nd_print(list);
printf("%d,",list->order);
list = list->next;
}
printf("\n");
}

void nd_free(named_data** nd) {
if ((*nd) == NULL)
return;
if ((*nd)->dlen > 0)
free((*nd)->data);
if ((*nd)->nlen > 0)
free((*nd)->name);
free((*nd));
*nd = NULL;
}

void nd_free_all(named_data** nd) {
if ((*nd) == NULL)
return;
nd_free_all(&(*nd)->next);
nd_free(nd);
}

//subtitle functions
int sub_resize(subtitle** sub, int len) {
if ((*sub) == NULL ) {
//printf("resizing null subtitle\n");
(*sub) = (subtitle*)calloc(sizeof(subtitle),1);
}
return ds_resize(&((*sub)->txt),len);
}

void sub_add_sorted(subtitle** list, subtitle** elem) {
if ((*list) == NULL) {
*list = *elem;
} else if ((*list) != (*elem)) {
//if element goes before at beginning of list
if ( (*list)->start_frame > (*elem)->start_frame) {
(*elem)->next = (*list);
*list = *elem;
} else {
subtitle *prev = (*list), *curr = (*list);
while (curr->next != NULL && curr->start_frame < (*elem)->start_frame) {
prev = curr;
curr = curr->next;
}
//if element goes before curr
if (curr->start_frame > (*elem)->start_frame) {
(*elem)->next = curr;
prev->next = (*elem);
//if element goes after curr
} else {
(*elem)->next = curr->next;
curr->next = (*elem);
}
}
}
}

void sub_print(subtitle* sub) {
if (sub == NULL) {
printf("subtitle is NULL\n");
} else if (sub->txt_color == sub->bord_color == sub->start_frame == sub->end_frame) {
printf("subtitle is uninitialized\n");
} else {
printf("subtitle info:\nstring/border colors: %.8X/%.8X\n",
sub->txt_color,sub->bord_color);
printf("next is %0X\n",(int)sub->next);
printf("x,y is %hd,%hd\n",sub->x,sub->y);
printf("string is...\n");
ds_print(sub->txt);
}
}

void sub_print_all(subtitle* list) {
printf("subtitle start_frame values: ");
if (list == NULL)
printf("(none)\n");
while(list != NULL) {
printf("%d,",list->start_frame);
list = list->next;
}
printf("\n");
}

void sub_free(subtitle** sub) {
if ((*sub) == NULL)
return;
ds_free(&((*sub)->txt));
free(*sub);
}

void sub_free_all(subtitle** sub) {
if ((*sub) == NULL)
return;
sub_free_all(&(*sub)->next);
sub_free(sub);
}

//console_event function
void ce_add_sorted(console_event** list, console_event** elem) {
if ((*list) == NULL) {
*list = *elem;
} else if ((*list) != (*elem)) {
//if element goes before at beginning of list
if ( (*list)->frame > (*elem)->frame) {
(*elem)->next = (*list);
*list = *elem;
} else {
console_event *prev = (*list), *curr = (*list);
while (curr->next != NULL && curr->frame < (*elem)->frame) {
prev = curr;
curr = curr->next;
}
//if element goes before curr
if (curr->frame > (*elem)->frame) {
(*elem)->next = curr;
prev->next = (*elem);
//if element goes after curr
} else {
(*elem)->next = curr->next;
curr->next = (*elem);
}
}
}
}

void ce_print_all(console_event* list) {
printf("printing console event list\n");
if (list == NULL)
printf("(none)\n");
while(list != NULL) {
printf("frame is %d, type is %c\n",list->frame, list->event+'0');
list = list->next;
}
}

void ce_free_all(console_event** list) {
console_event* tmp_ce;
while(*list != NULL) {
tmp_ce = *list;
*list = (*list)->next;
free(tmp_ce);
}
*list = NULL;
}

//gm2 functions
int gm2_init(gm2** g) {
if (*g != NULL)
gm2_free(*g);
//calloc space
(*g) = (gm2*)calloc(sizeof(gm2),1);
//XXX: figure out what else needs to go here
if ((*g) == NULL)
return 0;
return 1;
}

void gm2_print(gm2* g) {
if (g == NULL) {
printf("gm2 struct is NULL\n");
return;
}
if (g->file == NULL) {
printf("file is NULL\n");
} else {
struct stat gstat;
fstat(fileno(g->file),&gstat);
printf("file name is '%s', size is %d\n",g->fname->str,(int)gstat.st_size);
}

printf("ready is %c\n",g->ready+'0');
printf("uid is %d\n",g->uid);
printf("rerec_count = %d, rerec_frame = %d, first_frame = %d, last_frame = %d\n",
g->rerec_count,g->rerec_frames,g->first_frame,g->last_frame);
printf("auth_name: ");
ds_print(g->auth_name);
printf("auth_comment: ");
ds_print(g->auth_comment);

printf("bookmarks:\n");
nd_print_all(g->bookmarks);

printf("subtitles:\n");
sub_print_all(g->subs);

printf("console_events:\n");
ce_print_all(g->ce);

if (g->start_from == 1) {
printf("start_from == power on\n");
} else if (g->start_from == 2) {
printf("start_from == sram\n");
printf("sram is %dB long\n",g->emb_start->len);
} else if (g->start_from == 3) {
printf("start_from == saved state\n");
printf("saved state is %dB long\n",g->emb_start->len);
}

printf("frame_count = %d, frame_len = %d, g->frame_data->len = %d\n",
g->frame_count, g->frame_len, g->frame_data->len);

printf("byte_map is {%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c}\n",
g->byte_map[0]+'0',  g->byte_map[1]+'0',  g->byte_map[2]+'0',
g->byte_map[3]+'0',  g->byte_map[4]+'0',  g->byte_map[5]+'0',
g->byte_map[6]+'0',  g->byte_map[7]+'0',  g->byte_map[8]+'0',
g->byte_map[9]+'0',  g->byte_map[10]+'0', g->byte_map[11]+'0',
g->byte_map[12]+'0', g->byte_map[13]+'0', g->byte_map[14]+'0',
g->byte_map[15]+'0');
//print frame_data
}

void gm2_free(gm2* g) {
if (g == NULL)
return;
if (g->file != NULL)
fclose(g->file);
ds_free(&g->emb_start);
ds_free(&g->auth_name);
ds_free(&g->auth_comment);
ds_free(&g->frame_data);
ds_free(&g->fname);
nd_free_all(&g->bookmarks);
sub_free_all(&g->subs);
ce_free_all(&g->ce);
if (g->byte_map != NULL)
free(g->byte_map);
if (g->controller_types != NULL)
free(g->controller_types);
}

*/