import Divider from '@material-ui/core/Divider';
import FormControl from '@material-ui/core/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import MenuItem from '@material-ui/core/MenuItem';
import Paper from '@material-ui/core/Paper';
import PropTypes from 'prop-types';
import React from 'react';
import Select from '@material-ui/core/Select';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';

const disabledLabelColor = '#888';
const enabledLabelColor = '#000';

/**
 * The DatasetPanel component allows the user to control
 * the active dataset and see dataset metadata.
 */
class DatasetPanel extends React.Component {
  /**
   * The DatasetPanel constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);
    this.state = {
      datasetName: '',
      dataset: null,
    };

    this.handleDatasetChange = this.handleDatasetChange.bind(this);
    this.client = this.props.dsxContext.client;
    this.datasetMap = null;
    this.initializeDatasetMap();
  }

  /**
   * Update map based on new dataset prop.
   * @param {object} prevProps
   * @param {object} prevState
   */
  componentDidUpdate(prevProps, prevState) {
    this.initializeDatasetMap();
  }

  /**
   * Creates a reverse map from Dataset name to id.
   */
  initializeDatasetMap() {
    if (this.datasetMap != null) {
      delete this.datasetMap;
    }
    this.datasetMap = new Map();
    for (let i=0; i < this.props.datasets.length; i++) {
      let dataset = this.props.datasets[i];
      this.datasetMap.set(dataset.name, dataset.id);
    }
  }

  /**
   * Handles the user changing the current active dataset.
   * @param {Event} event
   */
  handleDatasetChange(event) {
    let datasetName = event.target.value;
    if (datasetName === '') {
      this.setState({
        datasetName: '',
        dataset: null,
      });
      this.props.onDatasetChange(null);
      return;
    }

    this.setState({ datasetName });
    let datasetId = this.datasetMap.get(datasetName);
    this.client.fetchDataset(datasetId).then(function(dataset) {
      this.setState({
        dataset: {
          id: datasetId,
          numberOfSamples: dataset.numberOfSamples,
          parameterNames: dataset.parameterNames,
          qoiNames: dataset.qoiNames,
        },
      });
      this.props.onDatasetChange(dataset);
    }.bind(this));
  };

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    let enabled = this.props.enabled && this.props.datasets &&
        this.props.datasets.length > 0;
    let textColor = enabled &&
        this.state.dataset ? enabledLabelColor : disabledLabelColor;
    let backgroundColor = enabled ? '#fff' : '#ddd';
    return (
      <Paper style={{
        padding: '15px',
        paddingBottom: '5px',
        backgroundColor: backgroundColor,
      }}>
        <div style={{ display:'flex', flexDirection:'column' }}>
          <FormControl className={classes.formControl}
            disabled={!enabled}>
            <InputLabel htmlFor='dataset-field'>Dataset</InputLabel>
            <Select ref="datasetCombo" value={this.state.datasetName}
              onChange={this.handleDatasetChange} inputProps={{
                name: 'dataset',
                id: 'dataset-field',
              }}>
              <MenuItem value="">
                <em>None</em>
              </MenuItem>
              {
                this.props.datasets.map((dataset) => (
                  <MenuItem value={dataset.name} key={dataset.id}>
                    {dataset.name}
                  </MenuItem>
                ))
              }
            </Select>
          </FormControl>
          <Divider />
          <List>
            <ListItem style={{ paddingLeft:'0px', paddingRight:'5px' }}>
              <div style={{
                width: '100%',
                display: 'flex',
                justifyContent: 'space-between' }}>
                <span style={{ color:textColor }}>
                  {' # Samples: '}
                </span>
                <span style={{ color:textColor }}>
                  { this.state.dataset ?
                    this.state.dataset.numberOfSamples :
                    '--' }
                </span>
              </div>
            </ListItem>
            <Divider/>
            <ListItem style={{ paddingLeft:'0px', paddingRight:'5px' }}>
              <div style={{
                width: '100%',
                display: 'flex',
                justifyContent: 'space-between',
              }}>
                <span style={{ color:textColor }}>
                  {'# Parameters: '}
                </span>
                <span style={{ color:textColor }}>
                  { this.state.dataset && this.state.dataset.parameterNames ?
                    this.state.dataset.parameterNames.length :
                    '--' }
                </span>
              </div>
            </ListItem>
            <Divider/>
            <ListItem style={{ paddingLeft:'0px', paddingRight:'5px' }}>
              <div style={{
                width: '100%',
                display: 'flex',
                justifyContent: 'space-between',
              }}>
                <span style={{ color:textColor }}>
                  {'# QoIs: '}
                </span>
                <span style={{ color:textColor }}>
                  { this.state.dataset ?
                    this.state.dataset.qoiNames.length :
                    '--' }
                </span>
              </div>
            </ListItem>
          </List>
          <div style={{ width:'100%', height:'5px' }}/>
        </div>
      </Paper>
    );
  }
}

// Enforce that Application receives styling.
DatasetPanel.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withDSXContext(withStyles({})(DatasetPanel));
