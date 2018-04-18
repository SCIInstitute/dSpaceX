import Client from './client';
import ConnectionDialog from './connectionDialog';
import DatasetPanel from './panels/datasetPanel';
import DecompositionPanel from './panels/decompositionPanel';
import DisplayPanel from './panels/displayPanel';
import Drawer from 'material-ui/Drawer';
import GraphWebGLWindow from './windows/graphWebGLWindow';
import PropTypes from 'prop-types';
import React from 'react';
import Toolbar from './toolbar';
import Workspace from './workspace';
import { withStyles } from 'material-ui/styles';

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
  appBar: {
    zIndex: theme.zIndex.drawer + 1,
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
      currentDecomposition: null,
      datasets: [],
    };

    this.connectButtonClicked = this.connectButtonClicked.bind(this);
    this.onConnect = this.onConnect.bind(this);
    this.onNetworkActivityStart = this.onNetworkActivityStart.bind(this);
    this.onNetworkActivityEnd = this.onNetworkActivityEnd.bind(this);
    this.onDatasetChange = this.onDatasetChange.bind(this);
    this.onDecompositionChange = this.onDecompositionChange.bind(this);

    this.client = new Client();
    this.client.addEventListener('connected', this.onConnect);
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
   * Handles whent he applications starts communicating with the server.
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
   * Handles updating dataflow to components when decomposition changes.
   * @param {object} decomposition
   */
  onDecompositionChange(decomposition) {
    this.setState({
      currentDecomposition: decomposition,
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
      <div className={classes.root}>
        <Toolbar className={classes.appBar}
          connectedToServer={this.state.connected}
          onConnectClick={this.connectButtonClicked}
          networkActive={this.state.networkActive} />
        <ConnectionDialog ref='connectiondialog' client={this.client}/>
        <Drawer PaperProps={{ elevation:6 }} variant='permanent'
          classes={{ paper:classes.drawerPaper }}>
          { /* Add div to account for menu bar */ }
          <div className={classes.toolbar} />
          <DatasetPanel datasets={this.state.datasets}
            onDatasetChange={this.onDatasetChange}
            client={this.client}/>
          <DecompositionPanel dataset={this.state.currentDataset}
            onDecompositionChange={this.onDecompositionChange}
            client={this.client}/>
          <DisplayPanel dataset={this.state.currentDataset}/>
          <div style={{
            backgroundColor: drawerMarginColor,
            height: '100%',
            width: '100%',
          }}></div>
        </Drawer>
        <Workspace className={classes.content}>
          { /* Add div to account for menu bar */ }
          <div className={classes.toolbar}/>
          <div className={classes.workspace}>
            {
              !!this.state.currentDecomposition ? [
                <GraphWebGLWindow key='1' dataset={this.state.currentDataset}
                  decomposition={this.state.currentDecomposition} />,
                // <WebGLWindow key='2' dataset={this.state.currentDataset}/>,
              ] : []
            }
          </div>
        </Workspace>
      </div>
    );
  }
}

// Enforce that Application receives styling.
Application.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withStyles(styles)(Application);
