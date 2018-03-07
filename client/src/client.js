

/**
 * The Client class encapsulates all client-server communication.
 */ 
class Client {
  /**
   * The Client Constructor
   * @param {string} url The address:port of the server.
   */
  constructor() {
    this.socketBd = null;
    this.socketUt = null;
  }

  connect(url) {    
    const protocolPrefix = 'ws://';

    this.socketBd = new WebSocket(protocolPrefix + url, "data-binary-protocol");
    this.socketBd.binaryType = 'arraybuffer';
    this.socketBd.onopen     = this.onSocketBdOpen_.bind(this);
    this.socketBd.onclose    = this.onSocketBdClose_.bind(this);
    this.socketBd.onmessage  = this.onSocketBdMessage_.bind(this);
    this.socketBd.onerror    = this.onSocketBdError_.bind(this);

    this.socketUt = new WebSocket(protocolPrefix + url, "ui-text-protocol");
    this.socketUt.onopen    = this.onSocketUtOpen_.bind(this);
    this.socketUt.onclose   = this.onSocketUtClose_.bind(this);
    this.socketUt.onmessage = this.onSocketUtMessage_.bind(this);
    this.socketUt.onerror   = this.onSocketUtError_.bind(this);
  }

  disconnect() {
    // TODO: Implement this method.
  }

  log(message) {
    console.log('wst: ' + message);
  }

  sendData_(data) {

  }

  onSocketUtOpen_(event) {
    this.log(' UI-text WebSocket Connected!');
    // if (wst.txtInit != undefined) {
    //   wst.socketUt.send(wst.txtInit);
    // }
  }

  onSocketUtClose_(event) {
    this.log(' UI-text WebSocket Disconnected!');
    // wstServerDown();
  }

  onSocketUtMessage_(event) {
    //  wst.log(" UI-text WebSocket getMessage: " + evt.data);
    // wstServerTextMessage(evt.data);
  }

  onSocketUtError_(event) {
    this.log(' UI-text WebSocket Error: ' + event.data);
  }

  onSocketBdOpen_(event) {
    this.log(' Data-binary WebSocket Connected!');
  }

  onSocketBdClose_(event) {
    this.log(' Data-binary WebSocket Disconnected!');
  }

  onSocketBdMessage_(event) {

  }
  
  onSocketBdError_(event) {
    alert(' Not connected to Server: Try reloading the page!');
    this.log(' Data-binary WebSocket Error: ' + event.data);
  }
}

export default Client;