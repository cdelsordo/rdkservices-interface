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
#include "WSClientconnectionmetadata.h"

class websocket_endpoint {
public:
  typedef websocketpp::client<websocketpp::config::asio_client> client;
  typedef std::map<int, connection_metadata::ptr> con_list;
  client m_endpoint;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
 
  con_list m_connection_list;
  client::connection_ptr m_currentConnection;
  int m_next_id; 

  websocket_endpoint();
  ~websocket_endpoint();
  int connect(std::string const & uri);
  connection_metadata::ptr get_metadata(int id);
  void close(int id, websocketpp::close::status::value code, std::string reason);
  void send(int id, std::string message);
  void wait_connect(int waitTime);
private:
  
};  