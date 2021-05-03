/*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************/

// Please note: initial websockets rdkservices communication will be in the clear
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
 
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
 
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <stdlib.h>     //for using the function sleep
 
typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
public:
  typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;
  // std::string response message vector for both synchronous and asynchronous messages
  // received from websockets server
  std::vector<std::string> m_messages;
  connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri);
  ~connection_metadata();
  void on_open(client * c, websocketpp::connection_hdl hdl);
  void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);
  void on_fail(client * c, websocketpp::connection_hdl hdl);
  void on_close(client * c, websocketpp::connection_hdl hdl);
  void record_sent_message(std::string message);
  std::string get_status ();
  int get_id ();
  websocketpp::connection_hdl get_hdl ();
  //return_type (*)(parameter_type_1, parameter_type_2, parameter_type_3)
  // void (*)(std::string);
  typedef void (*f_on_message_t) (std::string);
  f_on_message_t on_message_callback;
  void registerOnMessageCallback(f_on_message_t onMessageFuncPtr);
//  friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);
private:
  int m_id;
  /* Because of the limited thread safety of the connection_ptr the library also provides a
   * more flexible connection identifier, the connection_hdl. The connection_hdl has type
   * websocketpp::connection_hdl and it is defined in <websocketpp/common/connection_hdl.hpp>.
   * Note that unlike connection_ptr this is not dependent on the type or config of the endpoint.
   * Code that simply stores or transmits connection_hdl but does not use them can include only
   * the header above and can treat its hdls like values. Connection handles are not used directly.
   * They are used by endpoint methods to identify the target of the desired action.
   * For example, the endpoint method that sends a new message will take as a parameter the hdl
   * of the connection to send the message to.
   * When is it safe to use connection_hdl? connection_hdls may be used at any time from any thread.
   * They may be copied and stored in containers. Deleting a hdl will not affect the connection in any way.
   * Handles may be upgraded to a connection_ptr during a handler call by using endpoint::get_con_from_hdl()
   *   . The resulting connection_ptr is safe to use for the duration of that handler invocation.
   *     connection_hdls are guaranteed to be unique within a program. Multiple endpoints in a single program
   *     will always create connections with unique handles.
   *   .Using a connection_hdl with a different endpoint than the one that created its associated connection
   *    will result in undefined behavior.
   *   .Using a connection_hdl whose associated connection has been closed or deleted is safe.
   *    The endpoint will return a specific error saying the operation couldn't be completed because the
   *    associated connection doesn't exist. [TODO: more here? link to a connection_hdl FAQ elsewhere?] */
  websocketpp::connection_hdl m_hdl;
  std::string m_status;
  std::string m_uri;
  std::string m_server;
  std::string m_error_reason;
};
