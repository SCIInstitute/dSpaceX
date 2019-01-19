import Button from '@material-ui/core/Button';
import Client from './client.js';
import ConnectionDialog from './connectionDialog.js';
import { DSXProvider } from './dsxContext.js';
import DatasetPanel from './panels/datasetPanel.js';
import Drawer from '@material-ui/core/Drawer';
import EmptyWindow from './windows/emptyWindow.js';
import ErrorDialog from './errorDialog.js';
import GraphGLWindow from './windows/graphGLWindow.js';
import PropTypes from 'prop-types';
import React from 'react';
import ScatterPlotWindow from './windows/scatterPlotWindow';
import TableWindow from './windows/tableWindow.js';
import ThumbnailWindow from './windows/thumbnailWindow'
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
      datasets: [],
      windows: [],
    };

    this.connectButtonClicked = this.connectButtonClicked.bind(this);
    this.onConnect = this.onConnect.bind(this);
    this.onDisconnect = this.onDisconnect.bind(this);
    this.onNetworkActivityStart = this.onNetworkActivityStart.bind(this);
    this.onNetworkActivityEnd = this.onNetworkActivityEnd.bind(this);
    this.onDatasetChange = this.onDatasetChange.bind(this);
    this.addWindow = this.addWindow.bind(this);
    this.onWindowConfigChange = this.onWindowConfigChange.bind(this);

    this.client = new Client();
    this.client.addEventListener('connected', this.onConnect);
    this.client.addEventListener('disconnected', this.onDisconnect);
    this.client.addEventListener('networkActive', this.onNetworkActivityStart);
    this.client.addEventListener('networkInactive', this.onNetworkActivityEnd);
    // export client for debugging
    window.client = this.client;
  }

  /**
   * Callback invoked before the component receives new props.
   */
  componentWillMount() {
    console.log('Initializing Application...');
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
    this.setState({
      currentDataset: dataset,
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
   * Handles the user changing the configuration for a window.
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
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    let drawerMarginColor = this.state.connected ? '#fff' : '#ddd';
    return (
      <DSXProvider value={{ client:this.client }}>
        <div className={classes.root}>
          <ConnectionDialog ref='connectiondialog' client={this.client}/>
          <Toolbar connectedToServer={this.state.connected}
            onConnectClick={this.connectButtonClicked}
            networkActive={this.state.networkActive} />
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
                      enabled={this.state.connected}/>
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
            }}></div>
          </Drawer>
          <Workspace className={classes.content}>
            { /* Add div to account for menu bar */ }
            <div className={classes.toolbar}/>
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
                };
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
                          focusRow={this.state.sampleFocusIndex}/>
                      );
                    } else if (windowConfig.dataViewType === 'graph') {
                      return (
                        <GraphGLWindow key={i}
                          decomposition={windowConfig.decomposition}
                          dataset={this.state.currentDataset}
                          onNodeHover={this.onSampleFocus}/>
                      );
                    } else if (windowConfig.dataViewType === 'scatter_plot') {
                      return (
                        <ScatterPlotWindow key={i}
                        attributeGroup={windowConfig.scatterPlotAttributeGroup}
                        dataset={this.state.currentDataset}/>
                      );
                    } else if (windowConfig.dataViewType === 'thumbnail') {
                      return (<ThumbnailWindow key={i} dataset={this.state.currentDataset}/>)
                    }
                    else {
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
