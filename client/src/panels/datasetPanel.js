import Divider from 'material-ui/Divider';
import { FormControl } from 'material-ui/Form';
import { InputLabel } from 'material-ui/Input';
import List, { ListItem } from 'material-ui/List';
import { MenuItem } from 'material-ui/Menu';
import React from 'react';
import Paper from 'material-ui/Paper';
import PropTypes from 'prop-types';
import Select from 'material-ui/Select';
import { withStyles } from 'material-ui/styles';


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
    this.setState({ datasetName:datasetName });

    let datasetId = this.datasetMap.get(datasetName);
    this.client.fetchDataset(datasetId).then(function(dataset) {
      this.setState({
        dataset: {
          numberOfSamples: dataset.numberOfSamples,
          qoiNames: dataset.qoiNames,
          attributeNames: dataset.attributeNames,
        },
      });
    }.bind(this));
  };

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    return (
      <Paper style={{ padding:'15px', paddingBottom:'20px' }}>
        <div style={{ display:'flex', flexDirection:'column' }}>
          <FormControl className={classes.formControl}>
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
                <span style={{ color:'#888' }}>
                  {' # Samples: '}
                </span>
                <span style={{ color:'#888' }}>
                  { this.state.dataset ?
                    this.state.dataset.numberOfSamples :
                    '--' }
                </span>
              </div>
            </ListItem>
            <Divider />
            <ListItem style={{ paddingLeft:'0px', paddingRight:'5px' }}>
              <div style={{
                width: '100%',
                display: 'flex',
                justifyContent: 'space-between',
              }}>
                <span style={{ color:'#888' }}>
                  {'# QoIs: '}
                </span>
                <span style={{ color:'#888' }}>
                  { this.state.dataset ?
                    this.state.dataset.qoiNames.length :
                    '--' }
                </span>
              </div>
            </ListItem>
          </List>
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
