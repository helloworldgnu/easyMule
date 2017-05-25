#pragma once

class CNonCollector
{
public:
	CNonCollector(void);
	~CNonCollector(void);

	BOOL	Proc(LPCTSTR lpszEd2kLink);

	BOOL SendEd2kToCollector(LPCTSTR lpszEd2kLink);
};
