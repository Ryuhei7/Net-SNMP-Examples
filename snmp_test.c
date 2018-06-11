#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

int snmp()/*int argc, char ** argv*/
{
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

  /*********************/

  init_snmp("SNMP Test");

  snmp_sess_init( &session );
  session.version = SNMP_VERSION_1;
  session.community = "public";
  session.community_len = strlen(session.community);
  session.peername = "127.0.0.1";
  sess_handle = snmp_open(&session);
 
 //ここはGETNEXT＝snmpwalkと考えてよい。こっちだとOID配下をすべて持ってこれる。
  pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);

  read_objid(".1.3.6.1.2.1.2.2.1.16.1", id_oid, &id_len);
  snmp_add_null_var(pdu, id_oid, id_len); 
  read_objid(".1.3.6.1.2.1.2.2.1.10.1", id_oid, &id_len);
  snmp_add_null_var(pdu, id_oid, id_len);
  read_objid(".1.3.6.1.2.1.2.2.1.5", id_oid, &id_len);
  snmp_add_null_var(pdu, id_oid, id_len);

  
  status = snmp_synch_response(sess_handle, pdu, &response);

  for(vars = response->variables; vars; vars = vars->next_variable){
    print_value(vars->name, vars->name_length, vars);
  }
  
  snmp_free_pdu(response);
  snmp_close(sess_handle);

  return (0);
}

int main() {
  snmp();
  sleep(2);
  snmp();
  }
