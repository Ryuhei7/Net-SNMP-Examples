#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <stdio.h>
//#include <math.h>

using namespace std;

int counter=0;
int con[7]; //ポーリングしてきた数値の数


int snmp(){
	struct snmp_session session;
	struct snmp_session *sess_handle;
	
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;

	struct variable_list *vars;

	oid id_oid[MAX_OID_LEN];
	oid serial_oid[MAX_OID_LEN];

	size_t id_len = MAX_OID_LEN;
	size_t serial_len = MAX_OID_LEN;

	int status;
	
	ifstream ifs("oids.txt");
	int fscounter = 0;
	char str[32];
	char oids[3][32];
	if (ifs.fail())
	{
		cerr << "error" << endl;
		return -1;
	}
	while (ifs.getline(str, 32 - 1))
	{
		strcpy(oids[fscounter], str);
		fscounter = fscounter + 1;
	}
	//////////////////////////////////

	init_snmp("SNMP");

	snmp_sess_init( &session );
	session.version = SNMP_VERSION_2c;
	session.community = (u_char *)"public";
	session.community_len = strlen((const char *)session.community);
	session.peername = (char *)"127.0.0.1"; //自身のMIBへアクセス
	sess_handle = snmp_open(&session);

	pdu = snmp_pdu_create(SNMP_MSG_GETNEXT); //ポーリングする方法を指定

  
	for (int i = 0; i < sizeof(oids)/sizeof(oids[0]); i++ ){
		read_objid(oids[i], id_oid, &id_len);
		snmp_add_null_var(pdu, id_oid, id_len);
		}

	status = snmp_synch_response(sess_handle, pdu, &response);

	for(vars = response->variables; vars; vars = vars->next_variable){
		con[counter] = *(vars->val.integer);
		counter = counter + 1;
		}

	snmp_free_pdu(response);
	snmp_close(sess_handle);

	return (0);
}

int main() {
	snmp();
	sleep(3);
	snmp();

	float in = (con[3] - con[0]) * 8 * 100 / con[2];
	float out = (con[4] - con[1]) * 8 * 100 /con[2];

	cout << "InOctet = " << in << "% 使用中"<< endl;
	cout << "OutOctet = " << out << "% 使用中" << endl; 

}
