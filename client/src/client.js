

/**
 * The Client class encapsulates all client-server communication.
 */ 
class Client {
  /**
   * The Client Constructor
   * @param {string} url The address:port of the server.
   */
  constructor() {
    // Binary Data Socket
    this.socketBd = null;

    // UI Text Socket
    this.socketUt = null;

    // Unique Message Index
    this.messageIndex = 0;

    this.initializeEventHandling_();
  }

  /**
   * This method uses the DOM Event Handling framework
   * to provide event handling for the Client class.
   */
  initializeEventHandling_() {
    // DOM EventTarget Object
    let target = document.createTextNode(null);
    
    // Pass EventTarget interface calls to DOM EventTarget object.
    this.addEventListener = target.addEventListener.bind(target);
    this.removeEventListener = target.removeEventListener.bind(target);
    this.dispatchEvent = target.dispatchEvent.bind(target);
  }

  dispatch(eventName) {
    let event = document.createEvent('Event');
    event.initEvent(eventName, true, true);
    this.dispatchEvent(event);
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

  maybeUpdateState() {
    if (this.socketBd.readyState === WebSocket.OPEN &&
        this.socketUt.readyState === WebSocket.OPEN) {
      this.dispatch('connected');
    } else {

    }
  }

  log(message) {
    console.log('wst: ' + message);
  }

  sendData_(data) {

  }

  /**
   * Grab a list of the available datasets from the server.
   */
  fetchDatasetList() {
    // TODO: Implement
  }

  /**
   * Grab data for the specified dataset
   * @param {string} datasetId
   */
  fetchDataset(datasetId) {
    // TODO: Implement
  }

  /**
   * Compute Morse-Smale Decomposition
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   */
  fetchMorseSmaleDecomposition(datasetId, k) {

  }


  onSocketUtOpen_(event) {
    this.log(' UI-text WebSocket Connected!');
    // if (wst.txtInit != undefined) {
    //   wst.socketUt.send(wst.txtInit);
    // }
    this.maybeUpdateState();
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
    this.dispatch('error');
  }

  onSocketBdOpen_(event) {
    this.log(' Data-binary WebSocket Connected!');
    this.maybeUpdateState();
  }

  onSocketBdClose_(event) {
    this.log(' Data-binary WebSocket Disconnected!');
  }

  onSocketBdMessage_(event) {

  }
  
  onSocketBdError_(event) {
    // alert(' Not connected to Server: Try reloading the page!');
    this.log(' Data-binary WebSocket Error: ' + event.data);
    this.dispatch('error');
  }
}

export default Client;