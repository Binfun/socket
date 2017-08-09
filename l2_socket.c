#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define L2TYPE 0x0807
#define _setbits(flag,mask) flag |=  (mask)
#define _anyset(flag,mask) ((flag) & (mask)) != (0)

int l2_socket_init(char *ifname)
{
	int sock;
	struct ifreq ifreq;
	struct sockaddr_ll sockaddr_ll = 
	{
		PF_PACKET,
		0x0000,
		0x0000,
		ARPHRD_ETHER,
		PACKET_HOST,
		MAC_ADDR_LEN,
		{
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00
		}
	};

	sockaddr_ll.sll_protocol = htons(L2TYPE);

	if((sock = socket(sockaddr_ll.sll_family, SOCK_RAW, sockaddr_ll.sll_protocol)) == -1){
		printf(("create socket error : %s :%s\n", ifname, strerror(errno)));
		goto error;
	}
	memcpy (ifreq.ifr_name, ifname, sizeof (ifreq.ifr_name));

	if (ioctl (sock, SIOCGIFINDEX, &ifreq) == -1) 
	{
		printf(("get SIOCGIFINDEX error : %s :%s\n", ifname,strerror(errno)));
		goto error;
	}
	sockaddr_ll.sll_ifindex = ifreq.ifr_ifindex;

	if (ioctl (sock, SIOCGIFHWADDR, &ifreq) == -1) 
	{
		printf(("get SIOCGIFHWADDR error : %s : %s\n", ifname,strerror(errno)));
		goto error;
	}
	memcpy (sockaddr_ll.sll_addr, ifreq.ifr_ifru.ifru_hwaddr.sa_data, sizeof (sockaddr_ll.sll_addr));
	if (bind (sock, (struct sockaddr *) (&sockaddr_ll), sizeof (sockaddr_ll)) == -1) 
	{
		printf(("bind socket error : %s : %s\n", ifname,strerror(errno)));
		goto error;
	}

	if (ioctl (sock, SIOCGIFFLAGS, &ifreq) == -1) 
	{
		printf(("get SIOCGIFFLAGS error : %s : %s\n", ifname,strerror(errno)));
		goto error;
	}
	//chan->setting.ifstate = ifreq.ifr_flags;
	
	_setbits (ifreq.ifr_flags, (IFF_UP | IFF_BROADCAST | IFF_MULTICAST));
	_clrbits (ifreq.ifr_flags, (IFF_ALLMULTI));

	if (ioctl (sock, SIOCSIFFLAGS, &ifreq) == -1) 
	{
		printf(("set SIOCSIFFLAGS error : %s :%s\n", ifname,strerror(errno)));
		goto error;
	}

	return sock;

error:
	if(sock>0){
		close(sock);
	}
	return -1;
}
