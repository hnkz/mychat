#include <sqlite3.h>

extern char line[100][1024];

char*       compute_md5(char *data, char *mdString);
sqlite3*    connect_db(char *db_filename);
int         insert(sqlite3 *conn, char *table, char *values, char *err_msg);

void        insert_log(char *log_val, char *table);
int         get_row_num(char *username, char *pass_md5);
int         get_all_line(char *table, char *where, char *err_msg);
int         get_line_where(sqlite3 *conn, char *table, char *values, char *err_msg, int *row_num_p);

int         print_resp(void *get_prm, int col_cnt, char **row_txt, char **col_name);
int         set_row_num(void *get_prm, int col_cnt, char **row_txt, char **col_name);
int         set_line(void *get_prm, int col_cnt, char **row_txt, char **col_name);