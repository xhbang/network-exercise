#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 3) // Test for correct number of arguments
        DieWithUserMessage("Parameter(s)", "<Address/Name> <Port/Service>");
    char *addrString = argv[1];
    char *portString = argv[2];
    // Server address/name
    // Server port/service
    // Tell the system what kind(s) of address info
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;
    // Get address(es) associated with the specified name/service
    struct addrinfo *addrList; // Holder for list of addresses returned
    // Modify servAddr contents to reference linked list of addresses
    int rtnVal = getaddrinfo(addrString, portString, &addrCriteria, &addrList);
    if (rtnVal != 0)
        DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
    // Display returned addresses
    struct addrinfo *addr = addrList;
    for (; addr != NULL; addr = addr->ai_next){
        PrintSocketAddress(addr->ai_addr, stdout);
        fputc('\n', stdout);
    }
    freeaddrinfo(addrList); // Free addrinfo allocated in getaddrinfo()
    exit(0);
}
