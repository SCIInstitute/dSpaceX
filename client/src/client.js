

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

    this._initializeEventHandling();
  }

  /**
   * Increments the message index and returns the new value.
   * This method is responsible for ensuring all messages have 
   * unique ids.
   */
  _newMessageId() {
    this.messageIndex += 1;
    return this.messageIndex;
  }

  /**
   * This method uses the DOM Event Handling framework
   * to provide event handling for the Client class.
   */
  _initializeEventHandling() {
    // DOM EventTarget Object
    let target = document.createTextNode(null);

    // Pass EventTarget interface calls to DOM EventTarget object.
    this.addEventListener = target.addEventListener.bind(target);
    this.removeEventListener = target.removeEventListener.bind(target);
    this.dispatchEvent = target.dispatchEvent.bind(target);
  }

  /**
   * Constructs and dispatches an event with the eventName provided.
   * @param {string} eventName
   */
  _dispatch(eventName) {
    let event = document.createEvent('Event');
    event.initEvent(eventName, true, true);
    this.dispatchEvent(event);
  }

  /**
   * Connects all sockets to the desired host server.
   * @param {string} url
   */
  connect(url) {
    const protocolPrefix = 'ws://';

    this.socketBd = new WebSocket(protocolPrefix + url, "data-binary-protocol");
    this.socketBd.binaryType = 'arraybuffer';
    this.socketBd.onopen     = this._onSocketBdOpen.bind(this);
    this.socketBd.onclose    = this._onSocketBdClose.bind(this);
    this.socketBd.onmessage  = this._onSocketBdMessage.bind(this);
    this.socketBd.onerror    = this._onSocketBdError.bind(this);

    this.socketUt = new WebSocket(protocolPrefix + url, "ui-text-protocol");
    this.socketUt.onopen    = this._onSocketUtOpen.bind(this);
    this.socketUt.onclose   = this._onSocketUtClose.bind(this);
    this.socketUt.onmessage = this._onSocketUtMessage.bind(this);
    this.socketUt.onerror   = this._onSocketUtError.bind(this);
  }

  /**
   * Disconnect all websockets from the host server.
   */
  disconnect() {
    // TODO: Implement this method.
  }

  /**
   * Callback to handle potential state changes based on socket events.
   */
  _maybeUpdateState() {
    if (this.socketBd.readyState === WebSocket.OPEN &&
        this.socketUt.readyState === WebSocket.OPEN) {
      this._dispatch('connected');
    } else {

    }
  }

  /**
   * Logging wrapper.
   */
  _log(message) {
    console.log('wst: ' + message);
  }

  /**
   *
   */
  _sendData(data) {

  }

  /**
   * Grab a list of the available datasets from the server.
   */
  fetchDatasetList() {
    let command = {
      name: 'fetchDatasetList',
      id: this._newMessageId(),
    }
    this.socketUt.send(JSON.stringify(command));
  }

  /**
   * Grab data for the specified dataset
   * @param {string} datasetId
   */
  fetchDataset(datasetId) {
    let command = {
      name: 'fetchDataset',
      id: this._newMessageId(),
      datasetId: datasetId
    }
    this.socketUt.send(JSON.stringify(command));
  }

  /**
   * Compute Morse-Smale Decomposition
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   */
  fetchMorseSmaleDecomposition(datasetId, k) {

  }

  /**
   * Text Socket onOpen event callback.
   * @param {Event} event.
   */
  _onSocketUtOpen(event) {
    this._log(' UI-text WebSocket Connected!');
    // if (wst.txtInit != undefined) {
    //   wst.socketUt.send(wst.txtInit);
    // }
    this._maybeUpdateState();
  }

  /**
   * Text Socket onClose event callback.
   * @param {Event} event.
   */
  _onSocketUtClose(event) {
    this._log(' UI-text WebSocket Disconnected!');
    // wstServerDown();
  }

  /**
   * Text Socket onMessage event callback.
   * @param {Event} event.
   */
  _onSocketUtMessage(event) {
    //  wst.log(" UI-text WebSocket getMessage: " + evt.data);
    // wstServerTextMessage(evt.data);
  }

  /**
   * Text Socket onError event callback.
   * @param {Event} event.
   */
  _onSocketUtError(event) {
    this._log(' UI-text WebSocket Error: ' + event.data);
    this._dispatch('error');
  }

  /**
   * Binary Socket onOpen event callback.
   * @param {Event} event.
   */
  _onSocketBdOpen(event) {
    this._log(' Data-binary WebSocket Connected!');
    this._maybeUpdateState();
  }

  /**
   * Binary Socket onClose event callback.
   * @param {Event} event.
   */
  _onSocketBdClose(event) {
    this._log(' Data-binary WebSocket Disconnected!');
  }

  /**
   * Binary Socket onMessage event callback.
   * @param {Event} event.
   */
  _onSocketBdMessage(event) {

  }

  /**
   * Binary Socket onError event callback.
   * @param {Event} event.
   */
  _onSocketBdError(event) {
    // alert(' Not connected to Server: Try reloading the page!');
    this._log(' Data-binary WebSocket Error: ' + event.data);
    this._dispatch('error');
  }
}

export default Client;