/**
 * The Client class encapsulates all client-server communication.
 */
class Client {
  /**
   * The Client Constructor
   */
  constructor() {
    // Binary Data Socket
    this.socketBd = null;

    // UI Text Socket
    this.socketUt = null;

    // Unique Message Index
    this.messageIndex = 0;

    this._initializeEventHandling();

    this.commandResponseMap = {};
  }

  /**
   * Increments the message index and returns the new value.
   * This method is responsible for ensuring all messages have
   * unique ids.
   * @return {number}
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

    this.socketBd = new WebSocket(protocolPrefix + url, 'data-binary-protocol');
    this.socketBd.binaryType = 'arraybuffer';
    this.socketBd.onopen = this._onSocketBdOpen.bind(this);
    this.socketBd.onclose = this._onSocketBdClose.bind(this);
    this.socketBd.onmessage = this._onSocketBdMessage.bind(this);
    this.socketBd.onerror = this._onSocketBdError.bind(this);

    this.socketUt = new WebSocket(protocolPrefix + url, 'ui-text-protocol');
    this.socketUt.onopen = this._onSocketUtOpen.bind(this);
    this.socketUt.onclose = this._onSocketUtClose.bind(this);
    this.socketUt.onmessage = this._onSocketUtMessage.bind(this);
    this.socketUt.onerror= this._onSocketUtError.bind(this);
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
   * @param {string} message
   */
  _log(message) {
    console.log('wst: ' + message);
  }

  /**
   * Send the requested command.
   * @param {object} command
   * @param {function} callback
   */
  _sendCommand(command, callback) {
    command.id = this._newMessageId();
    this.commandResponseMap[command.id] = callback;
    this.socketUt.send(JSON.stringify(command));
  }

  /**
   * Wrap the command request in a promise so that the client can
   * be responsible for resolving or rejecting the response to the
   * calling context.
   * @param {object} command
   * @return {Promise}
   */
  _createCommandPromise(command) {
    return new Promise(function(resolve, reject) {
      this._dispatch('networkActive');
      this._sendCommand(command, function(response, error) {
        if (error) {
          reject(error);
        } else {
          resolve(response);
        }
        this._dispatch('networkInactive');
      }.bind(this));
    }.bind(this));
  }

  /**
   * Grab a list of the available datasets from the server.
   * @return {Promise}
   */
  fetchDatasetList() {
    let command = {
      name: 'fetchDatasetList',
    };
    return this._createCommandPromise(command);
  }

  /**
   * Grab data for the specified dataset.
   * @param {string} datasetId
   * @return {Promise}
   */
  fetchDataset(datasetId) {
    let command = {
      name: 'fetchDataset',
      datasetId: datasetId,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Grab the k-nearest neighbor adjacency for datasetId.
   * @param {string} datasetId
   * @param {number} k
   * @return {Promise}
   */
  fetchKNeighbors(datasetId, k) {
    let command = {
      name: 'fetchKNeighbors',
      datasetId: datasetId,
      k: k,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Compute Morse-Smale Decomposition
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   * @return {Promise}
   */
  fetchMorseSmaleDecomposition(datasetId, k) {
    let command = {
      name: 'fetchMorseSmaleDecomposition',
      datasetId: datasetId,
      k: k,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Compute Morse-Smale Decomposition, return
   * only the persistence range (i.e. min and max).
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   * @return {Promise}
   */
  fetchMorseSmalePersistence(datasetId, k) {
    console.log('creating promise to call fetchMorseSmalePersistence');
    let command = {
      name: 'fetchMorseSmalePersistence',
      datasetId: datasetId,
      k: k,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Fetch the crystal indexes composing a single morse smale persistence level.
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   * @param {number} persistenceLevel
   * @return {Promise}
   */
  fetchMorseSmalePersistenceLevel(datasetId, k, persistenceLevel) {
    let command = {
      name: 'fetchMorseSmalePersistenceLevel',
      datasetId: datasetId,
      k: k,
      persistenceLevel: persistenceLevel,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Fetch the details of a single crystal of a persistence level.
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   * @param {number} persistenceLevel
   * @param {number} crystalId
   * @return {Promise}
   */
  fetchMorseSmaleCrystal(datasetId, k, persistenceLevel, crystalId) {
    let command = {
      name: 'fetchMorseSmaleCrystal',
      datasetId: datasetId,
      k: k,
      persistenceLevel: persistenceLevel,
      crystalId: crystalId,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Fetch the details of a set of crystals of a persistence level.
   * @param {string} datasetId
   * @param {number} k number of neighbors.
   * @param {number} persistenceLevel
   * @param {array} crystalIds
   * @return {Promise}
   */
  fetchMorseSmaleCrystals(datasetId, k, persistenceLevel, crystalIds) {
    let command = {
      name: 'fetchMorseSmaleCrystals',
      datasetId: datasetId,
      k: k,
      persistenceLevel: persistenceLevel,
      crystalIds: crystalIds,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Grab the graph embedding.
   * @param {string} datasetId
   * @param {number} k
   * @param {number} persistenceLevel
   * @param {string} qoiName
   * @return {Promise}
   */
  fetchGraphEmbedding(datasetId, k, persistenceLevel, qoiName) {
    let command = {
      name: 'fetchGraphEmbedding',
      datasetId: datasetId,
      k: k,
      persistenceLevel: persistenceLevel,
      qoiName: qoiName,
    };
    return this._createCommandPromise(command);
  }

    //
    // NEW API for GaussianGP latent_space function should get passed: datasetId
    // - it loaded the dataset separately, so the id is enough
    // - it will return a new LatentSpace object
    // - this object can give us data to display (the 2d data shown in Wei's paper) the latent space at some point therein (could be an 8-dimensional coordinate)
    // - it also provides methods to generate new (virtual) data (domain params, shape, and QOI) based on a *desired* coordinate in the latent_space (start w/ 2d, but could go up to 8d)
    // - at a higher level, we will want to be able to combine or mute this "virtual" data produced by the latent space
    // - not certain, but the latent space returns a distribution so it quantifies the uncertainty in its guesses and so it might not return the exact points (D, S, Q) as it learned from
    // - for our visualizations of this data, we need some way to show the confidence of the guesses in the latent space
    // - the current application can pass D,S,Q to the library, and the lib can print out its results. Same things can be compared using the matlab and the new library 
  /**
   * Fetch the Shared Latent Space (SharedGP) (for the specified QOI of the current dataset?)
   * @param {string} datasetId
   * @param {string} QOI
   * @return {Promise}
   */
  fetchSharedLatentSpace(datasetId, qoi) {
    let command = {
      name: 'fetchSharedLatentSpace',
      datasetId: datasetId,
      qoi: qoi,
    };
    return this._createCommandPromise(command);
  }

  fetchImageForLatentSpaceUsingShapeOdds(datasetId, qoi) {
    let command = {
      name: 'fetchImageForLatentSpaceUsingShapeOdds',
      datasetId: datasetId,
      qoi: qoi,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Fetch the Morse-Smale regression points for the specified persistence level
   * @param {string} datasetId
   * @param {number} k nearest neighbors
   * @param {number} persistenceLevel
   * @return {Promise}
   */
  fetchMorseSmaleRegression(datasetId, k, persistenceLevel) {
    let command = {
      name: 'fetchMorseSmaleRegression',
      datasetId: datasetId,
      k: k,
      persistenceLevel: persistenceLevel,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Fetch the Morse-Smale extrema points for the specified persistence level
   * @param {string} datasetId
   * @param {number} k
   * @param {number} persistenceLevel
   * @return {Promise}
   */
  fetchMorseSmaleExtrema(datasetId, k, persistenceLevel) {
    let command = {
      name: 'fetchMorseSmaleExtrema',
      datasetId: datasetId,
      k: k,
      persistenceLevel: persistenceLevel,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Grab the parameter values for the given parameter
   * @param {string} datasetId
   * @param {string} parameterName
   * @return {Promise}
   */
  fetchParameter(datasetId, parameterName) {
    let command = {
      name: 'fetchParameter',
      datasetId: datasetId,
      parameterName: parameterName,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Grab the qoi values for the given qoi.
   * @param {string} datasetId
   * @param {string} qoiName
   * @return {Promise}
   */
  fetchQoi(datasetId, qoiName) {
    let command = {
      name: 'fetchQoi',
      datasetId: datasetId,
      qoiName: qoiName,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Grab the thumbnails for the given dataset.
   * @param {string} datasetId
   * @return {Promise}
   */
  fetchThumbnails(datasetId) {
    let command = {
      name: 'fetchThumbnails',
      datasetId: datasetId,
    };
    return this._createCommandPromise(command);
  }

  /**
   * Text Socket onOpen event callback.
   * @param {Event} event
   */
  _onSocketUtOpen(event) {
    this._log(' Text WebSocket Connected!');
    this._maybeUpdateState();
  }

  /**
   * Text Socket onClose event callback.
   * @param {Event} event
   */
  _onSocketUtClose(event) {
    this._log(' Text WebSocket Disconnected!');
    this._dispatch('disconnected');
    this._maybeUpdateState();
  }

  /**
   * Text Socket onMessage event callback.
   * @param {Event} event
   */
  _onSocketUtMessage(event) {
    let response = JSON.parse(event.data);
    this.commandResponseMap[response.id](response);
    delete this.commandResponseMap[response.id];
  }

  /**
   * Text Socket onError event callback.
   * @param {Event} event
   */
  _onSocketUtError(event) {
    this._log(' Text WebSocket Error: ' + event.data);
    this._dispatch('error');
  }

  /**
   * Binary Socket onOpen event callback.
   * @param {Event} event
   */
  _onSocketBdOpen(event) {
    this._log(' Binary WebSocket Connected!');
    this._maybeUpdateState();
  }

  /**
   * Binary Socket onClose event callback.
   * @param {Event} event
   */
  _onSocketBdClose(event) {
    this._log(' Binary WebSocket Disconnected!');
  }

  /**
   * Binary Socket onMessage event callback.
   * @param {Event} event
   */
  _onSocketBdMessage(event) {

  }

  /**
   * Binary Socket onError event callback.
   * @param {Event} event
   */
  _onSocketBdError(event) {
    // alert(' Not connected to Server: Try reloading the page!');
    this._log(' Binary WebSocket Error: ' + event.data);
    this._dispatch('error');
  }
}

export default Client;
