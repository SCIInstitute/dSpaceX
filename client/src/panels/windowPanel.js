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
      embeddingAlgorithm: this.props.embeddings[0] ? this.props.embeddings[0].name.trim() : 'None',
      visualizationQoi: null,
      xAttributeGroup: this.props.config.xAttributeGroup,
      xAttribute: this.props.config.xAttribute,
      yAttributeGroup: this.props.config.yAttributeGroup,
      yAttribute: this.props.config.yAttribute,
      markerAttributeGroup: this.props.config.markerAttributeGroup,
      markerAttribute: this.props.config.markerAttribute,
    };

    this.handleDataViewTypeChange = this.handleDataViewTypeChange.bind(this);
    this.handleTableAttributeGroup = this.handleTableAttributeGroup.bind(this);
    this.handleXAttributeGroup = this.handleXAttributeGroup.bind(this);
    this.handleXAttribute = this.handleXAttribute.bind(this);
    this.handleYAttributeGroup = this.handleYAttributeGroup.bind(this);
    this.handleYAttribute = this.handleYAttribute.bind(this);
    this.handleMarkerAttributeGroup = this.handleMarkerAttributeGroup.bind(this);
    this.handleMarkerAttribute = this.handleMarkerAttribute.bind(this);
    this.handleEmbeddingAlgorithmChange = this.handleEmbeddingAlgorithmChange.bind(this);
    this.handleDecompositionChange = this.handleDecompositionChange.bind(this);

    this.getTableOptions = this.getTableOptions.bind(this);
    this.getGraphOptions = this.getGraphOptions.bind(this);
    this.getScatterPlotOptions = this.getScatterPlotOptions.bind(this);
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
   * Handles x attribute group change
   * @param {event} event
   */
  handleXAttributeGroup(event) {
    let xAttributeGroup = event.target.value;
    let xAttribute = undefined;
    this.setState({ xAttributeGroup, xAttribute });
  }

  /**
   * Handles x attribute change
   * @param {event} event
   */
  handleXAttribute(event) {
    let xAttribute = event.target.value;
    this.setState({ xAttribute });
  }

  /**
   * Handles y attribute group change
   * @param {event} event
   */
  handleYAttributeGroup(event) {
    let yAttributeGroup = event.target.value;
    let yAttribute = undefined;
    this.setState({ yAttributeGroup, yAttribute });
  }

  /**
   * Handles y attribute change
   * @param {event} event
   */
  handleYAttribute(event) {
    let yAttribute = event.target.value;
    this.setState({ yAttribute });
  }

  /**
   * Handles marker attribute group change
   * @param {event} event
   */
  handleMarkerAttributeGroup(event) {
    let markerAttributeGroup = event.target.value;
    let markerAttribute = undefined;
    this.setState({ markerAttributeGroup, markerAttribute });
  }

  /**
   * Handles marker attribute change
   * @param {event} event
   */
  handleMarkerAttribute(event) {
    let markerAttribute = event.target.value;
    this.setState({ markerAttribute });
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
        {/* Table Attribute Group Dropdown */}
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
        {/* Embedding Algorithm Dropdown */}
        <FormControl className={classes.formControl}
          disabled={!this.props.enabled || !this.props.dataset}>
          <InputLabel htmlFor='algorithm-input'>Embedding Algorithm</InputLabel>
          <Select ref="algorithmCombo"
            value={this.state.embeddingAlgorithm}
            onChange={this.handleEmbeddingAlgorithmChange} inputProps={{
              name: 'algorithm',
              id: 'algorithm-input',
            }}>
            {!this.props.embeddings &&
            <MenuItem value="">
              <em>None</em>
            </MenuItem>}
            {this.props.embeddings.map((embedding) =>
              <MenuItem key={embedding.id} value={embedding.name.trim()}>
                <em>{embedding.name}</em>
              </MenuItem>)}
          </Select>
        </FormControl>

        {/* Metric Dropdown */}
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
            <MenuItem value='l2' disabled={true}>
              <em>L2</em>
            </MenuItem>
            <MenuItem value='l1' disabled={true}>
              <em>L1</em>
            </MenuItem>
          </Select>
        </FormControl>

        {/* Decomposition Panel */}
        <div style={{ height:'8px' }}/>
        <DecompositionPanel
          enabled={true}
          dataset={this.props.dataset}
          onDecompositionChange={this.handleDecompositionChange}
          client={this.client}/>
      </React.Fragment>
    );
  };

  /**
   * Gets the attribute groups
   * @param {string} attributeGroup
   * @return {JSX}
   */
  getAttributeNames(attributeGroup) {
    if (attributeGroup === 'parameters') {
      return this.props.dataset.parameterNames;
    } else if (attributeGroup === 'qois') {
      return this.props.dataset.qoiNames;
    }
  };

  /**
   * Gets the scatter plot options.
   * @return {JSX}
   */
  getScatterPlotOptions() {
    const { classes } = this.props;
    return (
      <ExpansionPanel disabled={!this.props.enabled || !this.props.dataset}
        defaultExpanded={true} style={{ paddingLeft:'0px', margin:'1px' }}>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon/>}/>
        <ExpansionPanelDetails style={{
          paddingLeft: '15px',
          paddingRight: '10px', margin: '1px', width: '100%',
          boxSizing: 'border-box',
        }}>
          <div style={{
            display: 'flex', flexDirection: 'column',
            width: '100%', boxSizing: 'border-box',
          }}>
            {/* x axis */}
            <Typography>x-axis</Typography>
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}
              style={{ width: '100%',
                boxSizing: 'border-box',
                paddingRight: '10px' }}>
              <InputLabel htmlFor='x-group-input'>
                Attribute Group</InputLabel>
              <Select ref="xGroupCombo"
                value={this.state.xAttributeGroup || ''}
                onChange={this.handleXAttributeGroup} inputProps={{
                  name: 'xGroup',
                  id: 'x-group-input',
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
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='x-attribute-input'>
                Attribute</InputLabel>
              <Select ref="xAttributeCombo"
                value={this.state.xAttribute || ''}
                onChange={this.handleXAttribute} inputProps={{
                  name: 'xAttribute',
                  id: 'x-attribute-input',
                }}>
                {this.state.xAttributeGroup
                && this.getAttributeNames(this.state.xAttributeGroup).map((attributeName, i) =>
                  <MenuItem key={i} value={attributeName.trim()}>{attributeName}</MenuItem>)}
              </Select>
            </FormControl>
            {/* y axis */}
            <div style={{ height:'10px' }}/>
            <Typography>y-axis</Typography>
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='y-group-input'>
                Attribute Group</InputLabel>
              <Select ref="yGroupCombo"
                value={this.state.yAttributeGroup || ''}
                onChange={this.handleYAttributeGroup} inputProps={{
                  name: 'yGroup',
                  id: 'y-group-input',
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
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='y-attribute-input'>
                Attribute</InputLabel>
              <Select ref="yAttributeCombo"
                value={this.state.yAttribute || ''}
                onChange={this.handleYAttribute} inputProps={{
                  name: 'yAttribute',
                  id: 'y-attribute-input',
                }}>
                {this.state.yAttributeGroup &&
                this.getAttributeNames(this.state.yAttributeGroup).map((attributeName, i) =>
                  <MenuItem key={i} value={attributeName.trim()}>{attributeName}</MenuItem>)}
              </Select>
            </FormControl>
            {/* marker area */}
            <div style={{ height:'10px' }}/>
            <Typography>Marker Area</Typography>
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='marker-group-input'>
                Attribute Group</InputLabel>
              <Select ref="markerGroupCombo"
                value={this.state.markerAttributeGroup || ''}
                onChange={this.handleMarkerAttributeGroup} inputProps={{
                  name: 'markerGroup',
                  id: 'marker-group-input',
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
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='marker-attribute-input'>
                Attribute</InputLabel>
              <Select ref="markerAttributeCombo"
                value={this.state.markerAttribute || ''}
                onChange={this.handleMarkerAttribute} inputProps={{
                  name: 'markerAttribute',
                  id: 'marker-attribute-input',
                }}>
                {this.state.markerAttributeGroup &&
                this.getAttributeNames(this.state.markerAttributeGroup).map((attributeName, i) =>
                  <MenuItem key={i} value={attributeName.trim()}>{attributeName}</MenuItem>)}
              </Select>
            </FormControl>
          </div>
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  };

  /**
   * Renders the component to HTML.
   * @return {JSX}
   */
  render() {
    const { classes } = this.props;
    return (
      <ExpansionPanel
        disabled={!this.props.enabled} defaultExpanded={true}
        style={{ paddingLeft:'0px', margin:'1px', paddingTop:'0px' }}>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon/>}>
          <Typography>
            {'Window # ' + (this.props.windowIndex + 1)}
          </Typography>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails style={{
          paddingLeft: '15px', paddingTop: '0px',
          paddingRight: '10px', marginLeft: '1px', marginRight: '1px',
          width: '100%', boxSizing: 'border-box',
        }}>
          <div style={{
            display: 'flex', flexDirection: 'column',
            width: '100%', boxSizing: 'border-box',
          }}>

            {/* DataView Type Dropdown */}
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='dataview-input'>Data View Type</InputLabel>
              <Select ref="dataviewCombo"
                value={this.state.dataViewType || ''}
                onChange={this.handleDataViewTypeChange} inputProps={{
                  name: 'dataview',
                  id: 'dataview-input',
                }}>
                <MenuItem value="graph">
                  <em>Embedding/Decomposition</em>
                </MenuItem>
                <MenuItem value="gallery">
                  <em>Gallery</em>
                </MenuItem>
                <MenuItem value="scatter_plot">
                  <em>Scatter Plot</em>
                </MenuItem>
                <MenuItem value="table">
                  <em>Table</em>
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
                  case 'scatter_plot':
                    return this.getScatterPlotOptions();
                  default:
                    return null;
                }
              })()
            }

          </div>
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
}

// Enforce that Application receives styling.
WindowPanel.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap WindowPanel in Styling Container.
export default withStyles({})(WindowPanel);
