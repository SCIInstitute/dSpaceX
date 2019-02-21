import React, { Component } from 'react';
import FormControl from '@material-ui/core/es/FormControl/FormControl';
import Histogram from './histogram';
import IconButton from '@material-ui/core/es/IconButton/IconButton';
import Input from '@material-ui/core/Input';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/es/MenuItem/MenuItem';
import Select from '@material-ui/core/Select';
import { Close } from '@material-ui/icons';
import { withStyles } from '@material-ui/core/styles';

/**
 * The FilterPanel provides the drop downs
 * to select the parameter/qoi and to setup
 * the histogram to create a filter for the gallery view.
 */
class FilterPanel extends Component {
  /**
   * The FilterPanelConstructor
   * @param {object} props
   */
  constructor(props) {
    super(props);
    this.state = {
      attributeGroup: '',
      attribute: '',
      numberBins: 10,
    };

    this.handleGroupChange = this.handleGroupChange.bind(this);
    this.handleAttributeChange = this.handleAttributeChange.bind(this);
    this.handleBinChange = this.handleBinChange.bind(this);
    this.onBrush = this.onBrush.bind(this);
  };

  /**
   * Handle attribute group change
   * @param {object} event
   */
  handleGroupChange(event) {
    this.setState({ attributeGroup:event.target.value, attribute:'' });
  };

  /**
   * Handle attribute change
   * @param {object} event
   */
  handleAttributeChange(event) {
    this.setState({ attribute:event.target.value });
  };

  handleBinChange(event) {
    this.setState( { numberBins:event.target.value });
  };

  getAttributeNames() {
    if (this.state.attributeGroup === 'parameters') {
      return this.props.parameters.map((param) => param.parameterName);
    } else if (this.state.attributeGroup === 'qois') {
      return this.props.qois.map((qoi) => qoi.qoiName);
    }
  };

  getData() {
    let data = null;
    if (this.state.attributeGroup === 'parameters') {
      data = this.props.parameters.filter((p) => p.parameterName === this.state.attribute)[0].parameter;
    } else if (this.state.attributeGroup === 'qois') {
      data = this.props.qois.filter((q) => q.qoiName === this.state.attribute)[0].qoi;
    }
    const max = Math.max(...data);
    const min = Math.min(...data);
    this.stepSize = (max - min) / this.state.numberBins;
    let step = min;
    let counts = [];
    while (step < max) {
      let low = step;
      let high = step + this.stepSize;
      counts.push(data.filter((d) => d >= low && d < high).length);
      step = high;
    }
    return counts;
  };

  onBrush(min, max) {
    min = min * this.stepSize;
    max = max * this.stepSize;
    this.props.addFilter(this.props.id, min, max, this.state.attributeGroup, this.state.attribute);
  }

  /**
   * Renders the filter panel
   * @return {jsx}
   */
  render() {
    const { classes } = this.props;
    return (
      <div style={{ width:'200px', marginLeft:'10px'}}>
        <IconButton variant='raised' style={{ marginLeft:'165px' }} onClick={() => { console.log('Remove Clicked'); }}>
          <Close className={classes.icon} fontSize='small'/>
        </IconButton>
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap', marginBottom:'5px' }}>
          <InputLabel htmlFor='filter-group-label'>Attribute Group</InputLabel>
          <Select
            value={this.state.attributeGroup}
            onChange={this.handleGroupChange}
            input={<Input name='attributeGroup' id='filter-group-label'/>}
            displayEmpty
            name='attributeGroup'
            autoWidth={true}>
            <MenuItem value='parameters'>Parameters</MenuItem>
            <MenuItem value='qois'>Qois</MenuItem>
          </Select>
        </FormControl>
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap', marginBottom:'5px' }}>
          <InputLabel htmlFor='filter-attribute-label'>Attribute</InputLabel>
          <Select
            value={this.state.attribute}
            onChange={this.handleAttributeChange}
            input={<Input name='attribute' id='filter-attribute-label'/>}
            displayEmpty
            name='attribute'
            autoWidth={true}>
            {this.state.attributeGroup && this.getAttributeNames().map((attributeName, i) => (
              <MenuItem key={i} value={attributeName.trim()}>{attributeName}</MenuItem>
            ))}
          </Select>
        </FormControl>
        {this.state.attributeGroup && this.state.attribute &&
        <Histogram
          size={[190, 100]}
          data={this.getData()}
          brushEnabled={true}
          onBrush={this.onBrush}/>}
        {this.state.attributeGroup && this.state.attribute &&
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap' }}>
          <InputLabel htmlFor='filter-bin-label'># of bins</InputLabel>
          <Select
            value={this.state.numberBins}
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
      </div>);
  }
}

export default withStyles({})(FilterPanel);
