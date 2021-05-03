/*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************/
#include "WSClientwebsocketendpoint.h"
 
  websocket_endpoint::websocket_endpoint () : m_next_id(0)
  {
    /*Within the websocket_endpoint constructor several things happen, First, we set the endpoint logging
     *behavior to silent by clearing all of the access and error logging channels.
     *[TODO: link to more information about logging] */
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
    /* Next, we initialize the transport system underlying the endpoint and set it to perpetual mode.
     *In perpetual mode the endpoint's processing loop will not exit automatically when it has no
     *connections. This is important because we want this endpoint to remain active while our
     *application is running and process requests for new WebSocket connections on demand as we
     *need them. Both of these methods are specific to the asio transport. They will not be necessary
     *or present in endpoints that use a non-asio config */
    m_endpoint.init_asio();
    m_endpoint.start_perpetual();
    /* Finally, we launch a thread to run the run method of our client endpoint.
     *While the endpoint is running it will process connection tasks (read and deliver
     *incoming messages, frame and send outgoing messages, etc). Because it is running in
     *perpetual mode, when there are no connections active it will wait for a new connection. */
    m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
  }  // End websocket_endpoint constructor
  
  /* The destructor for websocket_endpoint now stops perpetual mode (so the run thread exits after
   * the last connection is closed) and iterates through the list of open connections and requests a clean
   * close for each. Finally, the run thread is joined which causes the program to wait until those
   * connection closes complete. */
  websocket_endpoint::~websocket_endpoint() {
    m_endpoint.stop_perpetual();
    
    for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
      if (it->second->get_status() != "Open") {
        // Only close open connections
        continue;
      }
      websocketpp::lib::error_code ec;
      m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
      if (ec) {
		printf(" websocket_endpoint::~websocket_endpoint Error closing connection ec %d \n",ec);
      }
    }
    m_thread->join();
	m_thread.reset();
  }  // End End websocket_endpoint destructor
  
  /*A new WebSocket connection is initiated via a three step process.
   *First, a connection request is created by endpoint::get_connection(uri).
   *Next, the connection request is configured. Lastly, the connection request
   *is submitted back to the endpoint via endpoint::connect() which adds it to the queue
   *of new connections to make.
   *After endpoint::get_connection(...) and before endpoint::connect(): get_connection returns a
   *connection_ptr. It is safe to use this pointer to configure your new connection. Once you submit
   *the connection to connect you may no longer use the connection_ptr and should discard it immediately
   *for optimal memory management.
   *During a handler: WebSocket++ allows you to register hooks / callbacks / event handlers for
   *specific events that happen during a connection's lifetime. During the invocation of one
   *of these handlers the library guarantees that it is safe to use a connection_ptr for the
   *connection associated with the currently running handler.
   *websocket_endpoint::connect() begins by calling endpoint::get_connection() using a uri passed
   *as a parameter. Additionally, an error output value is passed to capture any errors that might
   *occur during. If an error does occur an error notice is printed along with a descriptive message
   *and the -1 / 'invalid' value is returned as the new ID. */
  int websocket_endpoint::connect(std::string const & uri) {
	printf("websocket_endpoint::connect entered uri: %s \n", uri.c_str());
    websocketpp::lib::error_code ec;
	// if you are running your client endpoint in perpetual mode there is no need to reset the endpoint
	// or the Asio io_service between connects. An endpoint can have multiple connections over its lifetime
    //(including multiple outstanding at the same time). If a connection fails or disconnects and you want
	// to reconnect, just call endpoint::connect again
	//con.reset();
    // The ability to open a new connection
    client::connection_ptr con = m_endpoint.get_connection(uri, ec);
	m_currentConnection = con;
    if (ec) {
	  printf("websocket_endpoint::connect Connect initialization error ec.message() %s \n", ec.message());
      return -1;
    }
    /* If connection creation succeeds, the next sequential connection ID is generated and a 
 	 *connection_metadata object is inserted into the connection list under that ID. Initially the metadata object stores
     *the connection ID, the connection_hdl, and the URI the connection was opened to.
	 *Next, the connection request is configured. For this step the only configuration we will do is setting up a few default handlers.
	 *Later on we will return and demonstrate some more detailed configuration that can happen here
	 *(setting user agents, origin, proxies, custom headers, subprotocols, etc). */
    int new_id = m_next_id++;
    connection_metadata::ptr metadata_ptr(new connection_metadata(new_id, con->get_handle(), uri));
    m_connection_list[new_id] = metadata_ptr;
	  
	/* WebSocket++ provides a number of execution points where you can register to have a handler run.
	 * Which of these points are available to your endpoint will depend on its config.
	 * TLS handlers will not exist on non-TLS endpoints for example. A complete list of handlers can be
	 * found at http://www.zaphoyd.com/websocketpp/manual/reference/handler-list.
	 * Handlers can be registered at the endpoint level and at the connection level.
	 * Endpoint handlers are copied into new connections as they are created. Changing an endpoint handler
	 * will affect only future connections. Handlers registered at the connection level will be bound to
	 * that specific connection only.
	 * The signature of handler binding methods is the same for endpoints and connections.
	 * The format is: set_*_handler(...). Where * is the name of the handler. For example,
	 * set_open_handler(...) will set the handler to be called when a new connection is open.
	 * set_fail_handler(...) will set the handler to be called when a connection fails to connect.
	 * All handlers take one argument, a callable type that can be converted to a std::function with the
	 * correct count and type of arguments. You can pass free functions, functors, and Lambdas with
	 * matching argument lists as handlers. In addition, you can use std::bind (or boost::bind) to
	 * register functions with non-matching argument lists. This is useful for passing additional
	 * parameters not present in the handler signature or member functions that need to carry a
	 * 'this' pointer. The function signature of each handler can be looked up in the list above in
	 * the manual. In general, all handlers include the connection_hdl identifying which connection
	 * this even is associated with as the first parameter. Some handlers (such as the message handler)
	 * include additional parameters. Most handlers have a void return value but some
	 * (validate, ping, tls_init) do not. The specific meanings of the return values are documented
	 * in the handler list linked above.

     * utility_client registers an open and a fail handler. We will use these to track whether each connection
	 * was successfully opened or failed. If it successfully opens, we will gather some information from
	 * the opening handshake and store it with our connection metadata.
	 * In this example we are going to set connection specific handlers that are bound directly to
	 * the metadata object associated with our connection. This allows us to avoid performing a lookup
	 * in each handler to find the metadata object we plan to update which is a bit more efficient.
		
	 * &connection_metadata::on_open is the address of the on_open member function of the connection_metadata
	 * class. metadata_ptr is a pointer to the connection_metadata object associated with this class.
	 * It will be used as the object on which the on_open member function will be called. &m_endpoint
	 * is the address of the endpoint in use. This parameter will be passed as-is to the on_open method.
	 * Lastly, websocketpp::lib::placeholders::_1 is a placeholder indicating that the bound function
	 * should take one additional argument to be filled in at a later time. WebSocket++ will fill in this
	 * placeholder with the connection_hdl when it invokes the handler. */
    con->set_open_handler(websocketpp::lib::bind(
      &connection_metadata::on_open,
      metadata_ptr,
      &m_endpoint,
      websocketpp::lib::placeholders::_1 // TODO: update.
    ));
    con->set_fail_handler(websocketpp::lib::bind(
      &connection_metadata::on_fail,
      metadata_ptr,
      &m_endpoint,
      websocketpp::lib::placeholders::_1 // TODO: update.
    ));
	
	/* In order to have this handler called when new messages are received we also register it with
	 * our connection. Note that unlike most other handlers, the message handler has two parameters and
	 * thus needs two placeholders.  CSD pass in an argument which is a callback pointer to process the message */
    con->set_message_handler(websocketpp::lib::bind(
      &connection_metadata::on_message,
      metadata_ptr,
      websocketpp::lib::placeholders::_1,
      websocketpp::lib::placeholders::_2
    ));
	
    m_endpoint.connect(con);
	wait_connect(1);    //loop while con->get_state() == websocketpp::session::state::open
    return new_id;
  }  // end connect

  /* The ability to view information about a previously opened connection. Every connection
   * that gets opened will be assigned an integer connection id that the user of the program can use
   * to interact with that connection.
   * In order to track information about each connection a connection_metadata object is defined.
   * This object stores the numeric connection id and a number of fields that will be filled in as
   * the connection is processed. Initially this includes the state of the connection
   * (opening, open, failed, closed, etc), the original URI connected to, an identifying value
   * from the server, and a description of the reason for connection failure/closure.
   * Future steps will add more information to this metadata object.
   */
  connection_metadata::ptr websocket_endpoint::get_metadata(int id) {
    con_list::const_iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
		printf(" websocket_endpoint::get_metadata NOT FOUND id %d \n",id);
      return connection_metadata::ptr();
    } else {
      return metadata_it->second;
    }
  }  // End get_metadata
	
  void websocket_endpoint::close(int id, websocketpp::close::status::value code, std::string reason) {
    websocketpp::lib::error_code ec;
    
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
	  printf("websocket_endpoint::close No connection found with id %d \n",id);
      return;
    }
    
    m_endpoint.close(metadata_it->second->get_hdl(), code, "", ec);
    if (ec) {
	 printf("websocket_endpoint::close Error initiating close \n");
    }
	m_currentConnection.reset();
  }  // End close
  /* send a message on a given connection and updates the show command to print a transcript of
   * all sent and received messages for that connection WebSocket messages have types indicated by their
   * opcode. The protocol currently specifies two different opcodes for data messages, text and binary.
   * Text messages represent UTF8 text and will be validated as such. Binary messages represent raw binary
   * bytes and are passed through directly with no validation. WebSocket++ provides the values
   * websocketpp::frame::opcode::text and websocketpp::frame::opcode::binary that can be used to
   * direct how outgoing messages should be sent and to check how incoming messages are formatted.
   * Messages are sent using endpoint::send. This is a thread safe method that may be called from 
   * anywhere to queue a message for sending on the specified connection. There are three send overloads 
   * for use with different scenarios. 
   * Each method takes a connection_hdl to indicate which connection to send the message on as well
   * as a frame::opcode::value to indicate which opcode to label the message as. All overloads are also
   * available with an exception free varient that fills in a a status/error code instead of throwing.

   * The first overload, connection_hdl hdl, std::string const & payload, frame::opcode::value op,
   * takes a std::string. The string contents are copied into an internal buffer and can be safely
   * modified after calling send.

   * The second overload, connection_hdl hdl, void const * payload, size_t len, frame::opcode::value op,
   * takes a void * buffer and length. The buffer contents are copied and can be safely modified after
   * calling send.

   * The third overload, connection_hdl hdl, message_ptr msg, takes a WebSocket++ message_ptr.
   * This overload allows a message to be constructed in place before the call to send.
   * It also may allow a single message buffer to be sent multiple times, including to multiple connections,
   * without copying. Whether or not this actually happens depends on other factors such as whether
   * compression is enabled. The contents of the message buffer may not be safely modified after being sent.
   
   * In many configurations, such as when the Asio based transport is in use, WebSocket++ is an asynchronous
   * system. As such the endpoint::send method may return before any bytes are actually written to the
   * outgoing socket. In cases where send is called multiple times in quick succession messages may be
   * coalesced and sent in the same operation or even the same TCP packet. When this happens the message
   * boundaries are preserved (each call to send will produce a separate message).

   * In the case of applications that call send from inside a handler this means that no messages will
   * be written to the socket until that handler returns. If you are planning to send many messages in 
   * this manor or need a message to be written on the wire before continuing you should look into using
   * multiple threads or the built in timer/interrupt handler functionality.

   * If the outgoing socket link is slow messages may build up in this queue. You can use 
   * connection::get_buffered_amount to query the current size of the written message queue to
   * decide if you want to change your sending behavior.
   
   * Like the close method, send will start by looking up the given connection ID in the connection list.
   * Next a send request is sent to the connection's handle with the specified WebSocket message and the
   * text opcode. Finally, we record the sent message with our connection metadata object so later our
   * show connection command can print a list of messages sent.*/
  void websocket_endpoint::send(int id, std::string message) {
	printf("websocket_endpoint::send entered \n");
    websocketpp::lib::error_code ec;
    
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
	 printf(" websocket_endpoint::send No connection found with id %d \n",id);
        return;
    }
    
    m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
    if (ec) {
      printf(" websocket_endpoint::send Error sending message: ec.message() %s \n", ec.message());
      return;
    }
    
    metadata_it->second->record_sent_message(message);
  }  // End send
  
  void websocket_endpoint::wait_connect(int waitTime)
  {
    for (int i=0; i < 10; i++) {
      if (m_currentConnection)
      {
        if (m_currentConnection->get_state() == websocketpp::session::state::open) {
          break;
	    }
		sleep(waitTime);
      }
    }
  }