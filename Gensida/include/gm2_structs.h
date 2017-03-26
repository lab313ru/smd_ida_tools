#ifndef __GM2_STRUCTS_H
#define __GM2_STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        int len;
        char* str;
    } delim_str;

    //these will be sorted order
    typedef struct named_data {
        int dlen, nlen, order;
        struct named_data* next;
        char *data, *name;
    } named_data;

    typedef struct console_event {
        int frame;
        struct console_event* next;;
        char event;
    } console_event;

    //these will be sorted by start_frame
    typedef struct subtitle {
        int txt_color, bord_color, start_frame, end_frame;
        delim_str* txt;
        struct subtitle* next;
        short x, y;
    } subtitle;

    /*typedef struct {
        FILE* file;
        int uid, rerec_count, rerec_frames;
        int frame_count, first_frame, last_frame, frame_len, curr_frame;
        delim_str *emb_start, *auth_name, *auth_comment, *frame_data, *fname;
        named_data* bookmarks;
        subtitle* subs;
        console_event* ce;
        char *byte_map, *controller_types;
        char start_from, ready, status;
        } gm2;*/

        //delim_str functions
        //*_resize() : return resized size or 0 on failure
    int ds_resize(delim_str**, int);
    int ds_set(delim_str**, const char*, int);
    void ds_print(delim_str**);
    void ds_free(delim_str**);

    //named_data functions
    int nd_resize_name(named_data**, int);
    int nd_resize_data(named_data**, int);
    void nd_add_sorted(named_data**, named_data**);
    void nd_print(named_data*);
    void nd_print_all(named_data*);
    void nd_free(named_data**);
    void nd_free_all(named_data**);

    //subtitle functions
    int sub_resize(subtitle**, int);
    void sub_add_sorted(subtitle**, subtitle**);
    void sub_print(subtitle*);
    void sub_print_all(subtitle*);
    void sub_free(subtitle**);
    void sub_free_all(subtitle**);

    //console_event functions
    void ce_add_sorted(console_event**, console_event**);
    void ce_print_all(console_event**);
    void ce_free_all(console_event**);

    //gm2 functions
    //int gm2_init(gm2**);
    //void gm2_print(gm2*);
    //void gm2_free(gm2*);

    //testing
    void gm2s_test(void);
    void print_hex(char*, unsigned short);

#ifdef __cplusplus
}
#endif

#endif
