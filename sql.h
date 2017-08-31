#ifndef _SQL_H_
#define _SQL_H_


#include<iostream>
#include<mysql.h>
#include<string>
#include<string.h>
#include<stdio.h>
class sql
{
public:
	sql(const std::string &user,const std::string &ip,const std::string &passwd,const std::string &db,const int &port);
	int connect();
	int insert(const std::string &name,const std::string &sex,\
				const std::string &school,const std::string &hobby);
	int select();
	~sql();
private:
	MYSQL* conn;
	std::string _user;
	std::string _ip;
	std::string _passwd;
	std::string _db;
	int _port;
};

#endif
