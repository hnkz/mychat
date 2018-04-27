#include <sqlite3.h>

char*       compute_md5(char *data, char *mdString);
void        test_db();
int         login_db_process(char *username, char *pass_md5);
sqlite3*    connect_db(char *db_filename);
int         get_all_line(sqlite3 *conn, char *table, char *err_msg);
int         print_resp(void *get_prm, int col_cnt, char **row_txt, char **col_name);
int         get_row(void *get_prm, int col_cnt, char **row_txt, char **col_name);
int         get_line_where(sqlite3 *conn, char *table, char *values, char *err_msg, int *row_num_p);
