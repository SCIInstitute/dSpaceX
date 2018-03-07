

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
    this.socketBd.onopen     = this.onSocketBdOpen.bind(this);
    this.socketBd.onclose    = this.onSocketBdClose.bind(this);
    this.socketBd.onmessage  = this.onSocketBdMessage.bind(this);
    this.socketBd.onerror    = this.onSocketBdError.bind(this);

    this.socketUt = new WebSocket(protocolPrefix + url, "ui-text-protocol");
    this.socketUt.onopen    = this.onSocketUtOpen.bind(this);
    this.socketUt.onclose   = this.onSocketUtClose.bind(this);
    this.socketUt.onmessage = this.onSocketUtMessage.bind(this);
    this.socketUt.onerror   = this.onSocketUtError.bind(this);
  }

  disconnect() {
    // TODO: Implement this method.
  }

  log(message) {
    console.log('wst: ' + message);
  }

  sendData(data) {

  }

  onSocketUtOpen(event) {
    this.log(' UI-text WebSocket Connected!');
    // if (wst.txtInit != undefined) {
    //   wst.socketUt.send(wst.txtInit);
    // }
  }

  onSocketUtClose(event) {
    this.log(' UI-text WebSocket Disconnected!');
    // wstServerDown();
  }

  onSocketUtMessage(event) {
    //  wst.log(" UI-text WebSocket getMessage: " + evt.data);
    // wstServerTextMessage(evt.data);
  }

  onSocketUtError(event) {
    this.log(' UI-text WebSocket Error: ' + event.data);
  }

  onSocketBdOpen(event) {
    this.log(' Data-binary WebSocket Connected!');
  }

  onSocketBdClose(event) {
    this.log(' Data-binary WebSocket Disconnected!');
  }

  onSocketBdMessage(event) {

  }
  
  onSocketBdError(event) {
    alert(' Not connected to Server: Try reloading the page!');
    this.log(' Data-binary WebSocket Error: ' + event.data);
  }
}

export default Client;