#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <stdio.h>

using namespace std;


const char *my_v3_passphrase = "snmp.password";

/*
// 接続先のipのリスト
 */
struct s_host {
	const char *name;
} hosts[] = {
	{ "127.0.0.1" },
	{ NULL }
};

/*
// ポーリングするoidの設定リスト
 */
struct s_oid {
	const char *Name;
	oid Oid[MAX_OID_LEN];
	size_t OidLen;
} oids[] = {
	{ "IF-MIB::ifInOctets.2" },
	{ ".1.3.6.1.2.1.2.2.1.16.1" },
	{ ".1.3.6.1.2.1.2.2.1.5.1" },
	//	{ ".1.3.6.1.2.1.25.2.3.1.2.1" },
	//	{ ".1.3.6.1.2.1.25.2.3.1.4.1" },
	//	{ ".1.3.6.1.2.1.25.2.3.1.6.1" },
	//	{ ".1.3.6.1.2.1.25.2.3.1.5.1" },
	{ ".1.3.6.1.4.1.2021.4.5.0" },
	{ ".1.3.6.1.4.1.2021.4.6.0" },
	//	{ ".1.3.6.1.4.1.2021.10.1.5.1" },
	//	{ ".1.3.6.1.2.1.25.3.3.1.2" },
	{ ".1.3.6.1.4.1.2021.10.1.5.2" },
	{ ".1.3.6.1.2.1.2.2.1.10.1" },
	{ ".1.3.6.1.2.1.2.2.1.16.1" },
	{ NULL }
};

/*
// 初期化
 */
void initialize (void)
{
	struct s_oid *op = oids;
	netsnmp_variable_list *vars;
	SOCK_STARTUP;

	/* ライブラリの初期化 */
	init_snmp("snmp_start");

	/* oidのリストをパース */
	while (op->Name) {
		op->OidLen = sizeof(op->Oid)/sizeof(op->Oid[0]);
		//		cout << op->OidLen << endl;
		if (!read_objid(op->Name, op->Oid, &op->OidLen)) {
			snmp_perror("read_objid");
			exit(1);
		}
		op++;
	}
}

/*
// PRINT関数
 */
int print_result (int status, snmp_session *sp, snmp_pdu *pdu)
{
	char buf[1024];
	struct variable_list *vp;
	int ix;
	struct timeval now;
	struct timezone tz;
	struct tm *tm;

	gettimeofday(&now, &tz);
	tm = localtime(&now.tv_sec);
	fprintf(stdout, "%.2d:%.2d:%.2d.%.6d ", tm->tm_hour, tm->tm_min, tm->tm_sec,
			(int)now.tv_usec);
	switch (status) {
		case STAT_SUCCESS:
			vp = pdu->variables;
			if (pdu->errstat == SNMP_ERR_NOERROR) {
				while (vp) {
					snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);
					fprintf(stdout, "%s: %s\n", sp->peername, buf);//*(vp->val.integer)
					//cout << *vp->name+2 << endl;
					//cout << vp->name << endl;
					vp = vp->next_variable;
				}
			}
			else {
				for (ix = 1; vp && ix != pdu->errindex; vp = vp->next_variable, ix++)
					;
				if (vp) snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
				else strcpy(buf, "(none)");
				fprintf(stdout, "%s: %s: %s\n",
						sp->peername, buf, snmp_errstring(pdu->errstat));
			}
			return 1;
		case STAT_TIMEOUT:
			fprintf(stdout, "%s: Timeout\n", sp->peername);
			return 0;
		case STAT_ERROR:
			snmp_perror(sp->peername);
			return 0;
	}
	return 0;
}

struct session {
	struct snmp_session *sess;		
	struct s_oid *current_oid;		
} sessions[sizeof(hosts)/sizeof(hosts[0])];

int active_hosts;			/* まだポーリングし終えてないホストの数 */


/*
// レスポンスハンドラ　 
 */
int asynch_response(int operation, struct snmp_session *sp, int reqid,
		struct snmp_pdu *pdu, void *magic)
{
	struct session *host = (struct session *)magic;
	struct snmp_pdu *req;
	if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
		if (print_result(STAT_SUCCESS, host->sess, pdu)) {

			if(netsnmp_oid_equals(pdu->variables->name, SNMP_MIN(host->current_oid->OidLen, pdu->variables->name_length), host->current_oid->Oid, host->current_oid->OidLen) != 0);
			{	
				//cout << host->current_oid->OidLen << endl;
				//cout << pdu->variables->name_length << endl;
				//cout << "" << endl;
				//memcpy(host->current_oid->Oid, )
			}
			host->current_oid++;			/* 次のoidをセット */
			if (host->current_oid->Name) {
				req = snmp_pdu_create(SNMP_MSG_GETNEXT);/* ポーリング方法 */
				snmp_add_null_var(req, host->current_oid->Oid, host->current_oid->OidLen);
				if (snmp_send(host->sess, req))
					return 1;
				else 
				{
					snmp_perror("snmp_send");
					snmp_free_pdu(req);
				}
			}
		}
	}
	else{
		cout << "異常導線入ってます" << endl;
		print_result(STAT_TIMEOUT, host->sess, pdu);
	}
	cout << "end" << endl; 
	active_hosts--;
	return 1;
}

/*
// 非同期通信関数 
 */
void asynchronous(void)
{
	struct session *hs;
	struct s_host *hp;

	/* すべてのホストにたいして初期ポーリング */

	for (hs = sessions, hp = hosts; hp->name; hs++, hp++) {
		struct snmp_pdu *req;
		struct snmp_session sess;
		snmp_sess_init(&sess);		
		sess.version = SNMP_VERSION_3;
		sess.peername = strdup(hp->name);
		sess.securityName = (char *)"user.name";
		sess.securityNameLen = strlen(sess.securityName);
		sess.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;

		//use the authentication method to MD5
		sess.securityAuthProto = usmHMACMD5AuthProtocol;
		sess.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
		sess.securityAuthKeyLen = USM_AUTH_KU_LEN;

		//set the authentication key to a MD5
		if(generate_Ku(sess.securityAuthProto,
					sess.securityAuthProtoLen,
					(u_char *)my_v3_passphrase, strlen(my_v3_passphrase),
					sess.securityAuthKey,
					&sess.securityAuthKeyLen) != SNMPERR_SUCCESS)
		{
			snmp_log(LOG_ERR,
					"Error generating Ku from authentication pass phrase. \n");
			exit(1);
		}

		sess.callback = asynch_response;		/* コールバックセット */
		sess.callback_magic = hs;
		if (!(hs->sess = snmp_open(&sess))) {
			snmp_perror("snmp_open");
			continue;
		}
		hs->current_oid = oids;
		req = snmp_pdu_create(SNMP_MSG_GETNEXT);	/* ポーリング方法 */
		snmp_add_null_var(req, hs->current_oid->Oid, hs->current_oid->OidLen);
		if (snmp_send(hs->sess, req))
			active_hosts++;
		else {
			snmp_perror("snmp_send");
			snmp_free_pdu(req);
		}
	}

	/* Selectでアクティブホストからの連絡待ち・ブロック */

	while (active_hosts) {
		int fds = 0, block = 1;
		fd_set fdset;
		struct timeval timeout;

		FD_ZERO(&fdset);
		snmp_select_info(&fds, &fdset, &timeout, &block);
		fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
		if (fds < 0) {
			perror("select failed");
			exit(1);
		}
		if (fds){
			snmp_read(&fdset);
		} else{
			snmp_timeout();

		}
	}

	/* cleanup */

	for (hp = hosts, hs = sessions; hp->name; hs++, hp++) {
		if (hs->sess) snmp_close(hs->sess);
	}
}

int main (int argc, char **argv)
{
	initialize();
	asynchronous();
	return 0;
}
