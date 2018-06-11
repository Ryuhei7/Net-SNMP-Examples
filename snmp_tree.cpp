#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h" 
#include <string.h>
#include <iostream>
using namespace std;

int find_last_oid(netsnmp_session * ss,oid *base,int base_length){ 
	netsnmp_pdu *response; 
	netsnmp_pdu *pdu; 
	int running = 1; 
	int status; 
	int length=0; 
	pdu = snmp_pdu_create(SNMP_MSG_GETNEXT); 
	snmp_add_null_var(pdu, base, base_length); 
	while(running){ 
		status = snmp_synch_response(ss, pdu, &response); 
		if (status != STAT_SUCCESS || !response){ 
			snmp_sess_perror("snmp_synch_response", ss); 
			exit(1); 
		} 
		if(response->errstat != SNMP_ERR_NOERROR){ 
			fprintf(stderr, "snmp: Error in packet: %s\n",snmp_errstring(response->errstat)); 
			exit(1); 
		} 
		if(response && snmp_oid_compare(response->variables->name,SNMP_MIN(base_length,response->variables->name_length),base, base_length) != 0){ 
			running = 0;
			cout << "さっきのが最後だったみたい" << endl;
			cout << response->variables->name_length << endl;
			cout << SNMP_MIN(base_length,response->variables->name_length)  << endl;
			cout << base_length << endl; 
		}
		else{
			cout << "もう一周いくよ" << endl;
			
			memcpy(base,response->variables->name,response->variables->name_length*sizeof(oid)); 
			length=response->variables->name_length; 
			pdu = snmp_pdu_create(SNMP_MSG_GETNEXT); 
			snmp_add_null_var(pdu, response->variables->name, 
					response->variables->name_length); 
		} 
		snmp_free_pdu(response); 
	}

	return length; 
} 
int main(){ 
	netsnmp_session session, *ss; 
	netsnmp_pdu *pdu; 
	netsnmp_pdu *response; 
	struct variable_list *vars;  
	oid base[128]={1,3,6,1,2,1,2,2,1,10}; 
	size_t base_length=10; 
	int status; 
	init_snmp("APC Check"); 
	snmp_sess_init(&session); 
	session.version = SNMP_VERSION_1; 
	session.community = (u_char*)"public"; 
	session.community_len = strlen((const char*)session.community); 
	session.peername = "127.0.0.1";//ここでIP指定
	ss = snmp_open(&session); 
	if(ss == NULL){ 
		snmp_sess_perror("snmp_open", &session); 
		exit(1); 
	} 
	int new_length=find_last_oid(ss, base, base_length); 
	pdu = snmp_pdu_create(SNMP_MSG_GET); 
	snmp_add_null_var(pdu, base, new_length); 
	status = snmp_synch_response(ss, pdu, &response); 
	if(status != STAT_SUCCESS || !response){ 
		snmp_sess_perror("snmp_synch_response", ss); 
		exit(1); 
	} 
	for(vars = response->variables; vars; vars = vars->next_variable){ 
		int i; 
		for(i=0;i<vars->name_length;i++){ 
			printf("%d ",vars->name[i]); 
		} 
		print_value(vars->name, vars->name_length, vars); 
	} 
	snmp_free_pdu(response); 
	snmp_close(ss); 
	return 0; 
} 
