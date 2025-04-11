#include"user.h"

void User::Init(string* UserId)
{

	strcpy(m_userId, UserId->c_str());
	cout << "init 아이디 : " << m_userId << endl;
	const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	const size_t max_index = sizeof(alphanum) - 1;

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dis(0, max_index);

	for (size_t i = 0; i < 8; i++) {
		m_userKey += alphanum[dis(gen)];
	}
	cout << " 아이디 : " << *UserId;

}