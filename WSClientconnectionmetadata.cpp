/*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************/
#include "WSClientconnectionmetadata.h"
 
 /*In order to process user input while network processing occurs in the background we are going to
  * use a separate thread for the WebSocket++ processing loop. This leaves the main thread free to
  *  process foreground user input. In order to enable simple RAII style resource management for our
  *  thread and endpoint we will use a wrapper object that configures them both in its constructor.
  */
 
//connection_metadata {
//public:
 
connection_metadata::connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
    : m_id(id)
    , m_hdl(hdl)
    , m_status("Connecting")
    , m_uri(uri)
    , m_server("N/A")
{}
 
connection_metadata::~connection_metadata() {}

void connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl) {
  m_status = "Open";
 
  client::connection_ptr con = c->get_con_from_hdl(hdl);
  m_server = con->get_response_header("Server");
  printf("connection_metadata::on_open m_status: %s m_server: %s \n",m_status, m_server);
}
  
  /* Receiving messages Messages are received by registering a message handler.
   * This handler will be called once per message received and its signature is 
   * void on_message(websocketpp::connection_hdl hdl, endpoint::message_ptr msg). The connection_hdl,
   * like the similar parameter from the other handlers is a handle for the connection that the message
   * was received on. The message_ptr is a pointer to an object that can be queried for the message payload,
   * opcode, and other metadata. Note that the message_ptr type, as well as its underlying message type,
   * is dependent on how your endpoint is configured and may be different for different configs.
   * The message receiving behave that we are implementing will be to collect all messages sent
   * and received and to print them in order when the show connection command is run.
   * The sent messages are already being added to that list. Now we add a message handler that
   * pushes received messages to the list as well. Text messages are pushed as-is. Binary messages
   * are first converted to printable hexadecimal format.
   */
  void connection_metadata::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
//		printf("connection_metadata::on_message opcode is text entered  msg->get_payload(): %s\n",msg->get_payload().c_str());
		// added below to process callback function
		on_message_callback(msg->get_payload());
        m_messages.push_back(msg->get_payload());
    } else {
        m_messages.push_back(websocketpp::utility::to_hex(msg->get_payload()));
    }
  }
 
  void connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Failed";
 
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
	printf("connection_metadata::on_fail m_error_reason: %s m_server: %s \n", m_error_reason.c_str(), m_server.c_str());
  }
  /* The WebSocket close handshake involves an exchange of optional machine readable close
   * codes and human readable reason strings. Each endpoint sends independent close details.
   * The codes are short integers. The reasons are UTF8 text strings of at most 125 characters.
   * More details about valid close code ranges and the meaning of each code can be found at
   * https://tools.ietf.org/html/rfc6455#section-7.4  The websocketpp::close::status namespace
   * contains named constants for all of the IANA defined close codes. It also includes free functions
   * to determine whether a value is reserved or invalid and to convert a code to a human readable text
   * representation.

   * During the close handler call WebSocket++ connections offer the following methods for accessing
   * close handshake information:
   *  .connection::get_remote_close_code(): Get the close code as reported by the remote endpoint
   *  .connection::get_remote_close_reason(): Get the close reason as reported by the remote endpoint
   *  .connection::get_local_close_code(): Get the close code that this endpoint sent.
   *  .connection::get_local_close_reason(): Get the close reason that this endpoint sent.
   *  .connection::get_ec(): Get a more detailed/specific WebSocket++ error_code indicating what library
   *     error (if any) ultimately resulted in the connection closure.
   * Note: there are some special close codes that will report a code that was not actually sent on the wire.
   * For example 1005/"no close code" indicates that the endpoint omitted a close code entirely and
   * 1006/"abnormal close" indicates that there was a problem that resulted in the connection closing
   * without having performed a close handshake. */
  void connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Closed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    std::stringstream s;
 //   s << "close code: " << con->get_remote_close_code() << " (" 
 //     << websocketpp::close::status::get_string(con->get_remote_close_code()) 
 //     << "), close reason: " << con->get_remote_close_reason();
 //     m_error_reason = s.str();
     printf("connection_metadata::on_close m_error_reason: %s \n", m_error_reason.c_str());
  }

  void connection_metadata::record_sent_message(std::string message) {
    m_messages.push_back(">> " + message);
  }

  std::string connection_metadata::get_status () {
    return m_status;
  }
  
  int connection_metadata::get_id () {
    return m_id; 
  }

  websocketpp::connection_hdl connection_metadata::get_hdl () {
    return m_hdl; 
  }
  
  void connection_metadata::registerOnMessageCallback(f_on_message_t onMessageFuncPtr)
  {
     on_message_callback = onMessageFuncPtr;
  }
//std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
//    out << "> URI: " << data.m_uri << "\n"
//        << "> Status: " << data.m_status << "\n"
//        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
//        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason);
//	out << "> Messages Processed: (" << data.m_messages.size() << ") \n";
 //   std::vector<std::string>::const_iterator it;
//      for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
//        out << *it << "\n";
 //   }
 //   return out;
//}
