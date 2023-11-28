#include <iostream>
#include "../lib/FO/fo.h"

using namespace std;

char *ntoa(int addr)
{
  static char buffer[18];
  sprintf(buffer, "%d.%d.%d.%d",
          (addr & 0x000000FF)      ,
          (addr & 0x0000FF00) >>  8,
          (addr & 0x00FF0000) >> 16,
          (addr & 0xFF000000) >> 24);
  return buffer;
}


/* For printing the routes. */
void printRoute(struct route_entry *rtInfo)
{
  /* Print Destination address */
  printf("%s\t", rtInfo->dst ? ntoa(rtInfo->dst) : "0.0.0.0  ");


  /* Print Gateway address */
  printf("%s\t", rtInfo->gateway ? ntoa(rtInfo->gateway) : "*.*.*.*");


  /* Print Interface Name */
  char *name;
  if_indextoname(rtInfo->ifindex, name);
  printf("%s\t", name);


  /* Print Source address */
  printf("%s\t", rtInfo->src ? ntoa(rtInfo->src) : "*.*.*.*");

  /* Print Table */
  printf("%d\t", rtInfo->table);

  /* Print Priority */
  printf("%d\n", rtInfo->metric);
}


int main(){
    printf("hello world");
    int ret = init_fo_socket();

    // char *dst = "10.34.45.123";
    char *dst = "0.0.0.0";
    char *gateway = "1.2.3.4";
    int ifindex = if_nametoindex("mytun");
    
    int metric = 10;

    uint32_t dst_net,gateway_net;
    dst_net = inet_addr(dst);
    gateway_net = inet_addr(gateway);

    int re = FO_add_a_route(dst_net, 0, gateway_net, ifindex, metric, 0);
    printf("%d\n", re);

    route_entry* table = (struct route_entry*)malloc(20 * sizeof(struct route_entry));
    int len = 0;
    int re2 = FO_get_routes(table, &len);

    printf("len is %d\n", len);
    route_entry* head = table;
    for(int i=0;i< len;i++){

        in_addr dst_net;
        dst_net.s_addr = head->dst;
        char *dst_char = inet_ntoa(dst_net);
        printf("%s\t", dst_char);
        
        in_addr gwy_net;
        gwy_net.s_addr = head->gateway;
        printf("%s\t", inet_ntoa(gwy_net));

        printf("%d\t", head->metric);

        char *name;
        if_indextoname(head->ifindex, name);
        printf("%s\t", name);

        in_addr src_net;
        src_net.s_addr = head->src;
        printf("%s\t", inet_ntoa(src_net));

        printf("\n");

        head += sizeof(struct route_entry);
    }



    return 0;
}