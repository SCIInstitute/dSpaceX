import DecompositionPanel from './decompositionPanel.js';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import FormControl from '@material-ui/core/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import PropTypes from 'prop-types';
import React from 'react';
import Select from '@material-ui/core/Select';
import TextField from '@material-ui/core/TextField';
import Typography from '@material-ui/core/Typography';
import { withStyles } from '@material-ui/core/styles';

/**
 * A Window Panel provides a display of the settings associated with any given
 * visualization window in the scene.
 */
class WindowPanel extends React.Component {
  /**
   * WindowPanel constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);
    this.state = {
      id: this.props.config.id,
      dataViewType: this.props.config.dataViewType,
      tableAttributeGroup: this.props.config.tableAttributeGroup,
      decomposition: null,
      embeddingAlgorithm: this.props.config.embeddingAlgorithm,
      visualizationQoi: null,
    };

    this.handleDataViewTypeChange =
        this.handleDataViewTypeChange.bind(this);
    this.handleTableAttributeGroup =
        this.handleTableAttributeGroup.bind(this);
    this.handleEmbeddingAlgorithmChange =
        this.handleEmbeddingAlgorithmChange.bind(this);
    this.handleDecompositionChange = this.handleDecompositionChange.bind(this);

    this.getTableOptions = this.getTableOptions.bind(this);
    this.getGraphOptions = this.getGraphOptions.bind(this);
  }

  /**
   * @param {object} prevProps
   * @param {prevState} prevState
   * @param {object} snapshot
   */
  componentDidUpdate(prevProps, prevState, snapshot) {
    if (prevState != this.state) {
      if (this.props.onConfigChange) {
        this.props.onConfigChange(this.state);
      }
    }
  }

  /**
   * Handle the window type changing between table and graph.
   * @param {object} event
   */
  handleDataViewTypeChange(event) {
    let dataViewType = event.target.value;
    this.setState({
      dataViewType: dataViewType,
    });
  }

  /**
   * Handle the table attribute group changing between params and qois.
   * @param {object} event
   */
  handleTableAttributeGroup(event) {
    let tableAttributeGroup = event.target.value;
    this.setState({
      tableAttributeGroup: tableAttributeGroup,
    });
  }

  /**
   * Handle the embedding algorithm changing.
   * @param {object} event
   */
  handleEmbeddingAlgorithmChange(event) {
    let algorithm = event.target.value;
    this.setState({
      embeddingAlgorithm: algorithm,
    });
  }

  /**
   * Handle the decomposition changing.
   * @param {object} decomposition
   */
  handleDecompositionChange(decomposition) {
    this.setState({
      decomposition: decomposition,
    });
  }

  /**
   * Return the set of table display types.
   * @return {JSX}
   */
  getTableOptions() {
    const { classes } = this.props;
    return (
      <React.Fragment>
        { /* Table Attribute Group Dropdown */ }
        <FormControl className={classes.formControl}
          disabled={!this.props.enabled || !this.props.dataset}>
          <InputLabel htmlFor='tablegroup-input'>Attribute Group</InputLabel>
          <Select ref="tablegroupCombo"
            value={this.state.tableAttributeGroup || ''}
            onChange={this.handleTableAttributeGroup} inputProps={{
              name: 'tablegroup',
              id: 'tablegroup-input',
            }}>
            <MenuItem value="parameters"
              disabled={!this.props.dataset.parameterNames.length}>
              <em>Parameters</em>
            </MenuItem>
            <MenuItem value="qois"
              disabled={!this.props.dataset.qoiNames.length}>
              <em>Qois</em>
            </MenuItem>
          </Select>
        </FormControl>
      </React.Fragment>
    );
  }

  /**
   * Get the configurable options for graph types.
   * @return {JSX}
   */
  getGraphOptions() {
    const { classes } = this.props;
    return (
      <React.Fragment>
        { /* Metric Dropdown */ }
        <FormControl className={classes.formControl}
          disabled={!this.props.enabled}>
          <InputLabel htmlFor='metric-field'>Metric</InputLabel>
          <Select ref="metricCombo"
            value="precomputed"
            inputProps={{
              name: 'metric',
              id: 'metric-field',
            }}>
            <MenuItem value='precomputed'>
              <em>Precomputed</em>
            </MenuItem>
          </Select>
        </FormControl>

        { /* Neighborhood (k) */ }
        <TextField
          id="neighborhood"
          label="Neighborhood Size"
          type="number"
          defaultValue="15"
          disabled={true}
          InputLabelProps={{
            shrink: true,
          }}/>

        { /* Embedding Algorithm Dropdown */ }
        <FormControl className={classes.formControl}
          disabled={!this.props.enabled || !this.props.dataset}>
          <InputLabel htmlFor='algorithm-input'>Embedding Algorithm</InputLabel>
          <Select ref="algorithmCombo"
            value={this.state.embeddingAlgorithm || 'precomputed'}
            onChange={this.handleEmbeddingAlgorithmChange} inputProps={{
              name: 'algorithm',
              id: 'algorithm-input',
            }}>
            <MenuItem value="precomputed">
              <em>Precomputed</em>
            </MenuItem>
            <MenuItem value="pca" disabled={true}>
              <em>PCA</em>
            </MenuItem>
            <MenuItem value="isomap" disabled={true}>
              <em>ISOMap</em>
            </MenuItem>
            <MenuItem value="t-sne" disabled={true}>
              <em>t-SNE</em>
            </MenuItem>
          </Select>
        </FormControl>

        <div style={{ height:'8px' }}></div>
        <DecompositionPanel
          enabled={true}
          dataset={this.props.dataset}
          onDecompositionChange={this.handleDecompositionChange}
          client={this.client}/>
      </React.Fragment>
    );
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    return (
      <ExpansionPanel
        disabled={!this.props.enabled} defaultExpanded={true}
        style={{ paddingLeft:'0px', margin:'1px', paddingTop:'0px' }}>
        <ExpansionPanelSummary expandIcon={ <ExpandMoreIcon/> }>
          <Typography>
            { 'Window # ' + (this.props.windowIndex + 1)}
          </Typography>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails style={{ paddingLeft: '15px', paddingTop: '0px',
          paddingRight: '10px', marginLeft: '1px', marginRight: '1px',
          width: '100%', boxSizing: 'border-box' }}>
          <div style={{ display: 'flex', flexDirection: 'column',
            width: '100%', boxSizing: 'border-box' }}>

            { /* DataView Type Dropdown */ }
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='dataview-input'>Data View Type</InputLabel>
              <Select ref="dataviewCombo"
                value={this.state.dataViewType || ''}
                onChange={this.handleDataViewTypeChange} inputProps={{
                  name: 'dataview',
                  id: 'dataview-input',
                }}>
                <MenuItem value="table">
                  <em>Table</em>
                </MenuItem>
                <MenuItem value="graph">
                  <em>Graph</em>
                </MenuItem>
              </Select>
            </FormControl>

            {
              /* Render the Appropriate Fields for the DataView Type */
              (() => {
                switch (this.state.dataViewType) {
                  case 'table':
                    return this.getTableOptions();
                  case 'graph':
                    return this.getGraphOptions();
                  default:
                    return null;
                };
              })()
            }

          </div>
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
};

// Enforce that Application receives styling.
WindowPanel.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap WindowPanel in Styling Container.
export default withStyles({})(WindowPanel);
