#include "nd_timer.h"

bool operator< (const timer &t1, const timer &t2)
{

	return t1.timeout < t2.timeout;
}
bool operator<= (const timer &t1, const timer &t2)
{

	return t1.timeout <= t2.timeout;
}

bool operator> (const timer &t1, const timer &t2)
{

	return t1.timeout > t2.timeout;
}
bool operator>= (const timer &t1, const timer &t2)
{

	return t1.timeout >= t2.timeout;
}

bool operator== (const timer &t1, const timer &t2)
{

	return t1.timeout == t2.timeout;
}

bool operator != (const timer &t1, const timer &t2)
{

	return t1.timeout != t2.timeout;
}
