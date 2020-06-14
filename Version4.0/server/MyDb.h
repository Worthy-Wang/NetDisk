#pragma once
#include <string>
#include <mysql/mysql.h>
#include <iostream>
using namespace std;

class MyDb
{
public:
    MyDb();
    ~MyDb();
    bool initDB(string host, string user, string pwd, string db_name, int port);
    bool exeSQL(string sql);
    bool select_one_SQL(std::string sql, string &str);
    bool select_many_SQL(std::string sql, string &str);
private:
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
};

MyDb::MyDb()
{
    mysql = mysql_init(NULL);

    if (!mysql)
    {
        cout << "Error:" << mysql_error(mysql);
        exit(1);
    }
}

MyDb::~MyDb()
{
    if (mysql)
    {
        mysql_close(mysql);
    }
}

bool MyDb::initDB(std::string host, std::string user, std::string passwd, std::string db_name, int port = 3306)
{
    mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), port, NULL, 0);
    if (!mysql)
    {
        cout << "Error: " << mysql_error(mysql);
        return false;
    }

    return true;
}

bool MyDb::exeSQL(std::string sql)
{
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }

    result = mysql_store_result(mysql);

    if (result)
    {
        //获取结果集中总共的字段数，即列数
        int num_fields = mysql_num_fields(result);
        unsigned long long num_rows = mysql_num_rows(result);
        for (unsigned long long i = 0; i < num_rows; i++)
        {
            row = mysql_fetch_row(result);
            if (!row)
            {
                break;
            }

            for (int j = 0; j < num_fields; j++)
            {
                cout << row[j] << "\t\t";
            }
            cout << endl;
        }
    }
    else
    {
        //代表执行的是update,insert,delete类的非查询语句
        if (mysql_field_count(mysql) == 0)
        {
            // 返回update,insert,delete影响的行数
            unsigned long long num_rows = mysql_affected_rows(mysql);

            return num_rows;
        }
        else
        {
            cout << "Get result error: " << mysql_error(mysql);
            return false;
        }
    }

    return true;
}

//只查找一个字段一行，并存入str中, 如果查找的不存在，那么str仍将会为空值
bool MyDb::select_one_SQL(std::string sql, string &str)
{
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }

    result = mysql_store_result(mysql);

    if (result)
    {
        //获取结果集中总共的字段数，即列数
        int num_fields = mysql_num_fields(result);
        unsigned long long num_rows = mysql_num_rows(result);

        for (unsigned long long i = 0; i < num_rows; i++)
        {
            row = mysql_fetch_row(result);
            if (!row)
            {
                break;
            }

            for (int j = 0; j < num_fields; j++)
            {
                str = row[j];
            }
        }
    }
    return true;
}


//查找多个字段多个行，并存入str中
bool MyDb::select_many_SQL(std::string sql, string &str)
{
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }

    result = mysql_store_result(mysql);

    if (result)
    {
        //获取结果集中总共的字段数，即列数
        int num_fields = mysql_num_fields(result);
        unsigned long long num_rows = mysql_num_rows(result);

        for (unsigned long long i = 0; i < num_rows; i++)
        {
            row = mysql_fetch_row(result);
            if (!row)
            {
                break;
            }

            for (int j = 0; j < num_fields; j++)
            {
                str += row[j];
                str += "|\t\t";
            }
            str += "\n";
        }
    }
    return true;
}