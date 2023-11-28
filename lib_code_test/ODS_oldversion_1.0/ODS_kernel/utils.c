/* Some utility functions borrowed from utils.c of  NIST's kernel-AODV
* implementation by Luke Klein-Berndt.*/

#include "utils.h"

/****************************************************

   utils.h
----------------------------------------------------
Contains many misc funcations that provide
basic functionality.
****************************************************/

/****************************************************

   inet_ntoa
----------------------------------------------------
Converts a IP address repersented in a 32 bit
unsigned int into a string
****************************************************/
char *inet_ntoa(__u32 ina)
{

    static char buf[4*sizeof "123"];
    unsigned char *ucp = (unsigned char *)&ina;
    sprintf(buf, "%d.%d.%d.%d",
            ucp[0] & 0xff,
            ucp[1] & 0xff,
            ucp[2] & 0xff,
            ucp[3] & 0xff);
    return buf;
}

int seq_less_or_equal(u_int32_t seq_one,u_int32_t seq_two)
{
    int *comp_seq_one = &seq_one;
    int *comp_seq_two = &seq_two;

    if (  ( *comp_seq_one - *comp_seq_two ) > 0 )
    {
        return 0;

    }
    else
        return 1;
}


/****************************************************

   inet_aton
----------------------------------------------------
Converts a string into a 32-bit unsigned int
****************************************************/
int inet_aton(const char *cp, __u32 *addr)
{
    unsigned int val;
    int                     base,
    n;
    char            c;
    u_int           parts[4];
    u_int      *pp = parts;

    for (;;)
    {


        //Collect number up to ``.''. Values are specified as for C:
        // 0x=hex, 0=octal, other=decimal.

        val = 0;
        base = 10;
        if (*cp == '0')
        {
            if (*++cp == 'x' || *cp == 'X')
                base = 16, cp++;
            else
                base = 8;
        }
        while ((c = *cp) != '\0')
        {
            if (isascii(c) && isdigit(c))
            {
                val = (val * base) + (c - '0');
                cp++;
                continue;

            }
            if (base == 16 && isascii(c) && isxdigit(c))
            {
                val = (val << 4) +
                      (c + 10 - (islower(c) ? 'a' : 'A'));
                cp++;
                continue;
            }
            break;
        }
        if (*cp == '.')
        {


            // Internet format: a.b.c.d a.b.c       (with c treated as
            // 16-bits) a.b         (with b treated as 24 bits)

            if (pp >= parts + 3 || val > 0xff)
                return (0);
            *pp++ = val, cp++;
        }
        else
            break;
    }

    // Check for trailing characters.

    if (*cp && (!isascii(*cp) || !isspace(*cp)))
        return (0);


    // Concoct the address according to the number of parts specified.

    n = pp - parts + 1;
    switch (n)
    {

    case 1:                 // a -- 32 bits
        break;

    case 2:                 //a.b -- 8.24 bits
        if (val > 0xffffff)
            return (0);
        val |= parts[0] << 24;
        break;

    case 3:                 //a.b.c -- 8.8.16 bits
        if (val > 0xffff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4:                 // a.b.c.d -- 8.8.8.8 bits
        if (val > 0xff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    }
    if (addr)
        *addr= htonl(val);
    return (1);
}

