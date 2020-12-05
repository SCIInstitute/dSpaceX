import * as d3 from 'd3';
import Button from '@material-ui/core/Button';
import Client from './data/client.js';
import ConnectionDialog from './connectionDialog.js';
import { DSXProvider } from './dsxContext.js';
import DataHelper from './data/dataHelper.js';
import DatasetPanel from './panels/datasetPanel.js';
import Drawer from '@material-ui/core/Drawer';
import EmbeddingMorseSmaleWindow from './windows/embeddingMorseSmaleWindow';
import EmptyWindow from './windows/emptyWindow.js';
import ErrorDialog from './errorDialog.js';
import FilterPanel from './panels/filterPanel';
import GalleryWindow from './windows/galleryWindow';
import PropTypes from 'prop-types';
import React from 'react';
import ScatterPlotWindow from './windows/scatterPlotWindow';
import TableWindow from './windows/tableWindow.js';
import Toolbar from './toolbar.js';
import WindowPanel from './panels/windowPanel.js';
import Workspace from './workspace.js';
import { withStyles } from '@material-ui/core/styles';


const drawerWidth = 260;
const styles = (theme) => ({
  root: {
    flexGrow: 1,
    zIndex: 1,
    overflow: 'hidden',
    position: 'relative',
    display: 'flex',
    height: 'calc(100vh)',
  },
  drawerPaper: {
    position: 'relative',
    width: drawerWidth,
    overflowX: 'hidden',
  },
  content: {
    flexGrow: 1,
    minWidth: 0,
    backgroundColor: '#eef',
    position: 'relative',
  },
  workspace: {
    display: 'grid',
    height: 'calc(100vh - 64px)',
    gridTemplateColumns: '1fr',
    gridTemplateRows: '1fr',
    gridGap: '0em',
  },
  toolbar: theme.mixins.toolbar,
});

/**
 * The top level dSpaceX client component.
 */
class Application extends React.Component {
  /**
   * Application constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.state = {
      connected: false,
      networkActive: false,
      currentDataset: null,
      displayFilterDrawer: false,
      datasets: [],
      windows: [],
      filters: [],
      selectedDesigns: new Set(),
      parameters: [],
      qois: [],
      distanceMetrics: [],
      currentMetric: null,
    };

    this.connectButtonClicked = this.connectButtonClicked.bind(this);
    this.onConnect = this.onConnect.bind(this);
    this.onDisconnect = this.onDisconnect.bind(this);
    this.onNetworkActivityStart = this.onNetworkActivityStart.bind(this);
    this.onNetworkActivityEnd = this.onNetworkActivityEnd.bind(this);
    this.onDatasetChange = this.onDatasetChange.bind(this);
    this.addWindow = this.addWindow.bind(this);
    this.onWindowConfigChange = this.onWindowConfigChange.bind(this);
    this.onDesignSelection = this.onDesignSelection.bind(this);
    this.onDesignLasso = this.onDesignLasso.bind(this);
    this.onDisplayFilterDrawer = this.onDisplayFilterDrawer.bind(this);
    this.onCrystalSelection = this.onCrystalSelection.bind(this);
    this.changeFilterOperation = this.changeFilterOperation.bind(this);
    this.onAddFilter = this.onAddFilter.bind(this);
    this.onUpdateFilter = this.onUpdateFilter.bind(this);
    this.onRemoveFilter = this.onRemoveFilter.bind(this);

    this.client = new Client();
    this.client.addEventListener('connected', this.onConnect);
    this.client.addEventListener('disconnected', this.onDisconnect);
    this.client.addEventListener('networkActive', this.onNetworkActivityStart);
    this.client.addEventListener('networkInactive', this.onNetworkActivityEnd);
    // export client for debugging
    window.client = this.client;

    // TODO Fix this! This is a hack to get d3-lasso working in the scatterPlotWindow
    window.d3=d3;

    // TODO finish data helper so data is managed in one place
    this.dataHelper = new DataHelper(this.client);
  }

  /**
   * Callback invoked immediately after the component is mounted.
   */
  componentDidMount() {
    console.log('Application Ready.');
  }

  /**
   * Handles the connect button being clicked.
   */
  connectButtonClicked() {
    this.refs.connectiondialog.open();
  }

  /**
   * Handles the application connecting to the remote server.
   */
  onConnect() {
    this.client.fetchDatasetList().then(function(response) {
      this.setState({
        connected: true,
        datasets: response.datasets,
      });
    }.bind(this));
  }

  /**
   * Handles the application disconnecting from the remote server.
   */
  onDisconnect() {
    this.refs.errorDialog.reportError('Unable to communicate with server.');
    this.setState({
      connected: false,
      networkActive: false,
    });
  }

  /**
   * Handles when the applications starts communicating with the server.
   */
  onNetworkActivityStart() {
    this.setState({
      networkActive: true,
    });
  }

  /**
   * Handles when the applications stops communicating with the server.
   */
  onNetworkActivityEnd() {
    this.setState({
      networkActive: false,
    });
  }

  /**
   * Handles updating dataflow to components when dataset changes.
   * @param {object} dataset
   */
  onDatasetChange(dataset) {
    const { datasetId, parameterNames, qoiNames, distanceMetrics } = dataset;

    // create a map of distanceMetrics and their fields and their fields' models
    let distancesMap = new Map();
    for (let d=0; d < distanceMetrics.length; d++) {
      let metric = distanceMetrics[d].name;
      let fields = distanceMetrics[d].fields;

      let fieldModels = new Map();
      for (let i=0; i < fields.length; i++) {
        let name = fields[i].name;
        let models = [];
        for (let m=0; m < fields[i].models.length; m++) {
          models.push(fields[i].models[m]);
        }
        fieldModels.set(name, models);
      }
      distancesMap.set(metric, fieldModels);
    }
    let currentMetric = distancesMap.keys().next().value;

    Promise.all([
      this.getParameters(datasetId, parameterNames),
      this.getQois(datasetId, qoiNames),
      this.client.fetchEmbeddingsList(datasetId, currentMetric),
    ]).then((results) => {
      const [parameters, qois, embeddingList] = results;
      this.setState({
        currentDataset: dataset,
        windows: [],
        selectedDesigns: new Set(),
        union: true,
        filters: [],
        parameters: parameters,
        qois: qois,
        distanceMetrics: distancesMap,
        embeddings: embeddingList.embeddings,
      });
    });
  }

  /**
   * Handles adding a new dataview window.
   */
  addWindow() {
    let windowConfig = {
      id: this.state.windows.length,
    };
    this.setState({
      windows: this.state.windows.concat(windowConfig),
    });
  }

  /**
   * Handles the user changing the configuration for a window in the workspace
   * by broadcasting that change to the other windows.
   * @param {object} config
   */
  onWindowConfigChange(config) {
    let prevConfig = this.state.windows[config.id];
    let newConfig = Object.assign(prevConfig, config);
    let windows = this.state.windows;
    windows[config.id] = newConfig;
    this.setState({
      windows: windows,
    });
  }

  /**
   * Handles selection of a design from any window
   * @param { object } event
   * @param { number } id
   */
  onDesignSelection(event, id) {
    event.stopPropagation();

    // append this design to current selection if holding ctrl/meta
    if (event.ctrlKey || event.metaKey) { // Works for mac and linux - need to test windows
      let selectedDesigns = new Set(this.state.selectedDesigns);
      selectedDesigns.add(id);
      this.setState({ selectedDesigns });
    } else { 
      let selectedDesigns = new Set();
      if (id !== -1) {
        selectedDesigns.add(id);
      }
      this.setState({ selectedDesigns });
    }
  }
  
  /**
   * Handles lasso of design from ScatterPlotWindow
   * @param {Array<object>} selection selected designs
   */
  onDesignLasso(selection) {
    let selectedDesigns = new Set();
    selection.each((s) => { selectedDesigns.add(s.id); });
    this.setState({ selectedDesigns });
  }

  /**
   * Handles updating of selected samples based on crystal selection
   * @param {Array<int>} crystalSamples
   */
  onCrystalSelection(crystalSamples) {
    let selectedDesigns = new Set(crystalSamples);
    this.setState({ selectedDesigns });
  }

  /**
   * Open and closes filter drawer when 'Filter' button is clicked
   */
  onDisplayFilterDrawer() {
    this.setState({
      displayFilterDrawer: !this.state.displayFilterDrawer,
    });
  }

  /**
   * Change filter operation from union to intersection
   * and vice versa.
   */
  changeFilterOperation() {
    this.setState({
      union: !this.state.union,
    });
  }

  /**
   * Handles when a new filter is added by selecting the '+' icon
   * @param {int} id
   */
  onAddFilter(id) {
    let filters = [...this.state.filters];
    const filter = {
      'id': id,
      'enabled': false,
      'min': 0,
      'max': Infinity,
      'attributeGroup': '',
      'attribute': '',
      'numberOfBins': 10,
    };
    filters.push(filter);
    this.setState({ filters });
  }

  /**
   * Updates the filter in the filters array
   * @param {object} filterConfig
   */
  onUpdateFilter(filterConfig) {
    let filters = [...this.state.filters];
    let index = filters.findIndex((f) => f.id === filterConfig.id);
    filters[index] = filterConfig;
    this.setState({ filters });
  }

  /**
   * Removes the filter from the filters array
   * @param {int} id
   */
  onRemoveFilter(id) {
    let filters = [...this.state.filters];
    filters = filters.filter((f) => f.id !== id);
    this.setState({ filters });
  }

  /**
   * Gets the parameters for the current data set
   * @param {string} datasetId
   * @param {Array<string>} parameterNames
   * @return {Promise<Array>}
   */
  async getParameters(datasetId, parameterNames) {
    let parameters = [];
    parameterNames.forEach(async (parameterName) => {
      let parameter =
          await this.client.fetchParameter(datasetId, parameterName);
      parameters.push(parameter);
    });
    return parameters;
  }

  /**
   * Gets the qois for the current data set
   * @param {string} datasetId
   * @param {Array<string>} qoiNames
   * @return {Promise<Array>}
   */
  async getQois(datasetId, qoiNames) {
    let qois = [];
    qoiNames.forEach(async (qoiName) => {
      let qoi =
          await this.client.fetchQoi(datasetId, qoiName);
      qois.push(qoi);
    });
    return qois;
  }

  /**
   * Gets the samples that should be displayed after the filters
   * are applied
   * @return {Set} indexes of images that should be visible
   */
  getActiveDesigns() {
    if (this.state.currentDataset === null) {
      return new Set();
    }

    const { numberOfSamples } = this.state.currentDataset;
    const { filters, union } = this.state;
    if (filters === undefined || filters.filter((f) => f.enabled) < 1) {
      return new Set([...Array(numberOfSamples).keys()]);
    } else if (union) {
      return this.getUnion();
    } else {
      return this.getIntersection();
    }
  }

  /**
   * Get union of designs from filter
   * @return {Set<any>}
   */
  getUnion() {
    const { filters, parameters, qois } = this.state;
    let activeDesigns = new Set();
    const enabledFilters = filters.filter((f) => f.enabled);
    enabledFilters.forEach((f) => {
      if (f.attributeGroup === 'parameters') {
        let params = parameters.filter((p) => p.parameterName === f.attribute)[0].parameter;
        let visibleParams = params.filter((p) => p >= f.min && p <= f.max);
        visibleParams.forEach((value) => {
          let indexes = this.findAllIndexes(params, value);
          indexes.forEach((index) => activeDesigns.add(index));
        });
      } else if (f.attributeGroup === 'qois') {
        let filteredQois = qois.filter((q) => q.qoiName === f.attribute)[0].qoi;
        let visibleQois = filteredQois.filter((q) => q >= f.min && q <= f.max);
        visibleQois.forEach((value) => {
          let indexes = this.findAllIndexes(filteredQois, value);
          indexes.forEach((index) => activeDesigns.add(index));
        });
      }
    });
    return activeDesigns;
  }

  /**
   * Get intersection of design from filters.
   * @return {Set<any>}
   */
  getIntersection() {
    const { filters, parameters, qois } = this.state;
    const { numberOfSamples } = this.state.currentDataset;
    let activeDesigns = new Set([...Array(numberOfSamples).keys()]);
    const enabledFilters = filters.filter((f) => f.enabled);
    enabledFilters.forEach((f) => {
      if (f.attributeGroup === 'parameters') {
        let params = parameters.filter((p) => p.parameterName === f.attribute)[0].parameter;
        let visibleParams = params.filter((p) => p >= f.min && p <= f.max);
        let newSet = new Set();
        visibleParams.forEach((value) => {
          let indexes = this.findAllIndexes(params, value);
          indexes.forEach((index) => newSet.add(index));
        });
        activeDesigns = new Set([...activeDesigns].filter((x) => newSet.has(x)));
      } else if (f.attributeGroup === 'qois') {
        let filteredQois = qois.filter((q) => q.qoiName === f.attribute)[0].qoi;
        let visibleQois = filteredQois.filter((q) => q >= f.min && q <= f.max);
        let newSet = new Set();
        visibleQois.forEach((value) => {
          let indexes = this.findAllIndexes(filteredQois, value);
          indexes.forEach((index) => newSet.add(index));
        });
        activeDesigns = new Set([...activeDesigns].filter((x) => newSet.has(x)));
      }
    });
    return activeDesigns;
  }

  /**
   * Finds all indexes that match a given value
   * @param {array} array
   * @param {number} value
   * @return {array} of indexes
   */
  findAllIndexes(array, value) {
    let indexes = [];
    let i = -1;
    while ((i = array.indexOf(value, i+1)) !== -1) {
      indexes.push(i);
    }
    return indexes;
  }

  /**
   * Renders the component to HTML.
   * @return {JSX}  //cool half-html, half-javascript thing, gnarly and yet helpful
   */
  render() {
    const { classes } = this.props;
    const activeDesigns = this.getActiveDesigns();
    let drawerMarginColor = this.state.connected ? '#fff' : '#ddd';
    return (
      <DSXProvider value={{ client:this.client }}>
        <div className={classes.root}>
          <ConnectionDialog ref='connectiondialog' client={this.client}/>
          <Toolbar connectedToServer={this.state.connected}
            dataset={this.state.currentDataset}
            onConnectClick={this.connectButtonClicked}
            networkActive={this.state.networkActive}
            onDisplayFilterDrawer={this.onDisplayFilterDrawer}
            displayFilterDrawer={this.state.displayFilterDrawer}
            filtersEnabled={this.state.filters.filter((f) => f.enabled).length > 0}/>
          <Drawer PaperProps={{ elevation:6 }} variant='permanent'
            classes={{ paper:classes.drawerPaper }}>
            { /* Add div to account for menu bar */ }
            <div className={classes.toolbar} />
            <DatasetPanel
              enabled={this.state.connected}
              datasets={this.state.datasets}
              onDatasetChange={this.onDatasetChange}/>
            {
              !!this.state.currentDataset ?
                this.state.windows.map((windowConfig, i) => {
                  return (
                    <WindowPanel key={i} windowIndex={i}
                      config={windowConfig}
                      onConfigChange={this.onWindowConfigChange}
                      dataset={this.state.currentDataset}
                      enabled={this.state.connected}
                      distanceMetrics={this.state.distanceMetrics}
                      embeddings={this.state.embeddings}/>
                  );
                }) : []
            }
            {
              (!!this.state.currentDataset && this.state.windows.length < 4) ?
                <Button color="primary" className={classes.button}
                  onClick={this.addWindow}>
                  Add Window
                </Button>
                : []
            }
            <div style={{
              backgroundColor: drawerMarginColor,
              height: '100%',
              width: '100%',
            }}/>
          </Drawer>
          <Workspace className={classes.content}>
            { /* Add div to account for menu bar */ }
            <div className={classes.toolbar}/>
            {this.state.displayFilterDrawer && <FilterPanel parameters={this.state.parameters}
              qois={this.state.qois}
              union={this.state.union}
              changeFilterOperation={this.changeFilterOperation}
              filters={this.state.filters}
              addFilter={this.onAddFilter}
              updateFilter={this.onUpdateFilter}
              removeFilter={this.onRemoveFilter}/>}
            <div className={classes.workspace} style={
              (() => {
                switch (this.state.windows.length) {
                  case 1:
                    return {
                      gridTemplateColumns: '1fr',
                      gridTemplateRows: '1fr',
                    };
                  case 2:
                    return {
                      gridTemplateColumns: '1fr 1fr',
                      gridTemplateRows: '1fr',
                    };
                  case 3:
                  case 4:
                    return {
                      gridTemplateColumns: '1fr 1fr',
                      gridTemplateRows: '1fr 1fr',
                    };
                }
              })()
            }>
              {
                !!this.state.currentDataset ?
                  this.state.windows.map((windowConfig, i) => {
                    if (windowConfig.dataViewType === 'table') {
                      return (
                        <TableWindow key={i}
                          attributeGroup={windowConfig.tableAttributeGroup}
                          dataset={this.state.currentDataset}
                          selectedDesigns={this.state.selectedDesigns}
                          onDesignSelection={this.onDesignSelection}
                          activeDesigns={activeDesigns}/>
                      );
                    } else if (windowConfig.dataViewType === 'graph') {
                      return (
                        <EmbeddingMorseSmaleWindow
                          key={i}
                          decomposition={windowConfig.decomposition}
                          embedding={this.state.embeddings.filter(
                            (emb) => emb.name === windowConfig.embeddingAlgorithm)[0]}
                          dataset={this.state.currentDataset}
                          selectedDesigns={this.state.selectedDesigns}
                          numberOfWindows={this.state.windows.length}
                          onDesignSelection={this.onDesignSelection}
                          onCrystalSelection={this.onCrystalSelection}
                          activeDesigns={activeDesigns}
                          screenWidthHack={this.state.windows.length>1}/>
                      );
                    } else if (windowConfig.dataViewType === 'scatter_plot') {
                      return (
                        <ScatterPlotWindow key={i}
                          config={windowConfig}
                          dataset={this.state.currentDataset}
                          selectedDesigns={this.state.selectedDesigns}
                          onDesignSelection={this.onDesignSelection}
                          onDesignLasso={this.onDesignLasso}
                          activeDesigns={activeDesigns}
                        />
                      );
                    } else if (windowConfig.dataViewType === 'gallery') {
                      return (
                        <GalleryWindow key={i}
                          dataset={this.state.currentDataset}
                          selectedDesigns={this.state.selectedDesigns}
                          onDesignSelection={this.onDesignSelection}
                          activeDesigns={activeDesigns}/>);
                    } else {
                      return (
                        <EmptyWindow key={i} id={i}/>
                      );
                    }
                  }) :
                  []
              }
            </div>
          </Workspace>
          <ErrorDialog ref='errorDialog' />
        </div>
      </DSXProvider>
    );
  }
}

// Enforce that Application receives styling.
Application.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withStyles(styles)(Application);
