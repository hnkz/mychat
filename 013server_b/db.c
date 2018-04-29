#include "../exp1.h"
#include "../exp1lib.h"
#include "db.h"
#include <openssl/md5.h>

int row_num = 0;
int line_num = 0;

char line[100][1024];

char* compute_md5(char *data, char *mdString) {
    MD5_CTX c;
    unsigned char md[MD5_DIGEST_LENGTH];
    int r, i;
    
    r = MD5_Init(&c);
    if(r != 1) {
        perror("init");
        exit(1);
    }
    
    r = MD5_Update(&c, data, strlen(data));
    if(r != 1) {
        perror("update");
        exit(1);
    }
    
    r = MD5_Final(md, &c);
    if(r != 1) {
        perror("final");
        exit(1);
    }
 
    for(i = 0; i < 16; i++)
         sprintf(&mdString[i * 2], "%02x", (unsigned int)md[i]);

    return mdString;
}

sqlite3 *connect_db(char *db_filename){
    int ret       = 0;
    sqlite3 *conn = NULL;

    ret = sqlite3_open(db_filename, &conn);
    if(SQLITE_OK != ret) {
        perror("sqlite3 connecting error");
        exit(-1);
    }

    return conn;
}

int insert(sqlite3 *conn, char *table, char *values, char *err_msg) {
    int ret       = 0;
    char sql_str[255];

    memset(&sql_str[0] , 0x00 , sizeof(sql_str));

    snprintf(&sql_str[0] , sizeof(sql_str)-1 , "insert into %s values(%s)", table, values);
    ret = sqlite3_exec(
            conn        , // DBコネクション
            &sql_str[0] , // SQL文
            NULL  , // コールバック関数
            NULL        , // CB関数に渡す引数
            &err_msg      // エラーメッセージ
        );
    return ret;
}

int get_all_line(char *table, char *where, char *err_msg) {
    int ret = 0;
    sqlite3 *conn = NULL;
    char sql_str[255];

    conn = connect_db("./db_test.sqlite3");

    if( SQLITE_OK != ret ){
        sqlite3_close( conn );
        printf("err: %s\n", err_msg);
        sqlite3_free( err_msg );
        exit(-1);
    }

    line_num = 0;

    memset(&sql_str[0], 0, sizeof(sql_str));

    snprintf(&sql_str[0] , sizeof(sql_str)-1 , "select * from %s where %s", table, where);
    ret = sqlite3_exec(
            conn        , // DBコネクション
            &sql_str[0] , // SQL文
            set_line  , // コールバック関数
            NULL        , // CB関数に渡す引数
            &err_msg      // エラーメッセージ
        );
    
    return line_num;
}

int get_line_where(sqlite3 *conn, char *table, char *where, char *err_msg, int *row_num_p) {
    int ret = 0;
    char sql_str[255];

    memset(&sql_str[0] , 0x00 , sizeof(sql_str));

    snprintf(sql_str , sizeof(sql_str)-1 , "select * from %s where %s", table, where);
    ret = sqlite3_exec(
            conn        , // DBコネクション
            &sql_str[0] , // SQL文
            set_row_num  , // コールバック関数
            NULL        , // CB関数に渡す引数
            &err_msg      // エラーメッセージ
        );
    *row_num_p = row_num;
    row_num = 0;
    return ret;
}

void insert_log(char *log_val, char *table) {
    int ret = 0;
    char *err_msg = NULL;
    sqlite3 *conn = NULL;

    conn = connect_db("./db_test.sqlite3");

    if( SQLITE_OK != ret ){
        sqlite3_close( conn );
        printf("err: %s\n", err_msg);
        sqlite3_free( err_msg );
        exit(-1);
    }

    ret = insert(conn, table, log_val, err_msg);
    if( SQLITE_OK != ret ){
        sqlite3_close( conn );
        printf("err: %s\n", err_msg);
        sqlite3_free( err_msg );
        exit(-1);
    }
    
    ret = sqlite3_close( conn );
    if( SQLITE_OK != ret ){
        exit(-1);
    } 
}

int get_row_num(char *username, char *pass_md5) {
	int ret = 0;
	int row_num;
	char *err_msg = NULL;
	sqlite3 *conn = NULL;
	char where[256];

	snprintf(where, sizeof(where)-1, "user_id=='%s' and pass=='%s'", username, pass_md5);

	conn = connect_db("./db_test.sqlite3");
	ret = get_line_where(conn, "user", where, err_msg, &row_num);

	if( SQLITE_OK != ret ){
		sqlite3_close( conn );
		printf("err: %s\n", err_msg);
		sqlite3_free( err_msg );
		return -1;
	}
	
	ret = sqlite3_close( conn );
	if( SQLITE_OK != ret ){
		exit(-1);
	}

	return row_num;
}

int set_row_num(
      void *get_prm   , // sqlite3_exec()の第4引数
      int col_cnt     , // 列数
      char **row_txt  , // 行内容
      char **col_name   // 列名
){
    row_num = sizeof(row_txt)/sizeof(*row_txt);
    return 0;
}

/**
 * print return lines
 * 
 */
int print_resp(
      void *get_prm   , // sqlite3_exec()の第4引数
      int col_cnt     , // 列数
      char **row_txt  , // 行内容
      char **col_name   // 列名
){
    int i;
    for(i = 0; i < col_cnt-1; i++) {
        printf("%s | ", row_txt[0]);
    }
    printf("%s\n", row_txt[i]);
    return 0;
}

int set_line(void *get_prm, int col_cnt, char **row_txt, char **col_name) {
    char log[1024];
    int i;
    
    memset(log, 0, sizeof(log));

    for(i = 0; i < col_cnt-1; i++) {
        strcat(log, row_txt[i]);
        strcat(log, " | ");
    }
    strcat(log, row_txt[i]);

    strcpy(line[line_num++], log);

    return 0;
}