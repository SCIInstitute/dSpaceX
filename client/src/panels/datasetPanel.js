import Divider from 'material-ui/Divider';
import { FormControl } from 'material-ui/Form';
import { InputLabel } from 'material-ui/Input';
import List from 'material-ui/List';
import { ListItem } from 'material-ui/List';
import { MenuItem } from 'material-ui/Menu';
import Paper from 'material-ui/Paper';
import PropTypes from 'prop-types';
import React from 'react';
import Select from 'material-ui/Select';
import { withStyles } from 'material-ui/styles';

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
      qoiName: '',
    };

    this.handleDatasetChange = this.handleDatasetChange.bind(this);
    this.handleQoiChange = this.handleQoiChange.bind(this);
    this.client = this.props.client;
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
          qoiNames: dataset.qoiNames,
          attributeNames: dataset.attributeNames,
        },
      });
      this.props.onDatasetChange(dataset);
    }.bind(this));
  };


  /**
   * Handles the user changing the current active qoi.
   * @param {Event} event
   */
  handleQoiChange(event) {
    let qoiName = event.target.value;
    if (qoiName === '') {
      this.setState({
        qoiName: null,
      });
      if (this.props.onQoiChange) {
        this.props.onQoiChange(null);
      }
      return;
    }

    this.setState({ qoiName });
    let datasetId = this.state.dataset.id;
    this.client.fetchQoi(datasetId, qoiName).then(function(qoiVector) {
      console.dir(qoiVector);
      if (this.props.onQoiChange) {
        this.props.onQoiChange(qoiVector);
      }
    }.bind(this));
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    let enabled = this.props.datasets && this.props.datasets.length > 0;
    let textColor =
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
          <FormControl className={classes.formControl}
            disabled={!this.state.dataset}>
            <InputLabel htmlFor='qoi-field'>QoI</InputLabel>
            <Select ref="qoiCombo" value={this.state.qoiName}
              onChange={this.handleQoiChange} inputProps={{
                name: 'qoi',
                id: 'qoi-field',
              }}>
              <MenuItem value="">
                <em>None</em>
              </MenuItem>
              {
                (this.state.dataset && this.state.dataset.qoiNames) ?
                  this.state.dataset.qoiNames.map((name) => (
                    <MenuItem value={name} key={name}>
                      {name}
                    </MenuItem>
                  ))
                  : []
              }
            </Select>
          </FormControl>
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
export default withStyles({})(DatasetPanel);
