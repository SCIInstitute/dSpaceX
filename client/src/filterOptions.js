import React, { Component } from 'react';
import { Close } from '@material-ui/icons';
import FormControl from '@material-ui/core/es/FormControl/FormControl';
import Histogram from './panels/histogram';
import IconButton from '@material-ui/core/es/IconButton/IconButton';
import Input from '@material-ui/core/Input';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/es/MenuItem/MenuItem';
import Paper from '@material-ui/core/Paper';
import Select from '@material-ui/core/Select';
import { withStyles } from '@material-ui/core/styles';

/**
 * The FilterOptions provides the drop downs
 * to select the parameter/qoi and to setup
 * the histogram to create a filter for the gallery view.
 */
class FilterOptions extends Component {
  /**
   * The FilterPanelConstructor
   * @param {object} props
   */
  constructor(props) {
    super(props);


    this.handleGroupChange = this.handleGroupChange.bind(this);
    this.handleAttributeChange = this.handleAttributeChange.bind(this);
    this.handleBinChange = this.handleBinChange.bind(this);
    this.onBrush = this.onBrush.bind(this);
  };

  /**
   * Handles attribute group change
   * @param {object} event
   */
  handleGroupChange(event) {
    const { updateFilter, filterConfig } = this.props;
    filterConfig.attributeGroup = event.target.value;
    filterConfig.attribute = '';
    updateFilter(filterConfig);
  };

  /**
   * Handles attribute change
   * @param {object} event
   */
  handleAttributeChange(event) {
    const { updateFilter, filterConfig } = this.props;
    filterConfig.attribute = event.target.value;
    updateFilter(filterConfig);
  };

  /**
   * Handles bin number change
   * @param {object} event
   */
  handleBinChange(event) {
    const { updateFilter, filterConfig } = this.props;
    filterConfig.numberOfBins = event.target.value;
    updateFilter(filterConfig);
  };

  /**
   * Gets the current attribute names based on the selected
   * attribute group
   * @return {Array} attribute names
   */
  getAttributeNames() {
    const { filterConfig, parameters, qois } = this.props;
    if (filterConfig.attributeGroup === 'parameters') {
      return parameters.map((param) => param.parameterName);
    } else if (filterConfig.attributeGroup === 'qois') {
      return qois.map((qoi) => qoi.qoiName);
    }
  };

  /**
   * Gets the data necessary to draw the histogram correctly
   * @return {Array}
   */
  getHistogramData() {
    const { filterConfig, parameters, qois } = this.props;
    let data = null;
    if (filterConfig.attributeGroup === 'parameters') {
      data = parameters.filter((p) => p.parameterName === filterConfig.attribute)[0].parameter;
    } else if (filterConfig.attributeGroup === 'qois') {
      data = qois.filter((q) => q.qoiName === filterConfig.attribute)[0].qoi;
    }
    const globalMax = Math.max(...data);
    this.globalMin = Math.min(...data);
    this.stepSize = (globalMax - this.globalMin) / filterConfig.numberOfBins;

    let counts = [];
    let steps = [];
    let low = this.globalMin;
    while (low < globalMax) {
      let high = low + this.stepSize;
      steps.push(low);
      steps.push(high);
      counts.push(data.filter((d) => d >= low && d < high).length);
      low = high;
    }
    return counts;
  };

  /**
   * When the histogram is brushed the functional collect all the necessary
   * data to update the filter
   * @param {number} selectionMin
   * @param {number} selectionMax
   */
  onBrush(selectionMin, selectionMax) {
    const { updateFilter, filterConfig } = this.props;

    filterConfig.enabled = true;
    filterConfig.min = (selectionMin * this.stepSize) + this.globalMin;
    filterConfig.max = (selectionMax * this.stepSize) + this.globalMin;
    filterConfig.selectionMin = selectionMin;
    filterConfig.selectionMax = selectionMax;

    updateFilter(filterConfig);
  }

  /**
   * Renders the filter panel
   * @return {jsx}
   */
  render() {
    const { classes, removeFilter, filterConfig, parameters, qois } = this.props;
    return (
      <Paper style={{ paddingLeft:'10px', paddingRight:'10px', paddingBottom:'20px' }}>
        <IconButton variant='contained' style={{ marginLeft:'170px' }} onClick={() => removeFilter(filterConfig.id)}>
          <Close className={classes.icon} fontSize='small'/>
        </IconButton>
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap', marginBottom:'5px' }}>
          <InputLabel htmlFor='filter-group-label'>Attribute Group</InputLabel>
          <Select
            value={filterConfig.attributeGroup}
            onChange={this.handleGroupChange}
            input={<Input name='attributeGroup' id='filter-group-label'/>}
            displayEmpty
            name='attributeGroup'
            autoWidth={true}>
            {<MenuItem value='parameters' disabled={!parameters.length > 0}>Parameters</MenuItem>}
            {<MenuItem value='qois' disabled={!qois.length > 0}>Qois</MenuItem>}
          </Select>
        </FormControl>
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap', marginBottom:'5px' }}>
          <InputLabel htmlFor='filter-attribute-label'>Attribute</InputLabel>
          <Select
            value={filterConfig.attribute}
            onChange={this.handleAttributeChange}
            input={<Input name='attribute' id='filter-attribute-label'/>}
            displayEmpty
            name='attribute'
            autoWidth={true}>
            {filterConfig.attributeGroup && this.getAttributeNames().map((attributeName, i) => (
              <MenuItem key={i} value={attributeName.trim()}>{attributeName}</MenuItem>
            ))}
          </Select>
        </FormControl>
        {filterConfig.attributeGroup && filterConfig.attribute &&
        <Histogram
          size={[200, 100]}
          data={this.getHistogramData()}
          brushEnabled={true}
          filterCount={this.props.filterCount}
          filterConfig={filterConfig}
          onBrush={this.onBrush}/>}
        {filterConfig.attributeGroup && filterConfig.attribute &&
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap' }}>
          <InputLabel htmlFor='filter-bin-label'># of bins</InputLabel>
          <Select
            value={filterConfig.numberOfBins}
            onChange={this.handleBinChange}
            input={<Input name='bin' id='filter-bin-label'/>}
            displayEmpty
            name='bin'
            autoWidth={true}>
            <MenuItem value={10}>10</MenuItem>
            <MenuItem value={25}>25</MenuItem>
            <MenuItem value={50}>50</MenuItem>
          </Select>
        </FormControl>}
      </Paper>);
  }
}

export default withStyles({})(FilterOptions);
