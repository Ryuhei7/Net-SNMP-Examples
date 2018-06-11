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
//int con[7]; //ポーリングしてきた数値の数
const char *my_v3_passphrase = "snmp.password";

int snmp(){
	netsnmp_session session;
	netsnmp_session *sess_handle;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;
	netsnmp_variable_list *vars;

	oid id_oid[MAX_OID_LEN];
	oid serial_oid[MAX_OID_LEN];

	size_t id_len = MAX_OID_LEN;
	size_t serial_len = MAX_OID_LEN;
	int status;

	ifstream ifs("oids.txt");
	int fscounter = 0;
	char str[64];
	char oids[10][64];
	if (ifs.fail())
	{
		cerr << "error" << endl;
		return -1;
	}
	while (ifs.getline(str, 64 - 1))
	{
		strcpy(oids[fscounter], str);
		fscounter = fscounter + 1;
	}

	//////////////////////////////////


	init_snmp("SNMP");

	snmp_sess_init( &session );
	session.version = SNMP_VERSION_3;
	session.peername = (char *)"127.0.0.1"; //自身のMIBへアクセス
	session.securityName = (char *)"user.name";
	session.securityNameLen = strlen(session.securityName);
	session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;

	//use the authentication method to MD5
	session.securityAuthProto = usmHMACMD5AuthProtocol;
	session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
	session.securityAuthKeyLen = USM_AUTH_KU_LEN;

	//set the authentication key to a MD5
	if(generate_Ku(session.securityAuthProto,
								session.securityAuthProtoLen,
								(u_char *)my_v3_passphrase, strlen(my_v3_passphrase),
								session.securityAuthKey,
								&session.securityAuthKeyLen) != SNMPERR_SUCCESS) 
	{
		snmp_log(LOG_ERR,
						"Error generaring Ku from authentication pass phrase. \n");
		exit(1);
	}

	SOCK_STARTUP;	
	sess_handle = snmp_open(&session);
	if(!sess_handle){
		snmp_sess_perror("ack", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	pdu = snmp_pdu_create(SNMP_MSG_GET); //ポーリングする方法を指定  
	for (int i = 0; i < sizeof(oids)/sizeof(oids[0]); i++ ){
		read_objid(oids[i], id_oid, &id_len);
		snmp_add_null_var(pdu, id_oid, id_len);
		}

	status = snmp_synch_response(sess_handle, pdu, &response);//ここでポーリングしてる

	for(vars = response->variables; vars; vars = vars->next_variable){
	  //con[counter] = *(vars->val.integer);
		//counter = counter + 1;
		print_value(vars->name, vars->name_length, vars);
		}

	if(response)
		snmp_free_pdu(response);
	snmp_close(sess_handle);

	SOCK_CLEANUP;
	return (0);
}

int main() {
	snmp();
/*
	sleep(3);
	snmp();

	float in = (con[3] - con[0]) * 8 * 100 / con[2];
	float out = (con[4] - con[1]) * 8 * 100 /con[2];

	cout << "InOctet = " << in << "% 使用中"<< endl;
	cout << "OutOctet = " << out << "% 使用中" << endl; 
*/
}
