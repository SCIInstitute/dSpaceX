import Input from '@material-ui/core/Input';
import React, { Component } from 'react';
import FormControl from '@material-ui/core/es/FormControl/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/es/MenuItem/MenuItem';
import Paper from '@material-ui/core/Paper';
import Select from '@material-ui/core/Select';
import { withStyles } from '@material-ui/core/styles';
import Histogram from './histogram';

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
    if (this.state.attributeGroup === 'parameters') {
      return this.props.parameters.filter((p) => p.parameterName === this.state.attribute)[0].parameter;
    } else if (this.state.attributeGroup === 'qois') {
      return this.props.qois.filter((q) => q.qoiName === this.state.attribute)[0].qoi;
    }
  };

  /**
   * Renders the filter panel
   * @return {jsx}
   */
  render() {
    const { classes } = this.props;
    return (
      <div style={{ width:'100%' }}>
        {this.state.attributeGroup && this.state.attribute &&
        <Histogram size={[500, 500]} data={this.getData()}/>}
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap' }}>
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
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap' }}>
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
        </FormControl>
      </div>);
  }
}

export default withStyles({})(FilterPanel);
